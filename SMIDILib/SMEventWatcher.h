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

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include "SMEventMIDI.h"
#include "SMEventSysMsg.h"
#include "SMMsgTransmitter.h"
#include "SMCommon.h"


namespace SMIDILib {

//******************************************************************************
// Event watcher class
//******************************************************************************
class SMIDILIB_API SMEventWatcher
{
public:
	
	//Constructor / Destructor
	SMEventWatcher(void);
	virtual ~SMEventWatcher(void);
	
	//Initialize
	int Initialize(SMMsgTransmitter* pMsgTrans);
	
	//Event monitoring
	int WatchEvent(unsigned char portNo, SMEvent* pEvent);

	//Event monitoring: for sequencer
	int WatchEventMIDI(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int WatchEventControlChange(unsigned char portNo, SMEventMIDI* pMIDIEvent);

	//Note event post control (Playback: false, Live: true)
	void SetNoteEventPostEnabled(bool enabled) { m_isNoteEventPostEnabled = enabled; }

private:
	
	//RPN/NRPN selection state
	enum RPN_NRPN_Select {
		RPN_NULL,
		RPN,
		NRPN
	};
	
	//RPN type
	enum RPN_Type {
		RPN_None,
		PitchBendSensitivity,
		MasterFineTune,
		MasterCourseTune
	};
	
	//Message transmission control
	SMMsgTransmitter* m_pMsgTrans;
	bool m_isNoteEventPostEnabled;
	
	//Pitch bend control
	unsigned char m_PitchBendSensitivity[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	
	//RPN control related
	RPN_NRPN_Select m_RPN_NRPN_Select[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_RPN_MSB[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_RPN_LSB[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	
	void _ClearChInfo();
	int _WatchEventMIDI(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _WatchEventControlChange(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _WatchEventControlChange2(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	RPN_Type _GetCurRPNType(unsigned char portNo, unsigned char chNo);
	int _WatchEventSysMsg(unsigned char portNo, SMEventSysMsg* pEventSysMsg);
	
};

} // end of namespace


