//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrl
//
// MIDI output device control class.
//
// Copyright (C) 2010-2021 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMOutDevCtrl.h"
#include "IPortOutput.h"
#include "LibremidiPortOutput.h"
#include "KDMAPIDirectOutput.h"
#include <libremidi/libremidi.hpp>
#include <map>
#include <memory>
#include <vector>
#include <set>
#include <cstdio>

using namespace YNBaseLib;

namespace SMIDILib {

//******************************************************************************
// Implementation data (hidden from header via pimpl)
//******************************************************************************
struct SMOutDevCtrl::ImplData
{
	std::vector<libremidi::output_port> outputPorts;
	std::map<unsigned long, std::unique_ptr<IPortOutput>> openDevices;
	std::set<unsigned long> kdmapiVirtualDevIds;
	bool kdmapiForkDetected = false;
	bool kdmapiStreamInitialized = false;
	SMTransportType currentTransportType = SMTransportType::None;

	IPortOutput* GetPortOutput(unsigned long devId)
	{
		auto it = openDevices.find(devId);
		if (it != openDevices.end())
			return it->second.get();
		return nullptr;
	}

	bool IsKDMAPIVirtualPort(unsigned long devId)
	{
		return kdmapiVirtualDevIds.count(devId) > 0;
	}
};

//******************************************************************************
// Constructor
//******************************************************************************
SMOutDevCtrl::SMOutDevCtrl(void)
{
	m_pImpl = new ImplData();

	for (unsigned char portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
	}
}

//******************************************************************************
// Destructor
//******************************************************************************
SMOutDevCtrl::~SMOutDevCtrl(void)
{
	ClosePortDevAll();
	delete m_pImpl;
	m_pImpl = nullptr;
}

//******************************************************************************
// Initialize
//******************************************************************************
int SMOutDevCtrl::Initialize()
{
	int result = 0;

	// Clear port info
	result = ClearPortInfo();
	if (result != 0) goto EXIT;

	// Build MIDI output device list
	result = _InitDevList();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize device list
//******************************************************************************
int SMOutDevCtrl::_InitDevList()
{
	int result = 0;
	unsigned long devId = 0;

	m_OutDevList.clear();
	m_pImpl->outputPorts.clear();
	m_pImpl->kdmapiVirtualDevIds.clear();
	m_pImpl->kdmapiForkDetected = false;

	// Detect OmniMIDI Mod before device enumeration
	KDMAPIDirectOutput::UnloadDLL();
	if (KDMAPIDirectOutput::LoadDLL() && KDMAPIDirectOutput::IsForkDetected()) {
		m_pImpl->kdmapiForkDetected = true;
	}

	// Enumerate WinMM output ports
	{
		libremidi::observer obs{{}, libremidi::winmm_observer_configuration{}};
		auto ports = obs.get_output_ports();
		for (auto& port : ports) {
			SMOutDevInfo devInfo;
			memset(&devInfo, 0, sizeof(SMOutDevInfo));
			devInfo.devId = devId;
			strncpy_s(
				devInfo.productName,
				SM_MIDIOUT_PRODUCT_NAME_MAX,
				port.port_name.c_str(),
				_TRUNCATE
			);

			m_OutDevList.push_back(devInfo);
			m_pImpl->outputPorts.push_back(std::move(port));
			devId++;
		}
	}

	if (m_pImpl->kdmapiForkDetected) {
		// Fork detected: add virtual KDMAPI ports instead of libremidi observer
		unsigned long portCount = KDMAPIDirectOutput::GetPortCount();
		for (unsigned long i = 0; i < portCount; i++) {
			SMOutDevInfo devInfo;
			memset(&devInfo, 0, sizeof(SMOutDevInfo));
			devInfo.devId = devId;

			char name[SM_MIDIOUT_PRODUCT_NAME_MAX];
			_snprintf_s(name, SM_MIDIOUT_PRODUCT_NAME_MAX, _TRUNCATE,
				"OmniMIDI Mod (KDMAPI Port %c)", 'A' + (char)i);
			strncpy_s(devInfo.productName, SM_MIDIOUT_PRODUCT_NAME_MAX, name, _TRUNCATE);

			m_OutDevList.push_back(devInfo);
			m_pImpl->outputPorts.push_back(libremidi::output_port{});
			m_pImpl->kdmapiVirtualDevIds.insert(devId);
			devId++;
		}
	}
	else {
		// Original OmniMIDI or not installed: enumerate via libremidi
		libremidi::observer obs{{}, libremidi::kdmapi::observer_configuration{}};
		auto ports = obs.get_output_ports();
		for (auto& port : ports) {
			SMOutDevInfo devInfo;
			memset(&devInfo, 0, sizeof(SMOutDevInfo));
			devInfo.devId = devId;
			strncpy_s(
				devInfo.productName,
				SM_MIDIOUT_PRODUCT_NAME_MAX,
				port.display_name.c_str(),
				_TRUNCATE
			);

			m_OutDevList.push_back(devInfo);
			m_pImpl->outputPorts.push_back(std::move(port));
			devId++;
		}
	}

	return result;
}

//******************************************************************************
// Get device count
//******************************************************************************
unsigned long SMOutDevCtrl::GetDevNum()
{
	return (unsigned long)m_OutDevList.size();
}

//******************************************************************************
// Get device product name
//******************************************************************************
int SMOutDevCtrl::GetDevProductName(
		unsigned long index,
		std::string& name
	)
{
	int result = 0;
	SMOutDevListItr itr;

	if (index >= m_OutDevList.size()) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	itr = m_OutDevList.begin();
	advance(itr, index);

	name = itr->productName;

EXIT:;
	return result;
}

//******************************************************************************
// Set device corresponding to port
//******************************************************************************
int SMOutDevCtrl::SetPortDev(
		unsigned char portNo,
		const char* pProductName
	)
{
	int result = 0;
	bool isFound = false;
	SMOutDevListItr itr;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pProductName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	for (itr = m_OutDevList.begin(); itr != m_OutDevList.end(); itr++) {
		if (strcmp(itr->productName, pProductName) == 0) {
			m_PortInfo[portNo].isExist = true;
			m_PortInfo[portNo].devId = itr->devId;
			isFound = true;
			break;
		}
	}
	if (!isFound) {
		TCHAR warnMsg[256];
		_sntprintf_s(warnMsg, 256, _TRUNCATE,
			_T("MIDI OUT device not found: %s (Port %c)"),
			pProductName, _T('A') + portNo);
		YN_SET_WARN(warnMsg, portNo, 0);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get device ID corresponding to port
//******************************************************************************
int SMOutDevCtrl::GetPortDevId(
		unsigned char portNo,
		unsigned long* pDevId
	)
{
	int result = 0;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pDevId == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_PortInfo[portNo].isExist) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	*pDevId = m_PortInfo[portNo].devId;

EXIT:;
	return result;
}

//******************************************************************************
// Open devices corresponding to all ports
//******************************************************************************
int SMOutDevCtrl::OpenPortDevAll()
{
	int result = 0;
	unsigned char portNo = 0;
	unsigned long devId = 0;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	// Initialize KDMAPI stream if fork is detected and any virtual port is assigned
	if (m_pImpl->kdmapiForkDetected) {
		bool hasKDMAPIPort = false;
		for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
			if (m_PortInfo[portNo].isExist && m_pImpl->IsKDMAPIVirtualPort(m_PortInfo[portNo].devId)) {
				hasKDMAPIPort = true;
				break;
			}
		}
		if (hasKDMAPIPort) {
			if (!KDMAPIDirectOutput::InitializeStream()) {
				YN_SET_WARN("KDMAPI stream initialization failed.", 0, 0);
			}
			else {
				m_pImpl->kdmapiStreamInitialized = true;
			}
		}
	}

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		// Skip if port does not exist
		if (!m_PortInfo[portNo].isExist) continue;

		devId = m_PortInfo[portNo].devId;

		// Skip if this device is already open (shared by another port)
		if (m_pImpl->GetPortOutput(devId) != nullptr) continue;

		if (m_pImpl->IsKDMAPIVirtualPort(devId)) {
			// KDMAPI virtual port: create KDMAPIDirectOutput
			if (!m_pImpl->kdmapiStreamInitialized) {
				YN_SET_WARN("KDMAPI stream not initialized.", devId, portNo);
				m_PortInfo[portNo].isExist = false;
				continue;
			}

			m_pImpl->openDevices[devId] = std::make_unique<KDMAPIDirectOutput>(portNo);
		}
		else {
			// libremidi port: create LibremidiPortOutput
			if (devId >= m_pImpl->outputPorts.size()) {
				YN_SET_WARN("MIDI OUT device open error: invalid device index.", devId, portNo);
				m_PortInfo[portNo].isExist = false;
				continue;
			}

			const auto& port = m_pImpl->outputPorts[devId];

			std::unique_ptr<libremidi::midi_out> pMidiOut;
			if (port.api == libremidi::API::KDMAPI) {
				pMidiOut = std::make_unique<libremidi::midi_out>(
					libremidi::output_configuration{},
					libremidi::kdmapi::output_configuration{}
				);
			}
			else {
				pMidiOut = std::make_unique<libremidi::midi_out>(
					libremidi::output_configuration{},
					libremidi::winmm_output_configuration{}
				);
			}

			auto err = pMidiOut->open_port(port);
			if (err.is_set()) {
				YN_SET_WARN("MIDI OUT device open error.", devId, portNo);
				m_PortInfo[portNo].isExist = false;
				continue;
			}

			m_pImpl->openDevices[devId] = std::make_unique<LibremidiPortOutput>(std::move(pMidiOut));
		}
	}

	// Determine transport type from open devices
	{
		SMTransportType transport = SMTransportType::None;
		for (const auto& [id, output] : m_pImpl->openDevices) {
			if (m_pImpl->IsKDMAPIVirtualPort(id)) {
				transport = SMTransportType::KDMAPIMod;
				break;
			}
			if (id < m_pImpl->outputPorts.size()) {
				const auto& p = m_pImpl->outputPorts[id];
				if (p.api == libremidi::API::KDMAPI) {
					transport = SMTransportType::KDMAPI;
				} else if (transport == SMTransportType::None) {
					transport = SMTransportType::WinMM;
				}
			}
		}
		m_pImpl->currentTransportType = transport;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Close devices corresponding to all ports
//******************************************************************************
int SMOutDevCtrl::ClosePortDevAll()
{
	m_pImpl->openDevices.clear();
	m_pImpl->currentTransportType = SMTransportType::None;

	if (m_pImpl->kdmapiStreamInitialized) {
		KDMAPIDirectOutput::TerminateStream();
		m_pImpl->kdmapiStreamInitialized = false;
	}

	return 0;
}

//******************************************************************************
// Get transport type
//******************************************************************************
SMTransportType SMOutDevCtrl::GetTransportType() const
{
	return m_pImpl->currentTransportType;
}

//******************************************************************************
// Check if KDMAPI Mod virtual ports and WinMM ports are mixed
//******************************************************************************
bool SMOutDevCtrl::HasMixedKDMAPIAndWinMM() const
{
	bool hasKDMAPI = false;
	bool hasWinMM = false;

	for (unsigned char portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		if (!m_PortInfo[portNo].isExist) continue;

		unsigned long devId = m_PortInfo[portNo].devId;

		if (m_pImpl->IsKDMAPIVirtualPort(devId)) {
			hasKDMAPI = true;
		} else if (devId < m_pImpl->outputPorts.size()
				&& m_pImpl->outputPorts[devId].api == libremidi::API::KDMAPI) {
			hasKDMAPI = true;
		} else {
			hasWinMM = true;
		}
	}

	return hasKDMAPI && hasWinMM;
}

//******************************************************************************
// Clear port info
//******************************************************************************
int SMOutDevCtrl::ClearPortInfo()
{
	int result = 0;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	for (unsigned char portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Send MIDI data (short message)
//******************************************************************************
int SMOutDevCtrl::SendShortMsg(
		unsigned char portNo,
		unsigned long msg
	)
{
	int result = 0;

	// If a port outside the supported range is specified, do nothing
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	// Do nothing if the port does not exist
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	{
		auto* pPortOutput = m_pImpl->GetPortOutput(m_PortInfo[portNo].devId);
		if (pPortOutput == nullptr) {
			result = YN_SET_ERR("Program error.", portNo, 0);
			goto EXIT;
		}

		int sendResult = pPortOutput->SendShort(msg);
		if (sendResult != 0) {
			result = YN_SET_ERR("MIDI OUT device output error.", portNo, msg);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Send MIDI data (long message)
//******************************************************************************
int SMOutDevCtrl::SendLongMsg(
		unsigned char portNo,
		unsigned char* pMsg,
		unsigned long size
	)
{
	int result = 0;

	if (pMsg == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	// If a port outside the supported range is specified, do nothing
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	// Do nothing if the port does not exist
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	{
		auto* pPortOutput = m_pImpl->GetPortOutput(m_PortInfo[portNo].devId);
		if (pPortOutput == nullptr) {
			result = YN_SET_ERR("Program error.", portNo, 0);
			goto EXIT;
		}

		int sendResult = pPortOutput->SendLong(pMsg, size);
		if (sendResult != 0) {
			result = YN_SET_ERR("MIDI OUT device output error.", portNo, size);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Note off on all ports
//******************************************************************************
int SMOutDevCtrl::NoteOffAll()
{
	int result = 0;

	for (unsigned char portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		if (!m_PortInfo[portNo].isExist) continue;

		auto* pPortOutput = m_pImpl->GetPortOutput(m_PortInfo[portNo].devId);
		if (pPortOutput == nullptr) continue;

		int sendResult = pPortOutput->NoteOffAll();
		if (sendResult != 0) {
			result = YN_SET_ERR("MIDI OUT device output error.", 0, portNo);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Sound off on all ports
//******************************************************************************
int SMOutDevCtrl::SoundOffAll()
{
	int result = 0;

	for (unsigned char portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		if (!m_PortInfo[portNo].isExist) continue;

		auto* pPortOutput = m_pImpl->GetPortOutput(m_PortInfo[portNo].devId);
		if (pPortOutput == nullptr) continue;

		int sendResult = pPortOutput->SoundOffAll();
		if (sendResult != 0) {
			result = YN_SET_ERR("MIDI OUT device output error.", 0, portNo);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

} // end of namespace
