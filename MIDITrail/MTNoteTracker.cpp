//******************************************************************************
//
// MIDITrail / MTNoteTracker
//
// Note tracker class.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
	m_PlayTimeMSec = 0;
	m_MaxPreMargin = 0;
}

MTNoteTracker::~MTNoteTracker()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteTracker::Create(
		SMSeqData* pSeqData
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

	// Get merged track
	result = pSeqData->GetMergedTrack(&mergedTrack);
	if (result != 0) goto EXIT;

	// Get tick-based and ms-based note lists (same index = same note)
	result = mergedTrack.GetNoteList(&noteListTick);
	if (result != 0) goto EXIT;

	result = mergedTrack.GetNoteListWithRealTime(&noteListMs, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

	// Build unified note data
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
	}

	m_CurNoteIndex = 0;
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
	m_Listeners.clear();
	m_CurNoteIndex = 0;
	m_PlayTimeMSec = 0;
	m_MaxPreMargin = 0;
}

//******************************************************************************
// Update (per-frame forward scan)
//******************************************************************************
void MTNoteTracker::Update(
		unsigned long playTimeMSec
	)
{
	m_PlayTimeMSec = playTimeMSec;

	unsigned long noteCount = (unsigned long)m_Notes.size();

	while (m_CurNoteIndex < noteCount) {
		const NoteData& note = m_Notes[m_CurNoteIndex];

		// Stop when beyond the pre-margin lookahead range
		if (playTimeMSec < note.startTimeMs - m_MaxPreMargin) {
			// Guard against unsigned underflow
			if (note.startTimeMs > m_MaxPreMargin) {
				break;
			}
		}

		// Notify matching listeners
		NoteEventType eventType = (note.lyric[0] == L'\0') ? NoteEventType::Note : NoteEventType::Lyric;

		for (const auto& entry : m_Listeners) {
			if (entry.filter != eventType) continue;

			unsigned long marginStart = (note.startTimeMs > entry.preMarginMs)
				? (note.startTimeMs - entry.preMarginMs) : 0;
			unsigned long marginEnd = note.endTimeMs + entry.postMarginMs;

			if (playTimeMSec >= marginStart && playTimeMSec <= marginEnd) {
				entry.pListener->OnNoteActivate(note, m_CurNoteIndex);
			}
		}

		// Advance cursor only when past startTimeMs
		if (playTimeMSec >= note.startTimeMs) {
			m_CurNoteIndex++;
		}
		else {
			break;
		}
	}
}

//******************************************************************************
// Seek
//******************************************************************************
void MTNoteTracker::Seek(
		unsigned long playTimeMSec
	)
{
	// Reset all listeners
	for (const auto& entry : m_Listeners) {
		entry.pListener->OnReset();
	}

	m_CurNoteIndex = 0;
	m_PlayTimeMSec = playTimeMSec;

	unsigned long noteCount = (unsigned long)m_Notes.size();

	// Re-notify currently active notes
	for (unsigned long i = 0; i < noteCount; i++) {
		const NoteData& note = m_Notes[i];

		NoteEventType eventType = (note.lyric[0] == L'\0') ? NoteEventType::Note : NoteEventType::Lyric;

		for (const auto& entry : m_Listeners) {
			if (entry.filter != eventType) continue;

			unsigned long marginStart = (note.startTimeMs > entry.preMarginMs)
				? (note.startTimeMs - entry.preMarginMs) : 0;
			unsigned long marginEnd = note.endTimeMs + entry.postMarginMs;

			if (playTimeMSec >= marginStart && playTimeMSec <= marginEnd) {
				entry.pListener->OnNoteActivate(note, i);
			}
		}
	}

	// Set cursor to the first note whose startTimeMs > playTimeMSec - m_MaxPreMargin
	unsigned long targetTime = (playTimeMSec > m_MaxPreMargin)
		? (playTimeMSec - m_MaxPreMargin) : 0;

	m_CurNoteIndex = 0;
	for (unsigned long i = 0; i < noteCount; i++) {
		if (m_Notes[i].startTimeMs > targetTime) {
			m_CurNoteIndex = i;
			break;
		}
		m_CurNoteIndex = i + 1;
	}
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
// Listener management
//******************************************************************************
void MTNoteTracker::AddListener(
		IMTNoteTrackerListener* pListener,
		NoteEventType filter,
		unsigned long preMarginMs,
		unsigned long postMarginMs
	)
{
	ListenerEntry entry;
	entry.pListener = pListener;
	entry.filter = filter;
	entry.preMarginMs = preMarginMs;
	entry.postMarginMs = postMarginMs;

	m_Listeners.push_back(entry);
	_UpdateMaxPreMargin();
}

void MTNoteTracker::RemoveListener(
		IMTNoteTrackerListener* pListener
	)
{
	for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it) {
		if (it->pListener == pListener) {
			m_Listeners.erase(it);
			break;
		}
	}
	_UpdateMaxPreMargin();
}

//******************************************************************************
// Update max pre-margin
//******************************************************************************
void MTNoteTracker::_UpdateMaxPreMargin()
{
	m_MaxPreMargin = 0;
	for (const auto& entry : m_Listeners) {
		if (entry.preMarginMs > m_MaxPreMargin) {
			m_MaxPreMargin = entry.preMarginMs;
		}
	}
}
