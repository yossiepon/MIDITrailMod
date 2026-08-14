//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrl
//
// MIDI output device control class.
//
// Copyright (C) 2010-2021 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMOutDevCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMOutDevCtrl::SMOutDevCtrl(void)
{
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
		m_PortInfo[portNo].hMIDIOut = NULL;
	}
}

//******************************************************************************
// Destructor
//******************************************************************************
SMOutDevCtrl::~SMOutDevCtrl(void)
{
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
	MMRESULT apiresult = 0;
	unsigned long devId = 0;
	unsigned long devNum = 0;
	MIDIOUTCAPS moc;
	SMOutDevInfo devInfo;

	m_OutDevList.clear();

	// Number of MIDI output devices
	devNum = midiOutGetNumDevs();

	// Get MIDI output device info
	for (devId = 0; devId < devNum; devId++) {

		ZeroMemory(&moc, sizeof(MIDIOUTCAPS));
		ZeroMemory(&devInfo, sizeof(SMOutDevInfo));

		apiresult= midiOutGetDevCaps(devId, &moc, sizeof(MIDIOUTCAPS));
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI OUT device access error.", apiresult, 0);
			goto EXIT;
		}
		devInfo.devId = devId;
		memcpy(devInfo.productName, moc.szPname, MAXPNAMELEN);

		// Register retrieved info to the list
		m_OutDevList.push_back(devInfo);
	}

EXIT:;
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
			//m_PortInfo[portNo].hMIDIOut = NULL;
			isFound = true;
			break;
		}
	}
	if (!isFound) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
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
	UINT apiresult = 0;
	unsigned char portNo = 0;
	unsigned char prevPortNo = 0;
	unsigned long devId;
	HMIDIOUT hMIDIOut = NULL;
	bool isOpen = false;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		// Skip if port does not exist
		if (!m_PortInfo[portNo].isExist) continue;

		// Get device ID corresponding to port
		devId = m_PortInfo[portNo].devId;

		// Handle the case where the same device is already open on another port
		isOpen = false;
		for (prevPortNo = 0; prevPortNo < portNo; prevPortNo++) {
			if (devId == m_PortInfo[prevPortNo].devId) {
				m_PortInfo[portNo].hMIDIOut = m_PortInfo[prevPortNo].hMIDIOut;
				isOpen = true;
				break;
			}
		}

		// Open the device newly
		if (!isOpen) {
			apiresult = midiOutOpen(
							&hMIDIOut,      // handle
							devId,          // MIDI output device identifier
							NULL,           // playback progress callback function
							NULL,           // user instance data passed to callback function
							CALLBACK_NULL   // callback flag: no callback
						);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
				goto EXIT;
			}
			m_PortInfo[portNo].hMIDIOut = hMIDIOut;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Close devices corresponding to all ports
//******************************************************************************
int SMOutDevCtrl::ClosePortDevAll()
{
	int result = 0;
	UINT apiresult = 0;
	unsigned char portNo = 0;
	unsigned char nextPortNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		// Skip if port does not exist
		if (!m_PortInfo[portNo].isExist) continue;

		// Skip if device is not open
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		// Close the device
		apiresult = midiOutClose(m_PortInfo[portNo].hMIDIOut);
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI OUT device close error.", 0, 0);
			goto EXIT;
		}
		m_PortInfo[portNo].hMIDIOut = NULL;

		// Handle the case where the same device is open on another port
		for (nextPortNo = portNo+1; nextPortNo < SM_MIDIOUT_PORT_NUM_MAX; nextPortNo++) {
			if (m_PortInfo[portNo].devId == m_PortInfo[nextPortNo].devId) {
				m_PortInfo[nextPortNo].hMIDIOut = NULL;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Clear port info
//******************************************************************************
int SMOutDevCtrl::ClearPortInfo()
{
	int result = 0;
	unsigned char portNo = 0;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
		m_PortInfo[portNo].hMIDIOut = NULL;
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
	UINT apiresult = 0;
	HMIDIOUT hMIDIOut = NULL;

	// If a port outside the supported range is specified, do nothing
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	// Do nothing if the port does not exist
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	// Error if the device is not open
	if (m_PortInfo[portNo].hMIDIOut == NULL) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	hMIDIOut = m_PortInfo[portNo].hMIDIOut;

	// Output message: takes about 0.3msec per the MIDI spec
	apiresult = midiOutShortMsg(hMIDIOut, msg);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, msg);
		goto EXIT;
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
	UINT apiresult = 0;
	HMIDIOUT hMIDIOut = NULL;
	MIDIHDR mh;

	//parameter check
	if (pMsg == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	// If a port outside the supported range is specified, do nothing
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	// Do nothing if the port does not exist
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	// Error if the device is not open
	if (m_PortInfo[portNo].hMIDIOut == NULL) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	hMIDIOut = m_PortInfo[portNo].hMIDIOut;

	// Create header
	memset((void*)&mh, 0, sizeof(MIDIHDR));
	mh.lpData         = (LPSTR)pMsg;
	mh.dwBufferLength = size;
	mh.dwFlags        = 0;

	// Prepare output buffer
	apiresult = midiOutPrepareHeader(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}
	// Output message: takes about 0.3msec or more per the MIDI spec
	apiresult = midiOutLongMsg(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}

	// Wait until output completes
	while ((mh.dwFlags & MHDR_DONE) == 0) {
		// There is no callback I/F, so this is the only way...
	}

	// Release output buffer
	apiresult = midiOutUnprepareHeader(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
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
	int i = 0;
	UINT apiresult = 0;
	unsigned long msg = 0;
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		// Skip if the port and device do not exist
		if (!m_PortInfo[portNo].isExist) continue;
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		//All tracks note off
		for (i = 0; i < 16; i++) {
			msg = (0x7B << 8) | (0xB0 | i);
			apiresult = midiOutShortMsg(m_PortInfo[portNo].hMIDIOut, msg);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device output error.", apiresult, portNo);
				goto EXIT;
			}
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
	int i = 0;
	UINT apiresult = 0;
	unsigned long msg = 0;
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		// Skip if the port and device do not exist
		if (!m_PortInfo[portNo].isExist) continue;
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		// Sound off on all tracks
		for (i = 0; i < 16; i++) {
			msg = (0x78 << 8) | (0xB0 | i);
			apiresult = midiOutShortMsg(m_PortInfo[portNo].hMIDIOut, msg);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device output error.", apiresult, portNo);
				goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

} // end of namespace

