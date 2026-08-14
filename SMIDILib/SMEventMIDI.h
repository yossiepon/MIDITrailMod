//******************************************************************************
//
// Simple MIDI Library / SMEventMIDI
//
// MIDI channel-message event class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Ideally this would derive from the event class, but that would greatly increase the number of new calls,
// so it's implemented as a stack-based data parsing utility class instead.

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"


namespace SMIDILib {

//******************************************************************************
// MIDI event class
//******************************************************************************
class SMIDILIB_API SMEventMIDI
{
public:

	//Channel message type
	enum ChMsg {
		None					= 0x00, // none
		NoteOff					= 0x80, // 8n kk vv
		NoteOn					= 0x90, // 9n kk vv
		PolyphonicKeyPressure	= 0xA0, // An kk vv
		ControlChange			= 0xB0, // Bn cc vv
		ProgramChange			= 0xC0, // Cn pp   
		ChannelPressure			= 0xD0, // Dn vv   
		PitchBend				= 0xE0  // En mm ll
	};

public:

	//Constructor / Destructor
	SMEventMIDI();
	virtual ~SMEventMIDI(void);

	//Attach event
	void Attach(SMEvent* pEvent);

	//MIDI outputGet message
	int GetMIDIOutShortMsg(unsigned long* pMsg);

	//Channel message
	ChMsg GetChMsg();

	//Get channel number
	unsigned char GetChNo();

	//Get note number
	unsigned char GetNoteNo();

	//Get velocity
	unsigned char GetVelocity();

	//Get control change number
	unsigned char GetCCNo();

	//Get control change value
	unsigned char GetCCValue();

	//Get program number
	unsigned char GetProgramNo();

	//Get channel pressure value
	unsigned char GetPressureValue();

	//Get pitch bend value
	short GetPitchBendValue();

private:

	SMEvent* m_pEvent;

	//Prohibit assignment and copy constructor
	void operator=(const SMEventMIDI&);
	SMEventMIDI(const SMEventMIDI&);

};

} // end of namespace

