//******************************************************************************
//
// MIDITrail / MTNoteEffect
//
// Note effect base class.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteEffect.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteEffect::MTNoteEffect()
{
	m_pNoteDesign = NULL;
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	m_RollAngle = 0.0f;
	m_isSkipping = false;
	ZeroMemory(m_Status, sizeof(m_Status));
	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));
}

MTNoteEffect::~MTNoteEffect()
{
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteEffect::Create(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNoteDesign11* pNoteDesign
	)
{
	int result = 0;

	if (pNoteDesign != NULL) {
		m_pNoteDesign = pNoteDesign;
	}
	else {
		result = m_NoteDesignLocal.Initialize(pSceneName, pSeqData);
		if (result != 0) goto EXIT;
		m_pNoteDesign = &m_NoteDesignLocal;
	}

	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	ZeroMemory(m_Status, sizeof(m_Status));
	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteEffect::Release()
{
	ZeroMemory(m_Status, sizeof(m_Status));
	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
}

//******************************************************************************
// OnNoteActivate (callback from NoteTracker)
//******************************************************************************
void MTNoteEffect::OnNoteActivate(
		const NoteData& note,
		unsigned long index
	)
{
	if (m_isSkipping) return;

	// Skip if already registered with same index
	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		if (m_Status[i].isActive && m_Status[i].index == index) {
			return;
		}
	}

	// Find empty slot
	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		if (!m_Status[i].isActive) {
			NoteEffectStatus& s = m_Status[i];
			s.isActive = true;
			s.keyStatus = BeforeNoteON;
			s.keyDownRate = 0.0f;
			s.index = index;
			s.portNo = note.portNo;
			s.chNo = note.chNo;
			s.noteNo = note.noteNo;
			s.velocity = note.velocity;
			s.startTimeMs = note.startTimeMs;
			s.endTimeMs = note.endTimeMs;
			memcpy(s.lyric, note.lyric, sizeof(s.lyric));

			OnActivate(s);
			return;
		}
	}

	// No empty slot: silently drop
}

//******************************************************************************
// OnNoteDeactivate (callback from NoteTracker on note end)
//******************************************************************************
void MTNoteEffect::OnNoteDeactivate(
		const NoteData& note,
		unsigned long index
	)
{
	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		if (m_Status[i].isActive && m_Status[i].index == index) {
			OnDeactivate(m_Status[i]);
			m_Status[i].isActive = false;
			return;
		}
	}
}

//******************************************************************************
// OnReset (callback from NoteTracker on seek)
//******************************************************************************
void MTNoteEffect::OnReset()
{
	m_isSkipping = false;

	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		if (m_Status[i].isActive) {
			OnDeactivate(m_Status[i]);
			m_Status[i].isActive = false;
		}
	}
}

//******************************************************************************
// Update (per-frame)
//******************************************************************************
int MTNoteEffect::Update(
		const MTSceneUpdateContext& ctx
	)
{
	int result = 0;

	m_CurTickTime = ctx.curTickTime;
	m_PlayTimeMSec = (ctx.liveTimeMSec != 0) ? ctx.liveTimeMSec : ctx.playTimeMSec;
	m_RollAngle = ctx.rollAngle;

	if (m_isSkipping) goto EXIT;

	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		if (!m_Status[i].isActive) continue;

		MTNoteEnvelopeResult env = m_pNoteDesign->CalcNoteEnvelope(
			m_PlayTimeMSec, m_Status[i].startTimeMs, m_Status[i].endTimeMs);
		m_Status[i].keyDownRate = env.keyDownRate;
		m_Status[i].keyStatus = env.keyStatus;
	}

	result = BuildVertices(m_PlayTimeMSec);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}
