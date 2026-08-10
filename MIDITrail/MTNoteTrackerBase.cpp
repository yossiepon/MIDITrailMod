//******************************************************************************
//
// MIDITrail / MTNoteTrackerBase
//
// Note tracker base class.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteTrackerBase.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteTrackerBase::MTNoteTrackerBase()
{
}

MTNoteTrackerBase::~MTNoteTrackerBase()
{
}

//******************************************************************************
// AddListener
//******************************************************************************
void MTNoteTrackerBase::AddListener(
		IMTNoteTrackerListener* pListener,
		NoteEventType filter
	)
{
	ListenerEntry entry;
	entry.pListener = pListener;
	entry.filter = filter;
	m_Listeners.push_back(entry);
}

//******************************************************************************
// RemoveListener
//******************************************************************************
void MTNoteTrackerBase::RemoveListener(
		IMTNoteTrackerListener* pListener
	)
{
	for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it) {
		if (it->pListener == pListener) {
			m_Listeners.erase(it);
			break;
		}
	}
}

//******************************************************************************
// Dispatch helpers
//******************************************************************************
void MTNoteTrackerBase::DispatchActivate(
		const NoteData& note,
		unsigned long index
	)
{
	NoteEventType eventType = (note.lyric[0] == L'\0') ? NoteEventType::Note : NoteEventType::Lyric;

	for (const auto& entry : m_Listeners) {
		if (entry.filter == eventType) {
			entry.pListener->OnNoteActivate(note, index);
		}
	}
}

void MTNoteTrackerBase::DispatchDeactivate(
		const NoteData& note,
		unsigned long index
	)
{
	NoteEventType eventType = (note.lyric[0] == L'\0') ? NoteEventType::Note : NoteEventType::Lyric;

	for (const auto& entry : m_Listeners) {
		if (entry.filter == eventType) {
			entry.pListener->OnNoteDeactivate(note, index);
		}
	}
}

void MTNoteTrackerBase::DispatchReset()
{
	for (const auto& entry : m_Listeners) {
		entry.pListener->OnReset();
	}
}
