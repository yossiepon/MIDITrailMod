//******************************************************************************
//
// Simple MIDI Library / SMEventMeta
//
// Meta event class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Ideally this would derive from the event class, but that would greatly
// increase the number of new calls, so it is implemented as a stack-based
// data parsing utility class instead.

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include <string>

namespace SMIDILib {


//******************************************************************************
// Meta event class
//******************************************************************************
class SMIDILIB_API SMEventMeta
{
public:

	//Constructor / Destructor
	SMEventMeta();
	~SMEventMeta(void);

	//Attach event
	void Attach(SMEvent* pEvent);

	//Get meta type
	unsigned char GetType();

	//Get tempo
	unsigned long GetTempo();

	//Get tempo(BPM)
	unsigned long GetTempoBPM();

	//Get text
	int GetText(std::string* pText);

	//Get port number
	unsigned char GetPortNo();

	//Get time signature
	void GetTimeSignature(unsigned long* pNumerator, unsigned long* pDenominator);

private:

	SMEvent* m_pEvent;

	//Prohibit assignment and copy constructor
	void operator=(const SMEventMeta&);
	SMEventMeta(const SMEventMeta&);

};

} // end of namespace

