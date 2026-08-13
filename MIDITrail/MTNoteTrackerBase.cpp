//******************************************************************************
//
// MIDITrail / MTNoteTrackerBase
//
// Note tracker base class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteTrackerBase.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteTrackerBase::MTNoteTrackerBase()
{
	memset(m_ActiveNotesPerCh, 0, sizeof(m_ActiveNotesPerCh));
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
bool MTNoteTrackerBase::HasActiveNotesOnPort(unsigned char portNo) const
{
	for (unsigned char ch = 0; ch < SM_MAX_CH_NUM; ch++) {
		if (m_ActiveNotesPerCh[portNo][ch] > 0) return true;
	}
	return false;
}

bool MTNoteTrackerBase::HasActiveNotesOnChannel(unsigned char portNo, unsigned char chNo) const
{
	return m_ActiveNotesPerCh[portNo][chNo] > 0;
}

unsigned short MTNoteTrackerBase::GetActiveChannelMask(unsigned char portNo) const
{
	unsigned short mask = 0;
	for (unsigned char ch = 0; ch < SM_MAX_CH_NUM; ch++) {
		if (m_ActiveNotesPerCh[portNo][ch] > 0) mask |= (1 << ch);
	}
	return mask;
}

void MTNoteTrackerBase::DispatchActivate(
		const NoteData& note,
		unsigned long index
	)
{
	if (note.lyric[0] == L'\0') {
		m_ActiveNotesPerCh[note.portNo][note.chNo]++;
	}
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
	if (note.lyric[0] == L'\0' && m_ActiveNotesPerCh[note.portNo][note.chNo] > 0) {
		m_ActiveNotesPerCh[note.portNo][note.chNo]--;
	}
	NoteEventType eventType = (note.lyric[0] == L'\0') ? NoteEventType::Note : NoteEventType::Lyric;

	for (const auto& entry : m_Listeners) {
		if (entry.filter == eventType) {
			entry.pListener->OnNoteDeactivate(note, index);
		}
	}
}

void MTNoteTrackerBase::DispatchReset()
{
	memset(m_ActiveNotesPerCh, 0, sizeof(m_ActiveNotesPerCh));
	for (const auto& entry : m_Listeners) {
		entry.pListener->OnReset();
	}
}
