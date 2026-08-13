//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl11
//
// Piano keyboard controller (Flat, Playback).
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrl11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrl11::MTPianoKeyboardCtrl11()
{
	m_pNoteTracker = NULL;
}

MTPianoKeyboardCtrl11::~MTPianoKeyboardCtrl11()
{
	Release();
}

//******************************************************************************
// Create
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

	MTPianoKeyboardCtrlBase11::Release();

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
	MTPianoKeyboardCtrlBase11::Release();
	m_pNoteTracker = NULL;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTPianoKeyboardCtrl11::Reset()
{
	MTPianoKeyboardCtrlBase11::Reset();
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
// _UpdateKeyStates (Playback: cursor scan + active color)
//******************************************************************************
void MTPianoKeyboardCtrl11::_UpdateKeyStates(
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	_EvaluateKeyStates(&m_Subs[kbdIndex], ctx.playTimeMSec);
	_ApplyActiveKeyColor(&m_Subs[kbdIndex], kbdIndex);
}

//******************************************************************************
// Max pitch bend shift across all ports (for single keyboard mode)
//******************************************************************************
float MTPianoKeyboardCtrl11::_GetMaxPitchBendShiftAllPorts(
		SMPortList* pPortList
	)
{
	float maxShift = 0.0f;
	for (unsigned long i = 0; i < pPortList->GetSize(); i++) {
		unsigned char portNo = 0;
		pPortList->GetPort(i, &portNo);
		float shift = m_pNoteDesign->GetMaxPitchBendShift(
			m_pNotePitchBend, portNo,
			m_pNoteTracker->GetActiveChannelMask(portNo));
		if (fabsf(shift) > fabsf(maxShift)) maxShift = shift;
	}
	return maxShift;
}
