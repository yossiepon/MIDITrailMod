//******************************************************************************
//
// MIDITrail / MTNoteLiveBase11
//
// Live note renderer base class (DX11).
// Manages NoteStatus array, SetNoteOn/Off events, and expiry logic.
// Subclasses (MTNoteAABBLive11, MTNoteCylindricalLive11) provide vertex
// generation and Draw.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "MTNotePitchBend.h"

//******************************************************************************
// Parameters
//******************************************************************************
#define MTNOTELIVENOTE_MAX_NUM  (2048)


//******************************************************************************
// Live note renderer base class
//******************************************************************************
class MTNoteLiveBase11 : public MTSceneComponent11
{
public:

	MTNoteLiveBase11();
	virtual ~MTNoteLiveBase11();

	// Note events (called from scene event handlers, use wall-clock time)
	void SetNoteOn(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo,
			unsigned char velocity
		);
	void SetNoteOff(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo
		);
	void AllNoteOff();
	void AllNoteOffOnCh(unsigned char portNo, unsigned char chNo);

	void Reset() override;

protected:

	//----------------------------------------------------------------------
	// Note status
	//----------------------------------------------------------------------
	struct NoteStatus {
		bool isActive;
		unsigned char portNo;
		unsigned char chNo;
		unsigned char noteNo;
		unsigned long startTime;
		unsigned long endTime;
	};

	NoteStatus m_NoteStatus[MTNOTELIVENOTE_MAX_NUM];
	unsigned long m_NoteNum;
	unsigned long m_LiveMonitorDisplayDuration;
	unsigned long m_LiveTimeMSec;

	// Pitch bend (not owned)
	MTNotePitchBend* m_pNotePitchBend;

	void _UpdateStatusOfNotes(unsigned long curTimeMs);
	void _ClearOldestNoteStatus(unsigned long* pClearedIndex);
	void _ResetNoteStatus();
};
