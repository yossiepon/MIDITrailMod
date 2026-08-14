//******************************************************************************
//
// Simple MIDI Library / SMEventSysEx
//
// System exclusive (SysEx) event class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Ideally this would derive from the event class, but that would greatly
// increase the number of new calls, so it is implemented as a stack-based

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"

namespace SMIDILib {


//******************************************************************************
// SysEx event class
//******************************************************************************
class SMIDILIB_API SMEventSysEx
{
public:

	//Constructor / Destructor
	SMEventSysEx();
	virtual ~SMEventSysEx(void);

	//Attach event
	void Attach(SMEvent* pEvent);

	//MIDI outputGet message
	int GetMIDIOutLongMsg(unsigned char** pPtrMsg, unsigned long* pSize);

private:

	SMEvent* m_pEvent;

	//Prohibit assignment and copy constructor
	void operator=(const SMEventSysEx&);
	SMEventSysEx(const SMEventSysEx&);

};

} // end of namespace

