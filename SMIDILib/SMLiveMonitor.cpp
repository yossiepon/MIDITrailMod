//******************************************************************************
//
// Simple MIDI Library / SMLiveMonitor
//
// Live (real-time input) monitor class.
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMLiveMonitor.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventSysMsg.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMLiveMonitor::SMLiveMonitor(void)
{	
	m_Status = StatusMonitorOFF;
	m_isMIDITHRU = true;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMLiveMonitor::~SMLiveMonitor()
{
	// Clear port info
	_ClearPortInfo();

	// Close MIDI device
	_CloseMIDIDev();
}

//******************************************************************************
// Initialize
//******************************************************************************
int SMLiveMonitor::Initialize(
		SMMsgQueue* pMsgQueue
	)
{
	int result = 0;
	
	m_pMsgQue = pMsgQueue;	
	
	// Initialize MIDI output device
	result = m_OutDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	// Initialize MIDI input device
	result = m_InDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	// Clear port info
	_ClearPortInfo();

	// Initialize event transmission object
	result = m_MsgTrans.Initialize(pMsgQueue);
	if (result != 0) goto EXIT;

	// Initialize event watcher
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// Register device corresponding to input port
//******************************************************************************
int SMLiveMonitor::SetInPortDev(
		const char* pProductName,
		bool isMIDITHRU
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = strcpy_s(m_InPortDevName, MAXPNAMELEN, pProductName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	m_isMIDITHRU = isMIDITHRU;
	
EXIT:;
	return result;
}

//******************************************************************************
// Register device corresponding to output port
//******************************************************************************
int SMLiveMonitor::SetOutPortDev(
		const char* pProductName
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = strcpy_s(m_OutPortDevName, MAXPNAMELEN, pProductName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Get input port device display name
//******************************************************************************
int SMLiveMonitor::GetInPortDevDisplayName(
		std::string& name
	)
{
	int result = 0;
	
	name = m_InPortDevName;
	
	return result;
}

//******************************************************************************
// Start monitor
//******************************************************************************
int SMLiveMonitor::Start()
{
	int result = 0;
	
	// Do nothing if already monitoring
	if (m_Status == StatusMonitorON) goto EXIT;

	// Open MIDI device
	result = _OpenMIDIDev();
	if (result != 0) goto EXIT;
	
	m_Status = StatusMonitorON;
	
EXIT:;
	return result;
}

//******************************************************************************
// Stop monitor
//******************************************************************************
int SMLiveMonitor::Stop()
{
	int result = 0;
	
	//All tracks note off
	result = m_OutDevCtrl.NoteOffAll();
	if (result != 0) goto EXIT;
	
	// Close MIDI device
	result = _CloseMIDIDev();
	if (result != 0) goto EXIT;

	m_Status = StatusMonitorOFF;
	
EXIT:;
	return result;
}

//******************************************************************************
// Clear port info
//******************************************************************************
void SMLiveMonitor::_ClearPortInfo()
{
	m_InPortDevName[0] = '\0';
	m_OutPortDevName[0] = '\0';
}

//******************************************************************************
// MIDI device open
//******************************************************************************
int SMLiveMonitor::_OpenMIDIDev()
{
	int result = 0;
	
	// Open the output port device
	if (strlen(m_OutPortDevName) > 0) {
		result = m_OutDevCtrl.SetPortDev(0, m_OutPortDevName);
		if (result != 0) goto EXIT;
	}
	result = m_OutDevCtrl.OpenPortDevAll();
	if (result != 0) goto EXIT;

	// Open the input port device
	if (strlen(m_InPortDevName) > 0) {
		result = m_InDevCtrl.SetPortDev(m_InPortDevName);
		if (result != 0) goto EXIT;

		// Register callback
		m_InDevCtrl.SetInReadCallBack(_InReadCallBack, this);
	}
	result = m_InDevCtrl.OpenPortDev();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI device close
//******************************************************************************
int SMLiveMonitor::_CloseMIDIDev()
{
	int result = 0;
	
	// Close the input port device
	result = m_InDevCtrl.ClosePortDev();
	if (result != 0) goto EXIT;

	// Close the output port device
	result = m_OutDevCtrl.ClosePortDevAll();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN read callback
//******************************************************************************
int SMLiveMonitor::_InReadCallBack(
		SMEvent* pEvent,
		void* pUserParam
	)
{
	int result = 0;
	SMLiveMonitor* pLiveMonitor = NULL;
	
	pLiveMonitor = (SMLiveMonitor*)pUserParam;
	
	if (pLiveMonitor != NULL) {
		result = pLiveMonitor->_InReadProc(pEvent);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN read process
//******************************************************************************
int SMLiveMonitor::_InReadProc(SMEvent* pEvent)
{
	int result = 0;

	// Filter MIDI events and register them to the message queue
	// includes control change monitoring
	result = _InReadProcParseEvent(pEvent);
	if (result != 0) goto EXIT;

	// Output to MIDI output device
	result = _InReadProcMIDITHRU(pEvent);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN read process: event parsing
//******************************************************************************
int SMLiveMonitor::_InReadProcParseEvent(SMEvent* pEvent)
{
	int result = 0;
	unsigned char portNo = 0;
	
	//Event watch
	result = m_EventWatcher.WatchEvent(portNo, pEvent);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN read process: MIDITHRU processing
//******************************************************************************
int SMLiveMonitor::_InReadProcMIDITHRU(SMEvent* pEvent)
{
	int result = 0;
	unsigned char portNo = 0;
	
	// Do nothing if MIDITHRU is off
	if (!m_isMIDITHRU) goto EXIT;

	//MIDIEvent transmission
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		result = _InReadProcSendMIDIEvent(portNo, pEvent);
		if (result != 0) goto EXIT;
	}
	// Send system exclusive
	else if (pEvent->GetType() == SMEvent::EventSysEx) {
		result = _InReadProcSendSysExEvent(portNo, pEvent);
		if (result != 0) goto EXIT;
	}
	// Send system message
	else if (pEvent->GetType() == SMEvent::EventSysMsg) {
		result = _InReadProcSendSysMsgEvent(portNo, pEvent);
		if (result != 0) goto EXIT;
	}
	
EXIT:;	
	return result;
}

//******************************************************************************
// MIDIEvent transmission
//******************************************************************************
int SMLiveMonitor::_InReadProcSendMIDIEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned long msg = 0;
	SMEventMIDI midiEvent;
	
	midiEvent.Attach(pEvent);
	
	//Get message
	result = midiEvent.GetMIDIOutShortMsg(&msg);
	if (result != 0) goto EXIT;
	
	// Output message
	result = m_OutDevCtrl.SendShortMsg(portNo, msg);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// SysExEvent transmission
//******************************************************************************
int SMLiveMonitor::_InReadProcSendSysExEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char* pVarMsg = NULL;
	unsigned long size = 0;
	SMEventSysEx sysExEvent;
	
	sysExEvent.Attach(pEvent);
	
	//Get message
	sysExEvent.GetMIDIOutLongMsg(&pVarMsg, &size);
	
	// Output message: control does not return until output completes
	result = m_OutDevCtrl.SendLongMsg(portNo, pVarMsg, size);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// SysMsgEvent transmission
//******************************************************************************
int SMLiveMonitor::_InReadProcSendSysMsgEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned long msg = 0;
	unsigned long size = 0;
	SMEventSysMsg sysMsgEvent;
	
	sysMsgEvent.Attach(pEvent);
	
	//Get message
	result = sysMsgEvent.GetMIDIOutShortMsg(&msg, &size);
	if (result != 0) goto EXIT;
	
	// Output message
	result = m_OutDevCtrl.SendShortMsg(portNo, msg);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

} // end of namespace


