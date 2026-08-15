//******************************************************************************
//
// MIDITrail / MTNoteTracker
//
// Note tracker (Playback).
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteTracker.h"

using namespace YNBaseLib;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteTracker::MTNoteTracker()
{
	m_CurNoteIndex = 0;
	m_DeactivationCursor = 0;
	m_PlayTimeMSec = 0;
}

MTNoteTracker::~MTNoteTracker()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteTracker::Create(
		SMSeqData* pSeqData,
		const MTLoadProgressContext* pProgress
	)
{
	int result = 0;

	SMTrack mergedTrack;
	SMNoteList noteListTick;
	SMNoteList noteListMs;
	unsigned long noteCount = 0;

	Release();

	if (pSeqData == NULL) {
		goto EXIT;
	}

	result = pSeqData->GetMergedTrack(&mergedTrack);
	if (result != 0) goto EXIT;

	result = mergedTrack.GetNoteList(&noteListTick);
	if (result != 0) goto EXIT;

	result = mergedTrack.GetNoteListWithRealTime(&noteListMs, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

	noteCount = noteListTick.GetSize();
	m_Notes.resize(noteCount);

	for (unsigned long i = 0; i < noteCount; i++) {
		SMNote noteTick;
		result = noteListTick.GetNote(i, &noteTick);
		if (result != 0) goto EXIT;

		SMNote noteMs;
		result = noteListMs.GetNote(i, &noteMs);
		if (result != 0) goto EXIT;

		NoteData& nd = m_Notes[i];
		nd.portNo = noteTick.portNo;
		nd.chNo = noteTick.chNo;
		nd.noteNo = noteTick.noteNo;
		nd.velocity = noteTick.velocity;
		nd.startTimeTick = noteTick.startTime;
		nd.endTimeTick = noteTick.endTime;
		nd.startTimeMs = noteMs.startTime;
		nd.endTimeMs = noteMs.endTime;
		memcpy(nd.lyric, noteTick.lyric, sizeof(nd.lyric));

		if (pProgress != NULL && (i & 0x3FFF) == 0) {
			pProgress->Fire(i, noteCount);
		}
	}

	_BuildMaxEndTimeMs();

	m_CurNoteIndex = 0;
	m_DeactivationCursor = 0;
	m_PlayTimeMSec = 0;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteTracker::Release()
{
	m_Notes.clear();
	m_Notes.shrink_to_fit();
	m_MaxEndTimeMs.clear();
	m_MaxEndTimeMs.shrink_to_fit();
	m_Listeners.clear();
	m_CurNoteIndex = 0;
	m_DeactivationCursor = 0;
	m_PlayTimeMSec = 0;
}

//******************************************************************************
// Update (IMTSceneManagedComponent)
//******************************************************************************
int MTNoteTracker::Update(const MTSceneUpdateContext& ctx)
{
	Advance(ctx.playTimeMSec);
	return 0;
}

//******************************************************************************
// Reset (IMTSceneManagedComponent)
//******************************************************************************
void MTNoteTracker::Reset()
{
	Seek(0);
}

//******************************************************************************
// Advance (per-frame forward scan + deactivation check)
//******************************************************************************
void MTNoteTracker::Advance(
		unsigned long playTimeMSec
	)
{
	m_PlayTimeMSec = playTimeMSec;

	unsigned long noteCount = (unsigned long)m_Notes.size();

	// Activation: forward scan from cursor
	while (m_CurNoteIndex < noteCount) {
		const NoteData& note = m_Notes[m_CurNoteIndex];

		if (playTimeMSec < note.startTimeMs) {
			break;
		}

		if (playTimeMSec >= note.startTimeMs && playTimeMSec <= note.endTimeMs) {
			DispatchActivate(note, m_CurNoteIndex);
		}

		m_CurNoteIndex++;
	}

	// Deactivation: prefix-max binary search for boundary
	unsigned long newBoundary = _FindDeactivationBoundary(playTimeMSec);
	for (unsigned long i = m_DeactivationCursor; i < newBoundary; i++) {
		DispatchDeactivate(m_Notes[i], i);
	}
	m_DeactivationCursor = newBoundary;
}

//******************************************************************************
// Seek
//******************************************************************************
void MTNoteTracker::Seek(
		unsigned long playTimeMSec
	)
{
	DispatchReset();

	m_CurNoteIndex = 0;
	m_DeactivationCursor = 0;
	m_PlayTimeMSec = playTimeMSec;

	unsigned long noteCount = (unsigned long)m_Notes.size();

	// Re-notify currently active notes
	for (unsigned long i = 0; i < noteCount; i++) {
		const NoteData& note = m_Notes[i];

		if (playTimeMSec >= note.startTimeMs && playTimeMSec <= note.endTimeMs) {
			DispatchActivate(note, i);
		}
	}

	// Set cursor to the first note whose startTimeMs > playTimeMSec
	m_CurNoteIndex = 0;
	for (unsigned long i = 0; i < noteCount; i++) {
		if (m_Notes[i].startTimeMs > playTimeMSec) {
			m_CurNoteIndex = i;
			break;
		}
		m_CurNoteIndex = i + 1;
	}

	// Set deactivation cursor
	m_DeactivationCursor = _FindDeactivationBoundary(playTimeMSec);
}

//******************************************************************************
// Note data access
//******************************************************************************
unsigned long MTNoteTracker::GetNoteCount() const
{
	return (unsigned long)m_Notes.size();
}

const NoteData& MTNoteTracker::GetNote(
		unsigned long index
	) const
{
	return m_Notes[index];
}

//******************************************************************************
// Build prefix-max array of endTimeMs
//******************************************************************************
void MTNoteTracker::_BuildMaxEndTimeMs()
{
	unsigned long noteCount = (unsigned long)m_Notes.size();
	m_MaxEndTimeMs.resize(noteCount);

	if (noteCount == 0) return;

	m_MaxEndTimeMs[0] = m_Notes[0].endTimeMs;
	for (unsigned long i = 1; i < noteCount; i++) {
		m_MaxEndTimeMs[i] = (m_Notes[i].endTimeMs > m_MaxEndTimeMs[i - 1])
			? m_Notes[i].endTimeMs : m_MaxEndTimeMs[i - 1];
	}
}

//******************************************************************************
// Binary search for deactivation boundary
// Returns the first index where m_MaxEndTimeMs[index] >= playTimeMSec,
// meaning all notes before this index have ended.
//******************************************************************************
unsigned long MTNoteTracker::_FindDeactivationBoundary(
		unsigned long playTimeMSec
	) const
{
	unsigned long noteCount = (unsigned long)m_MaxEndTimeMs.size();
	if (noteCount == 0) return 0;

	unsigned long lo = 0;
	unsigned long hi = noteCount;

	while (lo < hi) {
		unsigned long mid = lo + (hi - lo) / 2;
		if (m_MaxEndTimeMs[mid] < playTimeMSec) {
			lo = mid + 1;
		}
		else {
			hi = mid;
		}
	}

	return lo;
}
