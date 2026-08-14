//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRoll11
//
// Piano keyboard controller for PianoRoll scenes.
//
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrlRoll11.h"
#include "MTPianoKeyboardFlat11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrlRoll11::MTPianoKeyboardCtrlRoll11()
{
	m_MaxKeyboardIndex = 0;
	ZeroMemory(m_KeyboardIndex, sizeof(m_KeyboardIndex));
	ZeroMemory(m_KbdPortNo, sizeof(m_KbdPortNo));
	for (int i = 0; i < SM_MAX_PORT_NUM; i++) {
		m_KeyboardIndex[i] = -1;
	}
}

MTPianoKeyboardCtrlRoll11::~MTPianoKeyboardCtrlRoll11()
{
}

//******************************************************************************
// Create
//******************************************************************************
int MTPianoKeyboardCtrlRoll11::Create(
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
	unsigned long index = 0;
	unsigned long keyboardIndex = 0;
	unsigned char portNo = 0;

	if (pSeqData == NULL || pNoteTracker == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	// Port -> keyboard index mapping
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_KeyboardIndex[index] = -1;
	}
	ZeroMemory(m_KbdPortNo, sizeof(m_KbdPortNo));

	if (!isSingleKeyboard) {
		for (index = 0; index < m_PortList.GetSize(); index++) {
			m_PortList.GetPort(index, &portNo);
			m_KeyboardIndex[portNo] = keyboardIndex;
			m_KbdPortNo[keyboardIndex] = portNo;
			keyboardIndex++;
			if (keyboardIndex == m_KeyboardDesign.GetKeyboardMaxDispNum()) break;
		}
		m_MaxKeyboardIndex = (unsigned char)keyboardIndex;
	}
	else {
		m_KeyboardDesign.SetKeyboardSingle();
		m_KeyboardIndex[0] = 0;
		m_KbdPortNo[0] = 0;
		m_MaxKeyboardIndex = 1;
	}

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
// Create keyboards (one MTPianoKeyboardFlat11 per active port)
//******************************************************************************
int MTPianoKeyboardCtrlRoll11::_CreateKeyboards(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	for (unsigned char index = 0; index < m_MaxKeyboardIndex; index++) {
		m_Subs[index].pKeyboard = new MTPianoKeyboardFlat11();
		if (m_Subs[index].pKeyboard == NULL) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		result = m_Subs[index].pKeyboard->Create(pDevice, pContext, pSceneName, pSeqData, m_pSRV);
		if (result != 0) goto EXIT;
	}
	m_NumKbd = m_MaxKeyboardIndex;

EXIT:;
	return result;
}

//******************************************************************************
// Per-key index params (Roll: filter by port, no channel filter)
//******************************************************************************
void MTPianoKeyboardCtrlRoll11::_GetPerKeyIndexParams(
		unsigned long kbdIndex,
		int& outPortFilter,
		int& outChFilter
	)
{
	outPortFilter = m_isSingleKeyboard ? -1 : (int)m_KbdPortNo[kbdIndex];
	outChFilter = -1;
}

//******************************************************************************
// Active key color (Roll: from DesignMod with channel parameter)
//******************************************************************************
void MTPianoKeyboardCtrlRoll11::_ApplyActiveKeyColor(
		MTKbdSub* pSub,
		unsigned long kbdIndex
	)
{
	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		MTKeyboardKeyState& ks = pSub->keyStates[noteNo];
		if (ks.rate >= 1.0f) {
			Color noteColor((unsigned int)ks.color);
			Color activeColor = m_KeyboardDesign.GetActiveKeyColor(ks.chNo, noteNo, 0, &noteColor);
			ks.color = activeColor.BGRA();
		}
	}
}

//******************************************************************************
// World matrix (Roll: model orientation baked in keyboard, simplified)
//******************************************************************************
Matrix MTPianoKeyboardCtrlRoll11::_ComputeWorldMatrix(
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	unsigned char portNo = m_KbdPortNo[kbdIndex];

	Vector3 playbackPos = m_NoteDesign.GetWorldMoveVector();
	playbackPos.x += m_NoteDesign.GetPlayPosX(ctx.curTickTime);

	Vector3 basePos = m_KeyboardDesign.GetKeyboardBasePos((int)kbdIndex, ctx.rollAngle);
	if (m_isSingleKeyboard) {
		basePos.x += _GetMaxPitchBendShiftAllPorts(&m_PortList);
	}
	else {
		basePos.x += m_pNoteDesign->GetMaxPitchBendShift(m_pNotePitchBend, portNo,
		             m_pNoteTracker->GetActiveChannelMask(portNo));
	}

	float scale = m_KeyboardDesign.GetKeyboardResizeRatio();

	float rollAngle = ctx.rollAngle;
	if (rollAngle < 0.0f) rollAngle += 360.0f;

	Matrix rotateMatrix1, rotateMatrix2, rotateMatrix3;
	if ((rollAngle > 120.0f) && (rollAngle < 300.0f)) {
		rotateMatrix1 = Matrix::CreateRotationX(XM_PI / 2.0f);
		rotateMatrix2 = Matrix::CreateRotationZ(XM_PI / 2.0f);
	}
	else {
		rotateMatrix1 = Matrix::CreateRotationX(-XM_PI / 2.0f);
		rotateMatrix2 = Matrix::CreateRotationZ(XM_PI / 2.0f);
	}
	rotateMatrix3 = Matrix::CreateRotationX(XMConvertToRadians(rollAngle));

	return Matrix::CreateScale(scale)
	     * Matrix::CreateTranslation(basePos)
	     * rotateMatrix1
	     * rotateMatrix2
	     * rotateMatrix3
	     * Matrix::CreateTranslation(playbackPos);
}


