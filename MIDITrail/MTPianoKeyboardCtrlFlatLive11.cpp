//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlFlatLive11
//
// Piano keyboard controller for Live mode (Flat, DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrlFlatLive11.h"
#include "MTPianoKeyboardRain11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrlFlatLive11::MTPianoKeyboardCtrlFlatLive11()
{
	m_LiveTimeMSec = 0;
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

	result = _CreateKeyboard(pDevice, pContext, pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create keyboard (single Rain-style keyboard for Live)
//******************************************************************************
int MTPianoKeyboardCtrlFlatLive11::_CreateKeyboard(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName
	)
{
	int result = 0;

	m_Subs[0].pKeyboard = new MTPianoKeyboardRain11();
	if (m_Subs[0].pKeyboard == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	result = m_Subs[0].pKeyboard->Create(pDevice, pContext, pSceneName, NULL, m_pSRV);
	if (result != 0) goto EXIT;

	m_NumKbd = 1;

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
// Update
//******************************************************************************
int MTPianoKeyboardCtrlFlatLive11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	int result = 0;

	m_LiveTimeMSec = ctx.liveTimeMSec;

	if (m_Subs[0].pKeyboard == NULL) goto EXIT;

	if (!m_isSkipping) {
		_EvaluateLiveKeyStates();
	}

	{
		Matrix world = _ComputeWorldMatrix(ctx);
		result = m_Subs[0].pKeyboard->Update(m_pContext, m_Subs[0].keyStates, world);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Evaluate live key states from LiveKeyState array
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::_EvaluateLiveKeyStates()
{
	unsigned long upMs = m_KeyUpDurMs;

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		LiveKeyState& lk = m_LiveKeys[noteNo];
		MTKeyboardKeyState& ks = m_Subs[0].keyStates[noteNo];

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
			Color activeColor = m_KeyboardDesign.GetActiveKeyColor(noteNo, 0, &noteColor);
			ks.color = activeColor.BGRA();
		}
	}
}

//******************************************************************************
// Compute world matrix (Rain-style: translation + rotation)
//******************************************************************************
Matrix MTPianoKeyboardCtrlFlatLive11::_ComputeWorldMatrix(
		const MTSceneUpdateContext& ctx
	)
{
	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(0, 0);
	return Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle))
	     * Matrix::CreateTranslation(moveVec);
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

	LiveKeyState& lk = m_LiveKeys[note.noteNo];
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

	LiveKeyState& lk = m_LiveKeys[note.noteNo];
	if (lk.isDown) {
		lk.isDown = false;
		lk.endTimeMs = m_LiveTimeMSec;
	}
}

//******************************************************************************
// IMTNoteTrackerListener: OnReset
//******************************************************************************
void MTPianoKeyboardCtrlFlatLive11::OnReset()
{
	ZeroMemory(m_LiveKeys, sizeof(m_LiveKeys));
	ZeroMemory(m_Subs[0].keyStates, sizeof(m_Subs[0].keyStates));
}
