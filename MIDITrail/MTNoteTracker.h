//******************************************************************************
//
// MIDITrail / MTNoteTracker
//
// Note tracker (Playback).
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include <vector>
#include "MTNoteTrackerBase.h"
#include "MTLoadingDefs.h"


//******************************************************************************
// Note tracker class (Playback)
//******************************************************************************
class MTNoteTracker : public MTNoteTrackerBase
{
public:

	MTNoteTracker();
	~MTNoteTracker();

	int Create(SMSeqData* pSeqData,
	           const MTLoadProgressContext* pProgress = NULL);
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
