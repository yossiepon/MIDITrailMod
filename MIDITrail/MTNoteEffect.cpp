//******************************************************************************
//
// MIDITrail / MTNoteEffect
//
// Note effect base class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
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
	_InitFreeList();
}

MTNoteEffect::~MTNoteEffect()
{
}

//******************************************************************************
// Free-list management
//******************************************************************************
void MTNoteEffect::_InitFreeList()
{
	m_FreeCount = NOTEEFFECT_MAX_SLOTS;
	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		m_FreeStack[i] = NOTEEFFECT_MAX_SLOTS - 1 - i;
	}
}

void MTNoteEffect::_ReleaseSlot(int slotIndex)
{
	m_Status[slotIndex].isActive = false;
	m_FreeStack[m_FreeCount] = slotIndex;
	m_FreeCount++;
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
	_InitFreeList();

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
	_InitFreeList();
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

	// Pop from free-list
	if (m_FreeCount <= 0) return;

	int slotIndex = m_FreeStack[--m_FreeCount];
	NoteEffectStatus& s = m_Status[slotIndex];
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
			if (m_Status[i].endTimeMs == 0) {
				m_Status[i].endTimeMs = note.endTimeMs;
			}
			else {
				_ReleaseSlot(i);
			}
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
			_ReleaseSlot(i);
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

		// Release slots for notes past their end time + release duration
		// (bypasses prefix-max deactivation barrier).
		// Live mode has endTimeMs==0 until NoteOff, so this is safe for both modes.
		if (m_Status[i].endTimeMs != 0) {
			unsigned long releaseEnd = m_Status[i].endTimeMs
				+ m_pNoteDesign->GetRippleReleaseDuration();
			if (m_PlayTimeMSec > releaseEnd) {
				_ReleaseSlot(i);
				continue;
			}
		}

		MTNoteEnvelopeResult env = m_pNoteDesign->CalcNoteEnvelope(
			m_PlayTimeMSec, m_Status[i].startTimeMs, m_Status[i].endTimeMs);
		m_Status[i].keyDownRate = env.keyDownRate;
		m_Status[i].keyStatus = env.keyStatus;

		if (env.keyDownRate == 0.0f && env.keyStatus == AfterNoteOFF) {
			_ReleaseSlot(i);
		}
	}

	result = BuildVertices(m_PlayTimeMSec);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}
