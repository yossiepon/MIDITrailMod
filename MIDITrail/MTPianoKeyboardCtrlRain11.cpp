//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRain11
//
// DX11 piano keyboard controller for Rain scene.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrlRain11.h"
#include "MTPianoKeyboardFlat11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrlRain11::MTPianoKeyboardCtrlRain11()
{
	m_isPlaybackPosTracking = true;
}

MTPianoKeyboardCtrlRain11::~MTPianoKeyboardCtrlRain11()
{
}

//******************************************************************************
// Create
//******************************************************************************
int MTPianoKeyboardCtrlRain11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNoteTracker* pNoteTracker,
		MTNotePitchBend* pNotePitchBend,
		bool isSingleKeyboard
	)
{
	int result = 0;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	m_pNoteDesign = &m_NoteDesign;
	m_KeyDownDurMs = m_KeyboardDesign.GetKeyDownDuration();
	m_KeyUpDurMs = m_KeyboardDesign.GetKeyUpDuration();

	result = MTPianoKeyboardCtrl11::Create(pDevice, pContext, pSceneName, pSeqData,
		pNoteTracker, pNotePitchBend, isSingleKeyboard);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create keyboards (Rain: one MTPianoKeyboardFlat11 per channel)
//******************************************************************************
int MTPianoKeyboardCtrlRain11::_CreateKeyboards(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	unsigned long numKbd = m_isSingleKeyboard ? 1 : SM_MAX_CH_NUM;
	for (unsigned char chNo = 0; chNo < numKbd; chNo++) {
		m_Subs[chNo].pKeyboard = new MTPianoKeyboardFlat11();
		if (m_Subs[chNo].pKeyboard == NULL) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		result = m_Subs[chNo].pKeyboard->Create(pDevice, pContext, pSceneName, pSeqData, m_pSRV);
		if (result != 0) goto EXIT;
	}
	m_NumKbd = numKbd;

EXIT:;
	return result;
}

//******************************************************************************
// Per-key index params (Rain: no port filter, filter by channel)
//******************************************************************************
void MTPianoKeyboardCtrlRain11::_GetPerKeyIndexParams(
		unsigned long kbdIndex,
		int& outPortFilter,
		int& outChFilter
	)
{
	outPortFilter = -1;
	outChFilter = m_isSingleKeyboard ? -1 : (int)kbdIndex;
}

//******************************************************************************
// Active key color (Rain: from base MTPianoKeyboardDesign)
//******************************************************************************
void MTPianoKeyboardCtrlRain11::_ApplyActiveKeyColor(
		MTKbdSub* pSub,
		unsigned long kbdIndex
	)
{
	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		MTKeyboardKeyState& ks = pSub->keyStates[noteNo];
		if (ks.rate >= 1.0f) {
			Color noteColor((unsigned int)ks.color);
			Color activeColor = m_KeyboardDesign.GetActiveKeyColor(noteNo, 0, &noteColor);
			ks.color = activeColor.BGRA();
		}
	}
}

//******************************************************************************
// World matrix (Rain: simple roll + position)
//******************************************************************************
Matrix MTPianoKeyboardCtrlRain11::_ComputeWorldMatrix(
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	unsigned char chNo = (unsigned char)kbdIndex;
	unsigned char portNo = 0;
	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(portNo, chNo);
	if (m_isPlaybackPosTracking) {
		moveVec.y += m_NoteDesign.GetPlayPosX(ctx.curTickTime);
	}
	return Matrix::CreateTranslation(moveVec)
	     * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
}
