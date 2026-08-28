//******************************************************************************
//
// MIDITrail / MTNoteTrackerLive
//
// Note tracker (Live).
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteTrackerLive.h"
#include "RDDiagManager.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteTrackerLive::MTNoteTrackerLive()
{
	m_NextIndex = 0;
	m_LiveTimeMSec = 0;
}

MTNoteTrackerLive::~MTNoteTrackerLive()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteTrackerLive::Create()
{
	Release();
	return 0;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteTrackerLive::Release()
{
	m_ActiveNotes.clear();
	m_Listeners.clear();
	m_NextIndex = 0;
	m_LiveTimeMSec = 0;
}

//******************************************************************************
// Update (IMTSceneManagedComponent)
//******************************************************************************
int MTNoteTrackerLive::Update(const MTSceneUpdateContext& ctx)
{
	m_LiveTimeMSec = ctx.liveTimeMSec;
	return 0;
}

//******************************************************************************
// Reset (IMTSceneManagedComponent)
//******************************************************************************
void MTNoteTrackerLive::Reset()
{
	m_ActiveNotes.clear();
	m_NextIndex = 0;
	DispatchReset();
}

//******************************************************************************
// SetNoteOn
//******************************************************************************
void MTNoteTrackerLive::SetNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	LiveNoteEntry entry;
	entry.isActive = true;
	entry.index = m_NextIndex++;

	NoteData& nd = entry.note;
	nd.portNo = portNo;
	nd.chNo = chNo;
	nd.noteNo = noteNo;
	nd.velocity = velocity;
	nd.startTimeTick = 0;
	nd.endTimeTick = 0;
	nd.startTimeMs = m_LiveTimeMSec;
	nd.endTimeMs = 0;
	nd.lyric[0] = L'\0';

	m_ActiveNotes.push_back(entry);
	DispatchActivate(entry.note, entry.index);

	int64_t polyphony = RDDiagManager::GetInt(RDMetricId::AppPolyphony) + 1;
	RDDiagManager::SetInt(RDMetricId::AppPolyphony, polyphony);
	int64_t peak = RDDiagManager::GetInt(RDMetricId::AppPolyphonyPeak);
	if (polyphony > peak) {
		RDDiagManager::SetInt(RDMetricId::AppPolyphonyPeak, polyphony);
	}
}

//******************************************************************************
// SetNoteOff
//******************************************************************************
void MTNoteTrackerLive::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	for (auto& entry : m_ActiveNotes) {
		if (entry.isActive
			&& entry.note.portNo == portNo
			&& entry.note.chNo == chNo
			&& entry.note.noteNo == noteNo
			&& entry.note.endTimeMs == 0) {
			entry.note.endTimeMs = m_LiveTimeMSec;
			DispatchDeactivate(entry.note, entry.index);
			entry.isActive = false;
			int64_t polyphony = RDDiagManager::GetInt(RDMetricId::AppPolyphony);
			if (polyphony > 0) {
				RDDiagManager::SetInt(RDMetricId::AppPolyphony, polyphony - 1);
			}
			return;
		}
	}
}

//******************************************************************************
// AllNoteOff
//******************************************************************************
void MTNoteTrackerLive::AllNoteOff()
{
	for (auto& entry : m_ActiveNotes) {
		if (entry.isActive && entry.note.endTimeMs == 0) {
			entry.note.endTimeMs = m_LiveTimeMSec;
			DispatchDeactivate(entry.note, entry.index);
			entry.isActive = false;
		}
	}
}

//******************************************************************************
// AllNoteOffOnCh
//******************************************************************************
void MTNoteTrackerLive::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	for (auto& entry : m_ActiveNotes) {
		if (entry.isActive
			&& entry.note.portNo == portNo
			&& entry.note.chNo == chNo
			&& entry.note.endTimeMs == 0) {
			entry.note.endTimeMs = m_LiveTimeMSec;
			DispatchDeactivate(entry.note, entry.index);
			entry.isActive = false;
		}
	}
}
