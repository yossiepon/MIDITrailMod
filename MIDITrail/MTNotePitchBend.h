//******************************************************************************
//
// MIDITrail / MTNotePitchBend
//
// Pitch bend data manager.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Holds pitch bend information per port/channel.

#pragma once

#include "SMCommon.h"
#include "IMTSceneManagedComponent.h"


//******************************************************************************
// Pitch bend info class
//******************************************************************************
class MTNotePitchBend : public IMTSceneManagedComponent
{
public:

	//Constructor / Destructor
	MTNotePitchBend(void);
	virtual ~MTNotePitchBend(void);

	//Initialize
	int Initialize();

	//Register pitch bend
	int SetPitchBend(
			unsigned char portNo,
			unsigned char chNo,
			short value,
			unsigned char sensitivity
		);

	//Get pitch bend value
	short GetValue(unsigned long portNo, unsigned long chNo);

	//Get pitch bend sensitivity
	unsigned char GetSensitivity(unsigned long portNo, unsigned long chNo);

	//Reset
	void Reset() override;

	//Set pitch bend display effect
	void SetEnable(bool isEnable);

private:

	//Pitch bend info
	struct MTNOTEPITCHBEND_PITCHBEND_INFO {
		short value;
		unsigned char sensitivity;
	};

private:

	//Pitch bend display effect
	bool m_isEnable;

	//Pitch bend info
	MTNOTEPITCHBEND_PITCHBEND_INFO m_PitchBend[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];

};

