//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlMod11
//
// DX11 piano keyboard controller Mod.
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrlMod11.h"
#include "MTPianoKeyboardMod11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrlMod11::MTPianoKeyboardCtrlMod11()
{
	m_MaxKeyboardIndex = 0;
	ZeroMemory(m_KeyboardIndex, sizeof(m_KeyboardIndex));
	for (int i = 0; i < SM_MAX_PORT_NUM; i++) {
		m_KeyboardIndex[i] = -1;
	}
}

MTPianoKeyboardCtrlMod11::~MTPianoKeyboardCtrlMod11()
{
}

//******************************************************************************
// Create
//******************************************************************************
int MTPianoKeyboardCtrlMod11::Create(
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

	// Mod design initialization
	result = m_NoteDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = m_DesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	// Port list
	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	// Port → keyboard index mapping
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_KeyboardIndex[index] = -1;
	}

	if (!isSingleKeyboard) {
		for (index = 0; index < m_PortList.GetSize(); index++) {
			m_PortList.GetPort(index, &portNo);
			m_KeyboardIndex[portNo] = keyboardIndex;
			keyboardIndex++;
			if (keyboardIndex == m_DesignMod.GetKeyboardMaxDispNum()) break;
		}
		m_MaxKeyboardIndex = (unsigned char)keyboardIndex;
	}
	else {
		m_DesignMod.SetKeyboardSingle();
		m_KeyboardIndex[0] = 0;
		m_MaxKeyboardIndex = 1;
	}

	// Base class Create (NoteDesign, KeyboardDesign, texture, keyboards, per-key index)
	result = MTPianoKeyboardCtrl11::Create(pDevice, pContext, pSceneName, pSeqData,
		pNoteTracker, pNotePitchBend, isSingleKeyboard);
	if (result != 0) goto EXIT;

	// Rebuild per-key index per port
	for (unsigned char k = 0; k < m_MaxKeyboardIndex; k++) {
		// Find port number for this keyboard index
		int port = -1;
		for (int p = 0; p < SM_MAX_PORT_NUM; p++) {
			if (m_KeyboardIndex[p] == (int)k) { port = p; break; }
		}
		result = _BuildPerKeyIndex(&m_Subs[k], port);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Create keyboards (Mod11 per active port)
//******************************************************************************
int MTPianoKeyboardCtrlMod11::_CreateKeyboards(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	for (unsigned char index = 0; index < m_MaxKeyboardIndex; index++) {
		m_Subs[index].pKeyboard = new MTPianoKeyboardMod11();
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
// Update
//******************************************************************************
int MTPianoKeyboardCtrlMod11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	int result = 0;

	if (m_isSkipping) goto EXIT;

	{
		// Playback position vector
		Vector3 playbackPos = m_NoteDesignMod.GetWorldMoveVector();
		playbackPos.x += m_NoteDesignMod.GetPlayPosX(ctx.curTickTime);

		unsigned char lastPortNo = 0;
		if (!m_isSingleKeyboard) {
			m_PortList.GetPort(m_PortList.GetSize() - 1, &lastPortNo);
		}

		for (unsigned char portNo = 0; portNo <= lastPortNo; portNo++) {
			int keyboardIndex = !m_isSingleKeyboard ? m_KeyboardIndex[portNo] : 0;
			if (keyboardIndex < 0) continue;

			if (m_Subs[keyboardIndex].pKeyboard == NULL) continue;

			// Evaluate key states
			_EvaluateKeyStates(&m_Subs[keyboardIndex], ctx.curTickTime);

			// Apply active key color (DesignMod palette) for fully pressed keys
			for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
				MTKeyboardKeyState& ks = m_Subs[keyboardIndex].keyStates[noteNo];
				if (ks.rate >= 1.0f) {
					// Find chNo from the note that contributed the max rate
					unsigned char chNo = 0;
					unsigned long lo = m_Subs[keyboardIndex].keyOffset[noteNo];
					unsigned long hi = m_Subs[keyboardIndex].keyOffset[noteNo + 1];
					for (unsigned long j = lo; j < hi; j++) {
						if (m_Subs[keyboardIndex].pNotes[j].color == ks.color) {
							chNo = m_Subs[keyboardIndex].pNotes[j].chNo;
							break;
						}
					}
					Color noteColor((unsigned int)ks.color);
					Color activeColor = m_DesignMod.GetActiveKeyColor(chNo, noteNo, 0, &noteColor);
					ks.color = activeColor.BGRA();
				}
			}

			// Mod world matrix: scale → base position → orientation → rollAngle → playback
			Vector3 basePos = m_DesignMod.GetKeyboardBasePos(keyboardIndex, ctx.rollAngle);
			basePos.x += _GetMaxPitchBendShift(portNo);

			float scale = m_DesignMod.GetKeyboardResizeRatio();

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

			Matrix world = Matrix::CreateScale(scale)
			             * Matrix::CreateTranslation(basePos)
			             * rotateMatrix1
			             * rotateMatrix2
			             * rotateMatrix3
			             * Matrix::CreateTranslation(playbackPos);

			// Dispatch to keyboard
			result = m_Subs[keyboardIndex].pKeyboard->Update(m_pContext, m_Subs[keyboardIndex].keyStates, world);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTPianoKeyboardCtrlMod11::Reset()
{
	MTPianoKeyboardCtrl11::Reset();
}

//******************************************************************************
// Max pitch bend shift
//******************************************************************************
float MTPianoKeyboardCtrlMod11::_GetMaxPitchBendShift(
		unsigned char portNo
	)
{
	if (m_pNotePitchBend == NULL) return 0.0f;

	float maxShift = 0.0f;
	for (unsigned char chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		short pbValue = m_pNotePitchBend->GetValue(portNo, chNo);
		unsigned char pbSensitivity = m_pNotePitchBend->GetSensitivity(portNo, chNo);
		float shift = m_NoteDesign.GetNoteStep() * pbSensitivity * ((float)pbValue / 8192.0f);
		if (fabsf(shift) > fabsf(maxShift)) {
			maxShift = shift;
		}
	}
	return maxShift;
}
