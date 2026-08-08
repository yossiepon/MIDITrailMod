//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl11
//
// DX11 piano keyboard controller base class.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPianoKeyboardCtrl11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrl11::MTPianoKeyboardCtrl11()
{
	m_pContext = NULL;
	m_pSRV = NULL;
	m_pNoteTracker = NULL;
	m_pNotePitchBend = NULL;
	m_pNoteDesign = NULL;
	m_NumKbd = 0;
	m_KeyDownDurMs = 0;
	m_KeyUpDurMs = 0;
	m_isSingleKeyboard = false;
	m_isSkipping = false;
	ZeroMemory(m_Subs, sizeof(m_Subs));
}

MTPianoKeyboardCtrl11::~MTPianoKeyboardCtrl11()
{
	Release();
}

//******************************************************************************
// Create (template method: derived sets m_pNoteDesign, m_KeyDownDurMs,
//         m_KeyUpDurMs before calling)
//******************************************************************************
int MTPianoKeyboardCtrl11::Create(
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

	Release();

	if (pSeqData == NULL || pNoteTracker == NULL || m_pNoteDesign == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_pContext = pContext;
	m_pNoteTracker = pNoteTracker;
	m_pNotePitchBend = pNotePitchBend;
	m_isSingleKeyboard = isSingleKeyboard;

	result = _LoadTexture(pDevice, pSceneName);
	if (result != 0) goto EXIT;

	result = _CreateKeyboards(pDevice, pContext, pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	for (unsigned long k = 0; k < m_NumKbd; k++) {
		int portFilter, chFilter;
		_GetPerKeyIndexParams(k, portFilter, chFilter);
		result = _BuildPerKeyIndex(&m_Subs[k], portFilter, chFilter);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTPianoKeyboardCtrl11::Release()
{
	for (unsigned long k = 0; k < SM_MAX_CH_NUM; k++) {
		_ReleaseSub(&m_Subs[k]);
	}
	m_NumKbd = 0;

	if (m_pSRV != NULL) {
		m_pSRV->Release();
		m_pSRV = NULL;
	}
	m_pNoteTracker = NULL;
	m_pNotePitchBend = NULL;
}

//******************************************************************************
// Build per-key index from NoteTracker data
//******************************************************************************
int MTPianoKeyboardCtrl11::_BuildPerKeyIndex(
		MTKbdSub* pSub,
		int portFilter,
		int chFilter
	)
{
	int result = 0;

	if (m_pNoteTracker == NULL) goto EXIT;

	unsigned long totalNotes = m_pNoteTracker->GetNoteCount();

	ZeroMemory(pSub->keyOffset, sizeof(pSub->keyOffset));
	for (unsigned long i = 0; i < totalNotes; i++) {
		const NoteData& nd = m_pNoteTracker->GetNote(i);
		if (portFilter >= 0 && nd.portNo != (unsigned char)portFilter) continue;
		if (chFilter >= 0 && nd.chNo != (unsigned char)chFilter) continue;
		if (nd.noteNo < SM_MAX_NOTE_NUM) {
			pSub->keyOffset[nd.noteNo + 1]++;
		}
	}

	for (int k = 1; k <= SM_MAX_NOTE_NUM; k++) {
		pSub->keyOffset[k] += pSub->keyOffset[k - 1];
	}
	pSub->noteCount = pSub->keyOffset[SM_MAX_NOTE_NUM];

	if (pSub->noteCount == 0) goto EXIT;

	pSub->pNotes = (MTKbdNote*)malloc(pSub->noteCount * sizeof(MTKbdNote));
	if (pSub->pNotes == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	{
		unsigned long tempOffset[SM_MAX_NOTE_NUM];
		memcpy(tempOffset, pSub->keyOffset, SM_MAX_NOTE_NUM * sizeof(unsigned long));

		for (unsigned long i = 0; i < totalNotes; i++) {
			const NoteData& nd = m_pNoteTracker->GetNote(i);
			if (portFilter >= 0 && nd.portNo != (unsigned char)portFilter) continue;
			if (chFilter >= 0 && nd.chNo != (unsigned char)chFilter) continue;
			if (nd.noteNo >= SM_MAX_NOTE_NUM) continue;

			unsigned long idx = tempOffset[nd.noteNo]++;
			pSub->pNotes[idx].startTimeMs = nd.startTimeMs;
			pSub->pNotes[idx].endTimeMs = nd.endTimeMs;
			pSub->pNotes[idx].color = m_pNoteDesign->GetNoteBoxColor(
				nd.portNo, nd.chNo, nd.noteNo).BGRA();
			pSub->pNotes[idx].chNo = nd.chNo;
		}
	}

	ZeroMemory(pSub->keyCursor, sizeof(pSub->keyCursor));

EXIT:;
	return result;
}

//******************************************************************************
// Evaluate key states (per-key cursor scan)
//******************************************************************************
void MTPianoKeyboardCtrl11::_EvaluateKeyStates(
		MTKbdSub* pSub,
		unsigned long playTimeMSec
	)
{
	if (pSub->pNotes == NULL) return;

	unsigned long downMs = m_KeyDownDurMs;
	unsigned long upMs = m_KeyUpDurMs;

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		unsigned long lo = pSub->keyOffset[noteNo];
		unsigned long hi = pSub->keyOffset[noteNo + 1];
		unsigned long c = pSub->keyCursor[noteNo];
		if (c < lo) c = lo;

		while (c < hi && pSub->pNotes[c].endTimeMs + upMs < playTimeMSec) c++;
		pSub->keyCursor[noteNo] = c;

		float rate = 0.0f;
		unsigned long useColor = 0xFFFFFFFF;
		unsigned char useCh = 0;

		for (unsigned long j = c; j < hi; j++) {
			unsigned long s = pSub->pNotes[j].startTimeMs;
			unsigned long e = pSub->pNotes[j].endTimeMs;

			if (s > playTimeMSec + downMs) break;

			float r;
			if (playTimeMSec < s) {
				float d = (float)(s - playTimeMSec);
				r = (d >= (float)downMs) ? 0.0f : (1.0f - d / (float)downMs);
			}
			else if (playTimeMSec <= e) {
				r = 1.0f;
			}
			else {
				float d = (float)(playTimeMSec - e);
				r = (d >= (float)upMs) ? 0.0f : (1.0f - d / (float)upMs);
			}

			if (r >= rate) {
				rate = r;
				useColor = pSub->pNotes[j].color;
				useCh = pSub->pNotes[j].chNo;
			}
		}

		pSub->keyStates[noteNo].rate = rate;
		pSub->keyStates[noteNo].color = useColor;
		pSub->keyStates[noteNo].chNo = useCh;
	}
}

//******************************************************************************
// Update (template method: evaluate -> color -> world -> dispatch)
//******************************************************************************
int MTPianoKeyboardCtrl11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	int result = 0;

	for (unsigned long k = 0; k < m_NumKbd; k++) {
		if (m_Subs[k].pKeyboard == NULL) continue;

		if (!m_isSkipping) {
			_EvaluateKeyStates(&m_Subs[k], ctx.playTimeMSec);
			_ApplyActiveKeyColor(&m_Subs[k], k);
		}

		Matrix world = _ComputeWorldMatrix(k, ctx);

		result = m_Subs[k].pKeyboard->Update(m_pContext, m_Subs[k].keyStates, world);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTPianoKeyboardCtrl11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	for (unsigned long k = 0; k < m_NumKbd; k++) {
		if (m_Subs[k].pKeyboard == NULL) continue;
		result = m_Subs[k].pKeyboard->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTPianoKeyboardCtrl11::Reset()
{
	for (unsigned long k = 0; k < m_NumKbd; k++) {
		ZeroMemory(m_Subs[k].keyCursor, sizeof(m_Subs[k].keyCursor));
		ZeroMemory(m_Subs[k].keyStates, sizeof(m_Subs[k].keyStates));
	}
}

//******************************************************************************
// Load texture
//******************************************************************************
int MTPianoKeyboardCtrl11::_LoadTexture(
		ID3D11Device* pDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR bmpFileName[_MAX_PATH] = {_T('\0')};
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Bitmap"));
	if (result != 0) goto EXIT;
	result = confFile.GetStr(_T("Keyboard"), bmpFileName, _MAX_PATH, MT_IMGFILE_KEYBOARD);
	if (result != 0) goto EXIT;

	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	result = DXTexture11::LoadFromFile(pDevice, imgFilePath, &m_pSRV);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release sub-keyboard
//******************************************************************************
void MTPianoKeyboardCtrl11::_ReleaseSub(MTKbdSub* pSub)
{
	if (pSub->pKeyboard != NULL) {
		pSub->pKeyboard->Release();
		delete pSub->pKeyboard;
		pSub->pKeyboard = NULL;
	}
	if (pSub->pNotes != NULL) {
		free(pSub->pNotes);
		pSub->pNotes = NULL;
	}
	pSub->noteCount = 0;
}
