//******************************************************************************
//
// Simple MIDI Library / SMInDevCtrl
//
// MIDI input device control class.
//
// Copyright (C) 2012-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMInDevCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMInDevCtrl::SMInDevCtrl(void)
{
	// Port info
	m_PortInfo.isExist = false;
	m_PortInfo.devId = 0;
	m_PortInfo.hMidiIn = NULL;
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));

	// Callback function
	m_pInReadCallBack = NULL;
	m_pCallBackUserParam = NULL;

	// Packet parsing
	m_isContinueSysEx = false;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMInDevCtrl::~SMInDevCtrl()
{
	m_InDevList.clear();
	ClosePortDev();
}

//******************************************************************************
// Initialize
//******************************************************************************
int SMInDevCtrl::Initialize()
{
	int result = 0;
	
	// Clear port info
	result = ClearPortInfo();
	if (result != 0) goto EXIT;

	// Build MIDI input device list
	result = _InitDevList();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// Initialize device list
//******************************************************************************
int SMInDevCtrl::_InitDevList()
{
	int result = 0;
	MMRESULT apiresult = 0;
	unsigned long devId = 0;
	unsigned long devNum = 0;
	MIDIINCAPS mic;
	SMInDevInfo devInfo;

	m_InDevList.clear();

	// Number of MIDI output devices
	devNum = midiInGetNumDevs();

	// Get MIDI output device info
	for (devId = 0; devId < devNum; devId++) {

		ZeroMemory(&mic, sizeof(MIDIINCAPS));
		ZeroMemory(&devInfo, sizeof(SMInDevInfo));

		apiresult= midiInGetDevCaps(devId, &mic, sizeof(MIDIINCAPS));
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI In device access error.", apiresult, 0);
			goto EXIT;
		}
		devInfo.devId = devId;
		memcpy(devInfo.productName, mic.szPname, MAXPNAMELEN);

		// Register retrieved info to the list
		m_InDevList.push_back(devInfo);
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Get device count
//******************************************************************************
unsigned long SMInDevCtrl::GetDevNum()
{
	return (unsigned long)m_InDevList.size();
}

//******************************************************************************
// Get device product name
//******************************************************************************
int SMInDevCtrl::GetDevProductName(
		unsigned long index,
		std::string& name
	)
{
	int result = 0;
	SMInDevListItr itr;
	
	if (index >= m_InDevList.size()) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	itr = m_InDevList.begin();
	advance(itr, index);
	
	name = itr->productName;
	
EXIT:;
	return result;
}

//******************************************************************************
// Set device corresponding to port
//******************************************************************************
int SMInDevCtrl::SetPortDev(
		const char* pProductName
	)
{
	int result = 0;
	bool isFound = false;
	SMInDevListItr itr;
	
	if (pProductName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	for (itr = m_InDevList.begin(); itr != m_InDevList.end(); itr++) {
		if (strcmp(itr->productName, pProductName) == 0) {
			m_PortInfo.isExist = true;
			m_PortInfo.devId = itr->devId;
			//m_PortInfo.hMidiIn = NULL;
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
// Register MIDI event read callback function
//******************************************************************************
void SMInDevCtrl::SetInReadCallBack(
		SMInReadCallBack pCallBack,
		void* pUserParam
	)
{
	m_pInReadCallBack = pCallBack;
	m_pCallBackUserParam = pUserParam;
}

//******************************************************************************
// Open device corresponding to port
//******************************************************************************
int SMInDevCtrl::OpenPortDev()
{
	int result = 0;
	MMRESULT apiresult = 0;
	HMIDIIN hMidiIn = NULL;
	unsigned char* pBuf = NULL;
	
	result = ClosePortDev();
	if (result != 0) goto EXIT;

	// Skip if port does not exist
	if (!m_PortInfo.isExist) goto EXIT;;

	m_isContinueSysEx = false;

	// Open device
	apiresult = midiInOpen(
					&hMidiIn,			// handle address
					m_PortInfo.devId,	// device identifier
					(DWORD_PTR)_InReadCallBack,	// callback function
					(DWORD_PTR)this,	// user instance data passed to callback function
					CALLBACK_FUNCTION	// callback flag: callback function
				);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
		goto EXIT;
	}
	m_PortInfo.hMidiIn = hMidiIn;
	
	// Create MIDI input buffer
	try {
		pBuf = new unsigned char[SM_MIDIIN_BUF_SIZE];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	// Create header
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	m_PortInfo.midiHdr.lpData         = (LPSTR)pBuf;
	m_PortInfo.midiHdr.dwBufferLength = SM_MIDIIN_BUF_SIZE;
	m_PortInfo.midiHdr.dwFlags        = 0;
	pBuf = NULL;
	
	// Prepare MIDI input buffer
	apiresult = midiInPrepareHeader(hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}

	// Register MIDI input buffer
	apiresult = midiInAddBuffer(hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}

	// Start MIDI input
	apiresult = midiInStart(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
		goto EXIT;
	}
	
EXIT:;
	delete [] pBuf;
	return result;
}

//******************************************************************************
// Close device corresponding to port
//******************************************************************************
int SMInDevCtrl::ClosePortDev()
{
	int result = 0;
	UINT apiresult = 0;
	
	// Skip if port does not exist
	if (!m_PortInfo.isExist) goto EXIT;

	// Skip if port is not open
	if (m_PortInfo.hMidiIn == NULL) goto EXIT;

	// Stop MIDI input
	//  if a buffer exists in the queue, the current buffer is marked as processed
	//  the dwBytesRecorded member of MIDIHDR holds the actual length of the data
	//  however an empty buffer left in the queue is not marked as processed
	apiresult = midiInStop(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}

	// Stop MIDI input
	//  return unprocessed input buffers to the callback function
	//  set the MHDR_DONE flag on the dwFlags member of MIDIHDR
	apiresult = midiInReset(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}

	// Release MIDI input buffer
	apiresult = midiInUnprepareHeader(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}

	// Discard buffer
	delete [] (unsigned char*)(m_PortInfo.midiHdr.lpData);
	m_PortInfo.midiHdr.lpData = NULL;

	// Close device
	apiresult = midiInClose(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", 0, 0);
		goto EXIT;
	}
	m_PortInfo.hMidiIn = NULL;
	
EXIT:;
	return result;
}


//******************************************************************************
// Clear port info
//******************************************************************************
int SMInDevCtrl::ClearPortInfo()
{
	int result = 0;
	
	result = ClosePortDev();
	if (result != 0) goto EXIT;
	
	m_PortInfo.isExist = false;
	m_PortInfo.devId = 0;
	m_PortInfo.hMidiIn = NULL;
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN read callback function
//******************************************************************************
void SMInDevCtrl::_InReadCallBack(
		HMIDIIN hMidiIn,
		UINT wMsg,
		DWORD_PTR dwInstance,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
	)
{
	SMInDevCtrl* pInDevCtrl = NULL;
	
	pInDevCtrl = (SMInDevCtrl*)dwInstance;
	pInDevCtrl->_InReadProc(hMidiIn, wMsg, dwParam1, dwParam2);
	
	return;
}

//******************************************************************************
// MIDI IN read process
//******************************************************************************
void SMInDevCtrl::_InReadProc(
		HMIDIIN hMidiIn,
		UINT wMsg,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
	)
{
	int result = 0;
	SMEvent event;
	
	switch (wMsg) {
		case MIM_OPEN:
			// MIDI input device open
			break;
		case MIM_CLOSE:
			// MIDI input device close
			break;
		case MIM_DATA:
			// MIDI message received
			//  dwParam1 MIDI message
			//  dwParam2 timestamp
			m_isContinueSysEx = false;
			result = _InReadProcMIDI(dwParam1, dwParam2, &event);
			if (result != 0) goto EXIT;
			break;
		case MIM_LONGDATA:
			// System exclusive received
			//  dwParam1 pointer to MIDIHDR structure
			//  dwParam2 timestamp
			result = _InReadProcSysEx((MIDIHDR*)dwParam1, dwParam2, &m_isContinueSysEx, &event);
			if (result != 0) goto EXIT;
			break;
		case MIM_ERROR:
			// Invalid MIDI message received
			break;
		case MIM_LONGERROR:
			// Invalid exclusive message received
			break;
		case MIM_MOREDATA:
			// Unprocessed MIDI message
			// only occurs when MIDI_IO_STATUS is specified in midiInOpen
			break;
		default:
			break;
	}

	// Call callback
	if ((m_pInReadCallBack != NULL) &&
		(event.GetType() != SMEvent::EventNone)) {
		result = m_pInReadCallBack(&event, m_pCallBackUserParam);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(NULL);
	}
	return;
}

//******************************************************************************
// MIDI message read process
//******************************************************************************
int SMInDevCtrl::_InReadProcMIDI(
		DWORD_PTR midiMessage,
		DWORD_PTR timestamp,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char status = 0;
	unsigned char data[2] = { 0, 0 };
	unsigned long dataLength = 0;
	
	status  = (unsigned char)((midiMessage      ) & 0x000000FF);
	data[0] = (unsigned char)((midiMessage >>  8) & 0x000000FF);
	data[1] = (unsigned char)((midiMessage >> 16) & 0x000000FF);
	
	if ((status & 0xF0) != 0xF0) {
		// MIDI message
		dataLength = _GetMIDIMsgSize(status) - 1;
		result = pEvent->SetMIDIData(status, data, dataLength);
		if (result != 0) goto EXIT;
	}
	else if (status == 0xF0) {
		// System exclusive message
		// this API behavior should not occur
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	else {
		// System common message or system realtime message
		dataLength = _GetSysMsgSize(status) - 1;
		result = pEvent->SetSysMsgData(status, data, dataLength);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// System exclusive read process
//******************************************************************************
int SMInDevCtrl::_InReadProcSysEx(
		MIDIHDR* pMIDIHDR,
		DWORD_PTR timestamp,
		bool* pIsContinueSysEx,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char* pData = NULL;
	UINT apiresult = 0;
	
	// If received data size is zero, do nothing
	if (pMIDIHDR->dwBytesRecorded == 0) goto EXIT;

	// First read of system exclusive
	if (!(*pIsContinueSysEx)) {
		pData = (unsigned char*)(pMIDIHDR->lpData);
		result = pEvent->SetSysExData(0xF0, pData + 1, pMIDIHDR->dwBytesRecorded - 1);
		if (result != 0) goto EXIT;
	}
	// Second and later packets
	else {
		pData = (unsigned char*)(pMIDIHDR->lpData);
		result = pEvent->SetSysExData(0xF7, pData, pMIDIHDR->dwBytesRecorded);
		if (result != 0) goto EXIT;
	}

	// Check the end of the system exclusive
	if (pData[(pMIDIHDR->dwBytesRecorded)-1] == 0xF7) {
		// System exclusive closes
		*pIsContinueSysEx = false;
	}
	else {
		// If the last byte is not 0xF7, data continues into the next packet
		*pIsContinueSysEx = true;
	}

	// Prepare MIDI input buffer
	apiresult = midiInPrepareHeader(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}

	// Register MIDI input buffer
	apiresult = midiInAddBuffer(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Get MIDI message size
//******************************************************************************
unsigned long SMInDevCtrl::_GetMIDIMsgSize(unsigned char status)
{
	unsigned long size = 0;

	switch (status & 0xF0) {
		case 0x80: size = 3; break;  // note off
		case 0x90: size = 3; break;  // note on
		case 0xA0: size = 3; break;  // polyphonic key pressure
		case 0xB0: size = 3; break;  // control change
		case 0xC0: size = 2; break;  // program change
		case 0xD0: size = 2; break;  // channel pressure
		case 0xE0: size = 3; break;  // pitch bend
		case 0xF0:
			size = _GetSysMsgSize(status);
			break;
	}
	
	return size;
}

//******************************************************************************
// Get system message size
//******************************************************************************
unsigned long SMInDevCtrl::_GetSysMsgSize(unsigned char status)
{
	unsigned long size = 0;
	
	switch (status) {
		case 0xF0: size = 0; break;  // F0 ... F7 system exclusive
		case 0xF1: size = 2; break;  // F1 dd     system common message: quarter frame (MTC)
		case 0xF2: size = 3; break;  // F2 dl dm  system common message: song position pointer
		case 0xF3: size = 2; break;  // F3 dd     system common message: song select
		case 0xF4: size = 1; break;  // F4 undefined
		case 0xF5: size = 1; break;  // F5 undefined
		case 0xF6: size = 1; break;  // F6 system common message: tune request
		case 0xF7: size = 1; break;  // F7 end of system exclusive
		case 0xF8: size = 1; break;  // F8 system realtime message: timing clock
		case 0xF9: size = 1; break;  // F9 undefined
		case 0xFA: size = 1; break;  // FA system realtime message: start
		case 0xFB: size = 1; break;  // FB system realtime message: continue
		case 0xFC: size = 1; break;  // FC system realtime message: stop
		case 0xFD: size = 1; break;  // FD undefined
		case 0xFE: size = 1; break;  // FE system realtime message: active sensing
		case 0xFF: size = 1; break;  // FF system realtime message: system reset
	}
	
	return size;
}

} // end of namespace


