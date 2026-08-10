//******************************************************************************
//
// MIDITrail / MTNoteTrackerLive
//
// Note tracker class (Live mode).
// Event-driven: SetNoteOn/Off dispatches Activate/Deactivate immediately.
// No pre-loaded note list. No forward scan.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <vector>
#include "MTNoteTrackerBase.h"


//******************************************************************************
// Note tracker class (Live)
//******************************************************************************
class MTNoteTrackerLive : public MTNoteTrackerBase
{
public:

	MTNoteTrackerLive();
	~MTNoteTrackerLive();

	int Create();
	void Release();

	int  Update(const MTSceneUpdateContext& ctx) override;
	void Reset() override;

	void SetNoteOn(unsigned char portNo, unsigned char chNo,
	               unsigned char noteNo, unsigned char velocity);
	void SetNoteOff(unsigned char portNo, unsigned char chNo,
	                unsigned char noteNo);
	void AllNoteOff();
	void AllNoteOffOnCh(unsigned char portNo, unsigned char chNo);

private:

	struct LiveNoteEntry {
		NoteData note;
		unsigned long index;
		bool isActive;
	};

	std::vector<LiveNoteEntry> m_ActiveNotes;
	unsigned long m_NextIndex;
	unsigned long m_LiveTimeMSec;
};
