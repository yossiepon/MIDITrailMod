//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlFlatLive11
//
// Piano keyboard controller (Flat, Live).
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrlFlatLive11.h"
#include "MTPianoKeyboardFlat11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrlFlatLive11::MTPianoKeyboardCtrlFlatLive11()
{
	m_LiveTimeMSec = 0;
	ZeroMemory(m_ActiveNoteCountPerKbd, sizeof(m_ActiveNoteCountPerKbd));
	ZeroMemory(m_ActiveNoteCountPerPortCh, sizeof(m_ActiveNoteCountPerPortCh));
	m_isPlaybackPosTracking = false;
	ZeroMemory(m_LiveKeys, sizeof(m_LiveKeys));
}

MTPianoKeyboardCtrlFlatLive11::~MTPianoKeyboardCtrlFlatLive11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTPianoKeyboardCtrlFlatLive11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		MTNotePitchBend* pNotePitchBend,
		bool isSingleKeyboard
	)
{
	int result = 0;

	MTPianoKeyboardCtrlBase11::Release();

	m_pContext = pContext;
	m_pNotePitchBend = pNotePitchBend;
	m_isSingleKeyboard = isSingleKeyboard;

	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	m_pNoteDesign = &m_NoteDesign;

	result = m_KeyboardDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;

	m_KeyDownDurMs = 0;
	m_KeyUpDurMs = m_KeyboardDesign.GetKeyUpDuration();

	result = _LoadTexture(pDevice, pSceneName);
	if (result != 0) goto EXIT;

	result = _CreateKeyboards(pDevice, pContext, pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create keyboards (one per channel, or single)
//******************************************************************************
int MTPianoKeyboardCtrlFlatLive11::_CreateKeyboards(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName
	)
{
	int result = 0;

	unsigned long numKbd = m_isSingleKeyboard ? 1 : SM_MAX_CH_NUM;
	for (unsigned long k = 0; k < numKbd; k++) {
		m_Subs[k].pKeyboard = new MTPianoKeyboardFlat11();
		if (m_Subs[k].pKeyboard == NULL) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}
		result = m_Subs[k].pKeyboard->Create(pDevice, pContext, pSceneName, NULL, m_pSRV);
		if (result != 0) goto EXIT;
	}
	m_NumKbd = numKbd;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::Release()
{
	MTPianoKeyboardCtrlBase11::Release();
}

//******************************************************************************
// _UpdateKeyStates (Live: evaluate live key states + update timestamp)
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::_UpdateKeyStates(
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	m_LiveTimeMSec = ctx.liveTimeMSec;
	_EvaluateLiveKeyStates(kbdIndex);
}

//******************************************************************************
// Evaluate live key states for a specific keyboard
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::_EvaluateLiveKeyStates(unsigned long kbdIndex)
{
	unsigned long upMs = m_KeyUpDurMs;

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		LiveKeyState& lk = m_LiveKeys[kbdIndex][noteNo];
		MTKeyboardKeyState& ks = m_Subs[kbdIndex].keyStates[noteNo];

		if (!lk.isDown && lk.endTimeMs == 0) {
			ks.rate = 0.0f;
			continue;
		}

		float rate;
		if (lk.isDown) {
			rate = 1.0f;
		}
		else {
			unsigned long elapsed = m_LiveTimeMSec - lk.endTimeMs;
			if (upMs > 0 && elapsed < upMs) {
				rate = 1.0f - (float)elapsed / (float)upMs;
			}
			else {
				rate = 0.0f;
				lk.endTimeMs = 0;
			}
		}

		ks.rate = rate;
		ks.color = lk.color;
		ks.chNo = lk.chNo;

		if (rate >= 1.0f) {
			Color noteColor((unsigned int)lk.color);
			Color activeColor = m_KeyboardDesign.GetActiveKeyColor(
				lk.chNo, noteNo, 0, &noteColor);
			ks.color = activeColor.BGRA();
		}
	}
}

//******************************************************************************
// Reset
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::Reset()
{
	ZeroMemory(m_LiveKeys, sizeof(m_LiveKeys));
	MTPianoKeyboardCtrlBase11::Reset();
}

//******************************************************************************
// IMTNoteTrackerListener: OnNoteActivate
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::OnNoteActivate(
		const NoteData& note,
		unsigned long index
	)
{
	if (note.noteNo >= SM_MAX_NOTE_NUM) return;

	unsigned long kbdIndex = m_isSingleKeyboard ? 0 : (note.chNo % m_NumKbd);
	LiveKeyState& lk = m_LiveKeys[kbdIndex][note.noteNo];
	if (!lk.isDown) {
		m_ActiveNoteCountPerKbd[kbdIndex]++;
		if (note.chNo < SM_MAX_CH_NUM) m_ActiveNoteCountPerPortCh[note.portNo][note.chNo]++;
	}
	lk.isDown = true;
	lk.startTimeMs = m_LiveTimeMSec;
	lk.endTimeMs = 0;
	lk.color = m_pNoteDesign->GetNoteBoxColor(note.portNo, note.chNo, note.noteNo).BGRA();
	lk.chNo = note.chNo;
}

//******************************************************************************
// IMTNoteTrackerListener: OnNoteDeactivate
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::OnNoteDeactivate(
		const NoteData& note,
		unsigned long index
	)
{
	if (note.noteNo >= SM_MAX_NOTE_NUM) return;

	unsigned long kbdIndex = m_isSingleKeyboard ? 0 : (note.chNo % m_NumKbd);
	LiveKeyState& lk = m_LiveKeys[kbdIndex][note.noteNo];
	if (lk.isDown) {
		lk.isDown = false;
		lk.endTimeMs = m_LiveTimeMSec;
		if (m_ActiveNoteCountPerKbd[kbdIndex] > 0) m_ActiveNoteCountPerKbd[kbdIndex]--;
		if (note.chNo < SM_MAX_CH_NUM && m_ActiveNoteCountPerPortCh[note.portNo][note.chNo] > 0) m_ActiveNoteCountPerPortCh[note.portNo][note.chNo]--;
	}
}

//******************************************************************************
// IMTNoteTrackerListener: OnReset
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::OnReset()
{
	ZeroMemory(m_LiveKeys, sizeof(m_LiveKeys));
	ZeroMemory(m_ActiveNoteCountPerKbd, sizeof(m_ActiveNoteCountPerKbd));
	ZeroMemory(m_ActiveNoteCountPerPortCh, sizeof(m_ActiveNoteCountPerPortCh));
	for (unsigned long k = 0; k < m_NumKbd; k++) {
		ZeroMemory(m_Subs[k].keyStates, sizeof(m_Subs[k].keyStates));
	}
}

//******************************************************************************
// GetActiveChannelMask
//******************************************************************************
unsigned short MTPianoKeyboardCtrlFlatLive11::GetActiveChannelMask(unsigned char portNo) const
{
	unsigned short mask = 0;
	for (unsigned char ch = 0; ch < SM_MAX_CH_NUM; ch++) {
		if (m_ActiveNoteCountPerPortCh[portNo][ch] > 0) mask |= (1 << ch);
	}
	return mask;
}
