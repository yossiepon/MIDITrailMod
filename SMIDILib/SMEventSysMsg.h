//******************************************************************************
//
// Simple MIDI Library / SMEventSysMsg
//
// System message event class.
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Ideally this would be designed as a subclass of the event class, but that
// would greatly increase the number of "new" calls, so it is implemented
// as a data-parsing utility class that can be processed on the stack.

// List of system messages handled by this class
//   F1 dd     System common message: Quarter frame (MTC)
//   F2 dl dm  System common message: Song position pointer
//   F3 dd     System common message: Song select
//   F4 Undefined
//   F5 Undefined
//   F6 System common message: Tune request
//   F8 System realtime message: Timing clock
//   F9 Undefined
//   FA System realtime message: Start
//   FB System realtime message: Continue
//   FC System realtime message: Stop
//   FD Undefined
//   FE System realtime message: Active sensing
//   FF System realtime message: System reset
// The following messages are outside the scope of this class
//   F0 ... F7 System exclusive
//   F7 End of system exclusive

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"


namespace SMIDILib {

//******************************************************************************
// System message event class
//******************************************************************************
class SMIDILIB_API SMEventSysMsg
{
public:

	//System message type
	enum SysMsg {
		None						= 0x00, // none
		//System common message
		Common_QuarterFrame			= 0xF1, // F1 dd
		Common_SongPositionPointer	= 0xF2, // F2 dl dm
		Common_SongSelect			= 0xF3, // F3 dd
		Common_TuneRequest			= 0xF6, // F6
		//System realtime message
		RealTime_TimingClock		= 0xF8, // F8
		RealTime_Start				= 0xFA, // FA
		RealTime_Continue			= 0xFB, // FB
		RealTime_Stop				= 0xFC, // FC
		RealTime_ActiveSensing		= 0xFE, // FE
		RealTime_SystemReset		= 0xFF  // FF
	};

public:

	//Constructor / Destructor
	SMEventSysMsg();
	virtual ~SMEventSysMsg(void);
	
	//Attach event
	void Attach(SMEvent* pEvent);
	
	//Get MIDI output short message
	int GetMIDIOutShortMsg(unsigned long* pMsg, unsigned long* pSize);
	
	//Get system message
	SysMsg GetSysMsg();
	
private:
	
	SMEvent* m_pEvent;
	
	//Prohibit assignment and copy constructor
	void operator=(const SMEventSysMsg&);
	SMEventSysMsg(const SMEventSysMsg&);
	
};

} // end of namespace


