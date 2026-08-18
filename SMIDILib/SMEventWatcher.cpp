//******************************************************************************
//
// Simple MIDI Library / SMEventWatcher
//
// Event watcher class.
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventWatcher.h"
#include "SMEventSysMsg.h"
#include <spdlog/spdlog.h>

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMEventWatcher::SMEventWatcher(void)
{
	m_pMsgTrans = NULL;
	m_isNoteEventPostEnabled = true;
	_ClearChInfo();
}

//******************************************************************************
// Destructor
//******************************************************************************
SMEventWatcher::~SMEventWatcher(void)
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int SMEventWatcher::Initialize(SMMsgTransmitter* pMsgTrans)
{
	int result = 0;
	
	m_pMsgTrans = pMsgTrans;
	
	_ClearChInfo();
	
	return result;
}

//******************************************************************************
// Event watch
//******************************************************************************
int SMEventWatcher::WatchEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	SMEventMIDI eventMIDI;
	SMEventSysMsg eventSysMsg;
	
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		eventMIDI.Attach(pEvent);
		
		//MIDIEvent monitoring
		result = _WatchEventMIDI(portNo, &eventMIDI);
		if (result != 0) goto EXIT;

		//Monitor control change
		if (eventMIDI.GetChMsg() == SMEventMIDI::ControlChange) {
			result = _WatchEventControlChange(portNo, &eventMIDI);
			if (result != 0) goto EXIT;
			result = _WatchEventControlChange2(portNo, &eventMIDI);
			if (result != 0) goto EXIT;
		}
	}
	else if (pEvent->GetType() == SMEvent::EventSysMsg) {
		eventSysMsg.Attach(pEvent);
		
		//System message event monitoring
		result = _WatchEventSysMsg(portNo, &eventSysMsg);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDIEvent watch
//******************************************************************************
int SMEventWatcher::WatchEventMIDI(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	return _WatchEventMIDI(portNo, pMIDIEvent);
}

//******************************************************************************
// Control change event watch
//******************************************************************************
int SMEventWatcher::WatchEventControlChange(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	return _WatchEventControlChange(portNo, pMIDIEvent);
}

//******************************************************************************
// Clear channel info
//******************************************************************************
void SMEventWatcher::_ClearChInfo()
{
	unsigned long portNo = 0;
	unsigned long chNo = 0;
	
	for (portNo = 0; portNo < SM_MAX_PORT_NUM; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			//RPN/NRPN selection state
			m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
			//RPN
			m_RPN_MSB[portNo][chNo] = 0x7F; //RPN NULL
			m_RPN_LSB[portNo][chNo] = 0x7F; //RPN NULL
			//Pitch bend sensitivity
			m_PitchBendSensitivity[portNo][chNo] = SM_DEFAULT_PITCHBEND_SENSITIVITY;
		}
	}
	
	return;
}

//******************************************************************************
// MIDIEvent monitoring processing
//******************************************************************************
int SMEventWatcher::_WatchEventMIDI(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	
	//Notify note off/on (for Live. Disabled during playback since NoteTracker manages it)
	if (m_isNoteEventPostEnabled) {
		if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOff) {
			m_pMsgTrans->PostNoteOff(
					portNo, pMIDIEvent->GetChNo(), pMIDIEvent->GetNoteNo());
		}
		else if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn) {
			m_pMsgTrans->PostNoteOn(
					portNo, pMIDIEvent->GetChNo(),
					pMIDIEvent->GetNoteNo(), pMIDIEvent->GetVelocity());
		}
	}
	
	//Notify pitch bend
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::PitchBend) {
		m_pMsgTrans->PostPitchBend(
				portNo,
				pMIDIEvent->GetChNo(),
				pMIDIEvent->GetPitchBendValue(),
				m_PitchBendSensitivity[portNo][pMIDIEvent->GetChNo()]
			);
	}
	
	return result;
}

//******************************************************************************
// Control change monitoring
//******************************************************************************
int SMEventWatcher::_WatchEventControlChange(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned char msb = 0;
	unsigned char chNo = 0;
	
	chNo = pMIDIEvent->GetChNo();
	
	//----------------------------------------------------------------
	// NRPN MSB / LSB
	//----------------------------------------------------------------
	//NRPN MSB (CC#99)
	if (pMIDIEvent->GetCCNo() == 0x63) {
		m_RPN_NRPN_Select[portNo][chNo] = NRPN;
	}
	//NRPN LSB (CC#98)
	if (pMIDIEvent->GetCCNo() == 0x62) {
		m_RPN_NRPN_Select[portNo][chNo] = NRPN;
	}
	
	//----------------------------------------------------------------
	// RPN MSB / LSB
	//----------------------------------------------------------------
	//RPN MSB (CC#101)
	if (pMIDIEvent->GetCCNo() == 0x65) {
		m_RPN_NRPN_Select[portNo][chNo] = RPN;
		m_RPN_MSB[portNo][chNo] = pMIDIEvent->GetCCValue();
	}
	//RPN LSB (CC#100)
	if (pMIDIEvent->GetCCNo() == 0x64) {
		m_RPN_NRPN_Select[portNo][chNo] = RPN;
		m_RPN_LSB[portNo][chNo] = pMIDIEvent->GetCCValue();
		if ((m_RPN_MSB[portNo][chNo] == 0x7F)
			&& (m_RPN_LSB[portNo][chNo] == 0x7F)) {
			m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
		}
	}
	
	//----------------------------------------------------------------
	// Data Entry MSB / LSB
	//----------------------------------------------------------------
	//Data Entry MSB (CC#6)
	if (pMIDIEvent->GetCCNo() == 0x06) {
		//Pitch bend sensitivity MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			m_PitchBendSensitivity[portNo][chNo] = pMIDIEvent->GetCCValue();
		}
	}
	//Data Entry LSB (CC#38)
	if (pMIDIEvent->GetCCNo() == 0x26) {
		//No control performed
	}

	//Data Increment (CC#96)
	if (pMIDIEvent->GetCCNo() == 0x60) {
		//Pitch bend sensitivity MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			msb = m_PitchBendSensitivity[portNo][chNo];
			if (msb < 24) {
				m_PitchBendSensitivity[portNo][chNo] = msb++;
			}
		}
	}
	//Data Decremnet (CC#97)
	if (pMIDIEvent->GetCCNo() == 0x61) {
		//Pitch bend sensitivity MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			msb = m_PitchBendSensitivity[portNo][chNo]++;
			if (msb > 0) {
				m_PitchBendSensitivity[portNo][chNo] = msb--;
			}
		}
	}
	
	//----------------------------------------------------------------
	// Reset All Controllers
	//----------------------------------------------------------------
	//Reset All Controllers (CC#121)
	if (pMIDIEvent->GetCCNo() == 0x79) {
		//Notify pitch bend: 0
		m_pMsgTrans->PostPitchBend(portNo, chNo, 0, m_PitchBendSensitivity[portNo][chNo]);
		//RPN/NRPN selection state
		m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
		//RPN
		m_RPN_MSB[portNo][chNo] = 0x7F; //RPN NULL
		m_RPN_LSB[portNo][chNo] = 0x7F; //RPN NULL

		//For Roland SC series / Yamaha MU series
		//CC#121 Reset All Controllers clears the following values
		//  An     Polyphonic key pressure  0
		//  Dn     Channel pressure  0
		//  En     Pitch bend  0
		//  CC#1   Modulation  0
		//  CC#11  Expression  127
		//  CC#64  Hold 1    0
		//  CC#65  Portamento  0
		//  CC#66  Sostenuto  0
		//  CC#67  Soft  0
		//  CC#98,99   NRPN  Unset state (already-set data is unchanged)
		//  CC#100,101 RPN   Unset state (already-set data is unchanged)
	}
	
	//EXIT:;
	return result;
}

//******************************************************************************
// Get RPN type
//******************************************************************************
SMEventWatcher::RPN_Type SMEventWatcher::_GetCurRPNType(
		unsigned char portNo,
		unsigned char chNo
	)
{
	RPN_Type type = RPN_None;
	
	if (m_RPN_NRPN_Select[portNo][chNo] == RPN) {
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x00)) {
			type = PitchBendSensitivity;
		}
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x01)) {
			type = MasterFineTune;
		}
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x02)) {
			type = MasterCourseTune;
		}
	}
	
	return type;
}

//******************************************************************************
// Control change monitoring2
//******************************************************************************
int SMEventWatcher::_WatchEventControlChange2(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned char chNo = 0;
	
	chNo = pMIDIEvent->GetChNo();
	
	//ALL SOUND OFF (CC#120) / ALL NOTE OFF (CC#123)
	if (m_isNoteEventPostEnabled) {
		if ((pMIDIEvent->GetCCNo() == 0x78) || (pMIDIEvent->GetCCNo() == 0x7B)) {
			m_pMsgTrans->PostAllNoteOff(portNo, chNo);
		}
	}
	
	return result;
}

//******************************************************************************
// System message event monitoring processing
//******************************************************************************
int SMEventWatcher::_WatchEventSysMsg(
		unsigned char portNo,
		SMEventSysMsg* pEventSysMsg
	)
{
	int result = 0;

	//Currently does nothing
	goto EXIT;

	{
		auto logger = spdlog::get("SM");
		if (logger) {
			switch (pEventSysMsg->GetSysMsg()) {
				case SMEventSysMsg::Common_QuarterFrame:       logger->trace("Common_QuarterFrame"); break;
				case SMEventSysMsg::Common_SongPositionPointer: logger->trace("Common_SongPositionPointer"); break;
				case SMEventSysMsg::Common_SongSelect:          logger->trace("Common_SongSelect"); break;
				case SMEventSysMsg::Common_TuneRequest:         logger->trace("Common_TuneRequest"); break;
				case SMEventSysMsg::RealTime_TimingClock:       logger->trace("RealTime_TimingClock"); break;
				case SMEventSysMsg::RealTime_Start:             logger->trace("RealTime_Start"); break;
				case SMEventSysMsg::RealTime_Continue:          logger->trace("RealTime_Continue"); break;
				case SMEventSysMsg::RealTime_Stop:              logger->trace("RealTime_Stop"); break;
				case SMEventSysMsg::RealTime_ActiveSensing:     logger->trace("RealTime_ActiveSensing"); break;
				case SMEventSysMsg::RealTime_SystemReset:       logger->trace("RealTime_SystemReset"); break;
				default: break;
			}
		}
	}

EXIT:;
	return result;
}

} // end of namespace


