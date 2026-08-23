//******************************************************************************
//
// MIDITrail / MTNoteTrackerBase
//
// Note tracker base class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteTrackerBase.h"
#include "RDDiagManager.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteTrackerBase::MTNoteTrackerBase()
	: m_TotalActiveNotes(0)
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

unsigned long MTNoteTrackerBase::GetTotalActiveNoteCount() const
{
	unsigned long total = 0;
	for (unsigned char port = 0; port < SM_MAX_PORT_NUM; port++) {
		for (unsigned char ch = 0; ch < SM_MAX_CH_NUM; ch++) {
			total += m_ActiveNotesPerCh[port][ch];
		}
	}
	return total;
}

void MTNoteTrackerBase::SetNoteCount(unsigned long noteCount)
{
	m_ActivatedFlags.assign(noteCount, 0);
}

void MTNoteTrackerBase::DispatchActivate(
		const NoteData& note,
		unsigned long index
	)
{
	if (note.lyric[0] == L'\0') {
		if (index < m_ActivatedFlags.size()) {
			m_ActivatedFlags[index] = 1;
		}
		m_ActiveNotesPerCh[note.portNo][note.chNo]++;
		m_TotalActiveNotes++;

		RDDiagManager::SetInt(RDMetricId::AppNoteTracking, static_cast<int64_t>(m_TotalActiveNotes));
		int64_t peak = RDDiagManager::GetInt(RDMetricId::AppNoteTrackingPeak);
		if (static_cast<int64_t>(m_TotalActiveNotes) > peak) {
			RDDiagManager::SetInt(RDMetricId::AppNoteTrackingPeak, static_cast<int64_t>(m_TotalActiveNotes));
		}

		int64_t activations = RDDiagManager::GetInt(RDMetricId::AppNoteActivationsPerFrame);
		RDDiagManager::SetInt(RDMetricId::AppNoteActivationsPerFrame, activations + 1);
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
	bool shouldDecrement = false;
	if (note.lyric[0] == L'\0') {
		if (m_ActivatedFlags.empty()) {
			shouldDecrement = (m_ActiveNotesPerCh[note.portNo][note.chNo] > 0);
		} else {
			shouldDecrement = (index < m_ActivatedFlags.size() && m_ActivatedFlags[index] != 0);
			if (shouldDecrement) {
				m_ActivatedFlags[index] = 0;
			}
		}
	}

	if (shouldDecrement) {
		if (m_ActiveNotesPerCh[note.portNo][note.chNo] > 0) {
			m_ActiveNotesPerCh[note.portNo][note.chNo]--;
		}
		if (m_TotalActiveNotes > 0) {
			m_TotalActiveNotes--;
		}
		RDDiagManager::SetInt(RDMetricId::AppNoteTracking, static_cast<int64_t>(m_TotalActiveNotes));
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
	m_TotalActiveNotes = 0;
	std::fill(m_ActivatedFlags.begin(), m_ActivatedFlags.end(), static_cast<uint8_t>(0));
	RDDiagManager::SetInt(RDMetricId::AppNoteTracking, 0);
	RDDiagManager::SetInt(RDMetricId::AppNoteTrackingPeak, 0);
	for (const auto& entry : m_Listeners) {
		entry.pListener->OnReset();
	}
}
