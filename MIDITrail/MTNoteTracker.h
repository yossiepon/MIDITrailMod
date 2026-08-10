//******************************************************************************
//
// MIDITrail / MTNoteTracker
//
// Note tracker class (Playback mode).
// Holds unified note list, performs ms-based forward scan,
// and notifies listeners of note activation/deactivation events.
// Deactivation uses prefix-max binary search for O(log n) efficiency.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <vector>
#include "MTNoteTrackerBase.h"


//******************************************************************************
// Note tracker class (Playback)
//******************************************************************************
class MTNoteTracker : public MTNoteTrackerBase
{
public:

	MTNoteTracker();
	~MTNoteTracker();

	int Create(SMSeqData* pSeqData);
	void Release();

	int  Update(const MTSceneUpdateContext& ctx) override;
	void Reset() override;

	void Advance(unsigned long playTimeMSec);
	void Seek(unsigned long playTimeMSec);

	unsigned long GetNoteCount() const;
	const NoteData& GetNote(unsigned long index) const;

private:

	std::vector<NoteData> m_Notes;
	unsigned long m_CurNoteIndex;
	unsigned long m_DeactivationCursor;
	std::vector<unsigned long> m_MaxEndTimeMs;
	unsigned long m_PlayTimeMSec;

	void _BuildMaxEndTimeMs();
	unsigned long _FindDeactivationBoundary(unsigned long playTimeMSec) const;
};
