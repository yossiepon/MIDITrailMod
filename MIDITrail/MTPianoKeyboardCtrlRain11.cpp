//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRain11
//
// Piano keyboard controller for Rain scenes.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
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
	memset(m_KbdPortNo, 0, sizeof(m_KbdPortNo));
	memset(m_KbdChNo, 0, sizeof(m_KbdChNo));
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

	if (pSeqData != NULL) {
		result = pSeqData->GetPortList(&m_PortList);
		if (result != 0) goto EXIT;
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
// Create keyboards (Rain: per-port * per-channel)
//******************************************************************************
int MTPianoKeyboardCtrlRain11::_CreateKeyboards(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	if (m_isSingleKeyboard) {
		m_Subs[0].pKeyboard = new MTPianoKeyboardFlat11();
		if (m_Subs[0].pKeyboard == NULL) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}
		result = m_Subs[0].pKeyboard->Create(pDevice, pContext, pSceneName, pSeqData, m_pSRV);
		if (result != 0) goto EXIT;
		m_KbdPortNo[0] = 0;
		m_KbdChNo[0] = 0;
		m_NumKbd = 1;
	}
	else {
		unsigned long numPorts = m_PortList.GetSize();
		if (numPorts == 0) numPorts = 1;

		unsigned long kbdIndex = 0;
		for (unsigned long p = 0; p < numPorts && kbdIndex < MT_KBD_MAX_SUBS; p++) {
			unsigned char portNo = 0;
			if (m_PortList.GetSize() > 0) {
				m_PortList.GetPort(p, &portNo);
			}
			for (unsigned char chNo = 0; chNo < SM_MAX_CH_NUM && kbdIndex < MT_KBD_MAX_SUBS; chNo++) {
				m_Subs[kbdIndex].pKeyboard = new MTPianoKeyboardFlat11();
				if (m_Subs[kbdIndex].pKeyboard == NULL) {
					result = YN_SET_ERR("Could not allocate memory.", 0, 0);
					goto EXIT;
				}
				result = m_Subs[kbdIndex].pKeyboard->Create(pDevice, pContext, pSceneName, pSeqData, m_pSRV);
				if (result != 0) goto EXIT;

				m_KbdPortNo[kbdIndex] = portNo;
				m_KbdChNo[kbdIndex] = chNo;
				kbdIndex++;
			}
		}
		m_NumKbd = kbdIndex;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Per-key index params (Rain: filter by port and channel)
//******************************************************************************
void MTPianoKeyboardCtrlRain11::_GetPerKeyIndexParams(
		unsigned long kbdIndex,
		int& outPortFilter,
		int& outChFilter
	)
{
	if (m_isSingleKeyboard) {
		outPortFilter = -1;
		outChFilter = -1;
	}
	else {
		outPortFilter = (int)m_KbdPortNo[kbdIndex];
		outChFilter = (int)m_KbdChNo[kbdIndex];
	}
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
// World matrix (Rain: port + channel positioning)
//******************************************************************************
Matrix MTPianoKeyboardCtrlRain11::_ComputeWorldMatrix(
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	unsigned char portNo = m_KbdPortNo[kbdIndex];
	unsigned char chNo = m_KbdChNo[kbdIndex];

	// のこぎり形配置: Y は chNo のみ（同一 ch は同じ高さ）、Z は portNo で奥行き分離
	unsigned char port0 = 0, ch0 = 0;
	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(port0, chNo);
	if (portNo != 0) {
		moveVec.z += m_KeyboardDesign.GetKeyboardBasePos(portNo, ch0).z
		           - m_KeyboardDesign.GetKeyboardBasePos(port0, ch0).z;
	}

	if (m_isSingleKeyboard) {
		moveVec.x += _GetMaxPitchBendShiftAllPorts(&m_PortList);
	}
	else if (m_pNotePitchBend != NULL && m_pNoteTracker->HasActiveNotesOnChannel(portNo, chNo)) {
		short v = m_pNotePitchBend->GetValue(portNo, chNo);
		unsigned char s = m_pNotePitchBend->GetSensitivity(portNo, chNo);
		moveVec.x += m_KeyboardDesign.GetPitchBendShift(v, s);
	}

	if (m_isPlaybackPosTracking) {
		moveVec.y += m_NoteDesign.GetPlayPosX(ctx.curTickTime);
	}
	return Matrix::CreateTranslation(moveVec)
	     * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
}
