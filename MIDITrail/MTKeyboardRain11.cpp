//******************************************************************************
//
// MIDITrail / MTKeyboardRain11
//
// DX11 Rain-scene keyboard (M4.7b) - port of MTPianoKeyboardCtrl (non-Mod).
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTKeyboardRain11.h"
#include "DXTexture11.h"
#include <stdlib.h>

using namespace YNBaseLib;
using namespace DirectX;

#define MTKBDRAIN11_SEEK_BACK_TICKS  (1920)


MTKeyboardRain11::MTKeyboardRain11()
{
	m_pSRV = NULL;
	m_pPitchBend = NULL;
	m_Ready = false;
	m_CurTickTime = 0;
	m_pBaseVerts = NULL;
	m_VertexNum = 0;
	m_NumKbd = 0;
	for (unsigned long i = 0; i < MTKBDRAIN11_MAX_KEYBOARDS; i++) {
		m_Subs[i].pWorkVerts = NULL;
		m_Subs[i].chNo = 0;
		m_Subs[i].dirty = false;
		m_Subs[i].pNotes = NULL;
		m_Subs[i].noteCount = 0;
		m_Subs[i].nextNoteIdx = 0;
		m_Subs[i].lastTick = 0;
		ZeroMemory(m_Subs[i].keyDown, sizeof(m_Subs[i].keyDown));
		ZeroMemory(m_Subs[i].keyColor, sizeof(m_Subs[i].keyColor));
		ZeroMemory(m_Subs[i].keyRenderedColor, sizeof(m_Subs[i].keyRenderedColor));
		ZeroMemory(m_Subs[i].keyMaxEndTick, sizeof(m_Subs[i].keyMaxEndTick));
		ZeroMemory(m_Subs[i].activeColNum, sizeof(m_Subs[i].activeColNum));
	}
}

MTKeyboardRain11::~MTKeyboardRain11()
{
	Release();
}

void MTKeyboardRain11::_ReleaseSub(SubKbd* pSub)
{
	pSub->prim.Release();
	if (pSub->pWorkVerts != NULL) { free(pSub->pWorkVerts); pSub->pWorkVerts = NULL; }
	if (pSub->pNotes != NULL) { free(pSub->pNotes); pSub->pNotes = NULL; }
	pSub->noteCount = 0;
	pSub->nextNoteIdx = 0;
	pSub->lastTick = 0;
	pSub->dirty = false;
	ZeroMemory(pSub->keyDown, sizeof(pSub->keyDown));
	ZeroMemory(pSub->keyColor, sizeof(pSub->keyColor));
	ZeroMemory(pSub->keyRenderedColor, sizeof(pSub->keyRenderedColor));
	ZeroMemory(pSub->keyMaxEndTick, sizeof(pSub->keyMaxEndTick));
	ZeroMemory(pSub->activeColNum, sizeof(pSub->activeColNum));
}

void MTKeyboardRain11::Release()
{
	for (unsigned long i = 0; i < MTKBDRAIN11_MAX_KEYBOARDS; i++) _ReleaseSub(&m_Subs[i]);
	m_NumKbd = 0;
	if (m_pSRV != NULL) { m_pSRV->Release(); m_pSRV = NULL; }
	if (m_pBaseVerts != NULL) { free(m_pBaseVerts); m_pBaseVerts = NULL; }
	m_VertexNum = 0;
	m_CurTickTime = 0;
	m_Ready = false;
}

void MTKeyboardRain11::Reset()
{
	m_CurTickTime = 0;
	for (unsigned long i = 0; i < m_NumKbd; i++) {
		m_Subs[i].nextNoteIdx = 0;
		m_Subs[i].lastTick = 0;
		ZeroMemory(m_Subs[i].keyMaxEndTick, sizeof(m_Subs[i].keyMaxEndTick));
		ZeroMemory(m_Subs[i].activeColNum, sizeof(m_Subs[i].activeColNum));
		m_Subs[i].dirty = true;
	}
}

void MTKeyboardRain11::SetCurTickTime(unsigned long curTickTime)
{
	m_CurTickTime = curTickTime;
	for (unsigned long i = 0; i < m_NumKbd; i++) {
		_AdvanceWindow(&m_Subs[i], curTickTime);
		m_Subs[i].dirty = true;
	}
}

//******************************************************************************
// live monitor key-press state (m_CurTickTime stays 0 in live, so any non-zero
// keyMaxEndTick reads as "down" in _ApplyKeyStates) - mirrors MTKeyboard11
//******************************************************************************
void MTKeyboardRain11::SetNoteOnLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo)
{
	if ((m_NumKbd == 0) || (noteNo >= SM_MAX_NOTE_NUM)) return;
	int sub = (m_NumKbd <= 1) ? 0 : ((chNo < SM_MAX_CH_NUM) ? (int)chNo : 0);
	m_Subs[sub].keyMaxEndTick[noteNo] = 0xFFFFFFFF;
	m_Subs[sub].keyColor[noteNo] = (unsigned long)m_NoteDesign.GetNoteBoxColor(portNo, chNo, noteNo);
	m_Subs[sub].activeColNum[noteNo] = 0;   // live path doesn't use the tick color list
	m_Subs[sub].dirty = true;
}

void MTKeyboardRain11::SetNoteOffLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo)
{
	(void)portNo;
	if ((m_NumKbd == 0) || (noteNo >= SM_MAX_NOTE_NUM)) return;
	int sub = (m_NumKbd <= 1) ? 0 : ((chNo < SM_MAX_CH_NUM) ? (int)chNo : 0);
	m_Subs[sub].keyMaxEndTick[noteNo] = 0;
	m_Subs[sub].dirty = true;
}

void MTKeyboardRain11::AllNoteOffLive()
{
	for (unsigned long i = 0; i < m_NumKbd; i++) {
		ZeroMemory(m_Subs[i].keyMaxEndTick, sizeof(m_Subs[i].keyMaxEndTick));
		m_Subs[i].dirty = true;
	}
}

void MTKeyboardRain11::_AdvanceWindow(SubKbd* pSub, unsigned long tick)
{
	if (tick + MTKBDRAIN11_SEEK_BACK_TICKS < pSub->lastTick) {
		pSub->nextNoteIdx = 0;
		ZeroMemory(pSub->keyMaxEndTick, sizeof(pSub->keyMaxEndTick));
	ZeroMemory(pSub->activeColNum, sizeof(pSub->activeColNum));
		pSub->lastTick = tick;
	}
	else if (tick > pSub->lastTick) {
		pSub->lastTick = tick;
	}

	while (pSub->nextNoteIdx < pSub->noteCount) {
		const KbdNote* pn = &pSub->pNotes[pSub->nextNoteIdx];
		if (pn->startTime > tick) break;
		if (pn->noteNo < SM_MAX_NOTE_NUM) {
			if (pn->endTime > pSub->keyMaxEndTick[pn->noteNo]) {
				pSub->keyMaxEndTick[pn->noteNo] = pn->endTime;
			}
			// push onto the key's active-color list (still-sounding notes only)
			if (pn->endTime > tick) {
				unsigned char n = pSub->activeColNum[pn->noteNo];
				if (n >= KBDR11_COLOR_CAP) {   // drop the oldest
					for (unsigned char k = 1; k < KBDR11_COLOR_CAP; k++)
						pSub->activeCol[pn->noteNo][k - 1] = pSub->activeCol[pn->noteNo][k];
					n = KBDR11_COLOR_CAP - 1;
				}
				pSub->activeCol[pn->noteNo][n].endTime = pn->endTime;
				pSub->activeCol[pn->noteNo][n].color   = pn->color;
				pSub->activeColNum[pn->noteNo] = n + 1;
			}
		}
		pSub->nextNoteIdx++;
	}
}

int MTKeyboardRain11::_ApplyKeyStates(ID3D11DeviceContext* pContext, SubKbd* pSub)
{
	int result = 0;
	bool changed = false;
	unsigned char note;
	DXP11_VERTEX* pBase = (DXP11_VERTEX*)m_pBaseVerts;
	DXP11_VERTEX* pWork = (DXP11_VERTEX*)pSub->pWorkVerts;

	if (!pSub->dirty) return 0;
	if ((pBase == NULL) || (pWork == NULL)) { pSub->dirty = false; return 0; }

	for (note = 0; note < SM_MAX_NOTE_NUM; note++) {
		// drop ended notes from the key's active-color list, take the latest
		// still-active note's color (revert as notes end).
		unsigned char an = pSub->activeColNum[note];
		if (an > 0) {
			unsigned char w = 0;
			for (unsigned char r = 0; r < an; r++) {
				if (pSub->activeCol[note][r].endTime > m_CurTickTime) {
					if (w != r) pSub->activeCol[note][w] = pSub->activeCol[note][r];
					w++;
				}
			}
			pSub->activeColNum[note] = w;
			if (w > 0) pSub->keyColor[note] = pSub->activeCol[note][w - 1].color;
		}

		bool wantDown = (pSub->keyMaxEndTick[note] > m_CurTickTime);
		bool needRebuild = (wantDown != pSub->keyDown[note])
		                || (wantDown && (pSub->keyColor[note] != pSub->keyRenderedColor[note]));
		if (!needRebuild) continue;

		unsigned long pos = 0, num = 0;
		m_Geom.GetKeyVertexRange(note, &pos, &num);
		if (num == 0) { pSub->keyDown[note] = wantDown; continue; }

		if (wantDown) {
			D3DXCOLOR col((D3DCOLOR)pSub->keyColor[note]);
			m_Geom.BuildKeyCPU(note, 1.0f, &col, &pWork[pos]);
			pSub->keyRenderedColor[note] = pSub->keyColor[note];
		}
		else {
			memcpy(&pWork[pos], &pBase[pos], (size_t)num * sizeof(DXP11_VERTEX));
		}
		pSub->keyDown[note] = wantDown;
		changed = true;
	}

	if (changed) {
		DXP11_VERTEX* pv = NULL;
		result = pSub->prim.LockVertex(pContext, &pv);
		if (result == 0) {
			memcpy(pv, pWork, (size_t)m_VertexNum * sizeof(DXP11_VERTEX));
			pSub->prim.UnlockVertex(pContext);
		}
	}

	pSub->dirty = false;
	return result;
}

//******************************************************************************
// Create: shared geometry/texture + one keyboard per active MIDI channel
//******************************************************************************
int MTKeyboardRain11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long vn = 0, in = 0, i = 0;
	void* pCpuVB = NULL;
	unsigned long* pCpuIB = NULL;
	TCHAR texPath[_MAX_PATH] = { _T('\0') };
	unsigned int tw = 0, th = 0;
	int chToSub[SM_MAX_CH_NUM];

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_Geom.InitForDX11(pSceneName, pSeqData);
	if (result != 0) goto EXIT;
	result = m_Design.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	for (i = 0; i < SM_MAX_CH_NUM; i++) chToSub[i] = -1;

	//----------------------------------
	// shared geometry (master CPU copy + index data)
	//----------------------------------
	m_Geom.GetGeometrySize(&vn, &in);
	if ((vn == 0) || (in == 0)) { result = 0; goto EXIT; }

	pCpuVB = malloc((size_t)vn * sizeof(DXP11_VERTEX));
	pCpuIB = (unsigned long*)malloc((size_t)in * sizeof(unsigned long));
	if ((pCpuVB == NULL) || (pCpuIB == NULL)) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	result = m_Geom.BuildGeometryCPU(pCpuVB, pCpuIB);
	if (result != 0) goto EXIT;

	m_VertexNum = vn;
	m_pBaseVerts = malloc((size_t)vn * sizeof(DXP11_VERTEX));
	if (m_pBaseVerts == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	memcpy(m_pBaseVerts, pCpuVB, (size_t)vn * sizeof(DXP11_VERTEX));

	//----------------------------------
	// notes -> per-channel compact arrays (two passes), discover used channels
	//----------------------------------
	{
		SMTrack track;
		SMNoteList noteList;
		SMNote note;
		unsigned long total = 0;
		unsigned long counts[SM_MAX_CH_NUM];
		unsigned long fill[SM_MAX_CH_NUM];
		// track color mode: keep each note's source track so the pressed-key color
		// (ActiveKeyColorType=NOTE) matches the track-channel note color.
		std::vector<unsigned char> trackNoList;
		bool trackMode = m_NoteDesign.IsTrackColorMode();

		for (i = 0; i < SM_MAX_CH_NUM; i++) { counts[i] = 0; fill[i] = 0; }

		// Honor KeyboardMaxDispNum: >1 stacks one keyboard per channel (0..15) in
		// depth (KeyboardStepZ); =1 is a single flat keyboard (e.g. Rain2D, where
		// StepZ~0 so 16 channel keyboards would pile onto the exact same spot).
		// (Geometry-only; the live monitor builds the keyboards here without a song.)
		if (m_Design.GetKeyboardMaxDispNum() <= 1) {
			m_NumKbd = 1;
			m_Subs[0].chNo = 0;
			for (i = 0; i < SM_MAX_CH_NUM; i++) chToSub[i] = 0;  //all channels -> one keyboard
		}
		else {
			m_NumKbd = SM_MAX_CH_NUM;
			for (i = 0; i < SM_MAX_CH_NUM; i++) {
				chToSub[i] = (int)i;
				m_Subs[i].chNo = (int)i;
			}
		}

		// note timeline -> per-channel arrays (playback only; live has no song and
		// lights keys directly from real-time MIDI via SetNoteOnLive/OffLive).
		if (pSeqData != NULL) {
			if (trackMode) {
				result = MTNoteDesign::BuildMergedNoteListWithTrack(pSeqData, &noteList, &trackNoList);
				if (result != 0) goto EXIT;
			}
			else {
				result = pSeqData->GetMergedTrack(&track);
				if (result != 0) goto EXIT;
				result = track.GetNoteList(&noteList);
				if (result != 0) goto EXIT;
			}
			total = noteList.GetSize();

			for (i = 0; i < total; i++) {
				if (noteList.GetNote(i, &note) != 0) { result = YN_SET_ERR("Program error.", i, 0); goto EXIT; }
				if (note.chNo >= SM_MAX_CH_NUM) continue;
				counts[chToSub[note.chNo]]++;
			}
		}

		// per-keyboard GPU buffers + CPU work mirror
		for (i = 0; i < m_NumKbd; i++) {
			DXP11_VERTEX* pv = NULL;
			unsigned long* pi = NULL;
			result = m_Subs[i].prim.CreateVertexBuffer(pDevice, vn);
			if (result != 0) goto EXIT;
			result = m_Subs[i].prim.CreateIndexBuffer(pDevice, in);
			if (result != 0) goto EXIT;
			result = m_Subs[i].prim.LockVertex(pContext, &pv);
			if (result != 0) goto EXIT;
			memcpy(pv, pCpuVB, (size_t)vn * sizeof(DXP11_VERTEX));
			m_Subs[i].prim.UnlockVertex(pContext);
			result = m_Subs[i].prim.LockIndex(pContext, &pi);
			if (result != 0) goto EXIT;
			memcpy(pi, pCpuIB, (size_t)in * sizeof(unsigned long));
			m_Subs[i].prim.UnlockIndex(pContext);
			m_Subs[i].prim.SetMaterialAmbient(0.55f, 0.55f, 0.55f);

			m_Subs[i].pWorkVerts = malloc((size_t)vn * sizeof(DXP11_VERTEX));
			if (m_Subs[i].pWorkVerts == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
			memcpy(m_Subs[i].pWorkVerts, pCpuVB, (size_t)vn * sizeof(DXP11_VERTEX));

			if (counts[i] > 0) {
				m_Subs[i].pNotes = (KbdNote*)malloc((size_t)counts[i] * sizeof(KbdNote));
				if (m_Subs[i].pNotes == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
			}
			m_Subs[i].noteCount = counts[i];
		}

		// fill per-channel note arrays (note list sorted by start -> stays sorted)
		if (pSeqData != NULL) {
			for (i = 0; i < total; i++) {
				if (noteList.GetNote(i, &note) != 0) { result = YN_SET_ERR("Program error.", i, 0); goto EXIT; }
				if (note.chNo >= SM_MAX_CH_NUM) continue;
				int sub = chToSub[note.chNo];
				if (sub < 0) continue;
				KbdNote* pn = &m_Subs[sub].pNotes[fill[sub]++];
				pn->startTime = note.startTime;
				pn->endTime   = note.endTime;
				pn->color     = trackMode
					? (D3DCOLOR)m_NoteDesign.GetTrackChannelColor(trackNoList[i], note.chNo)
					: (D3DCOLOR)m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
				pn->noteNo    = note.noteNo;
			}
		}
	}

	// keyboard texture (shared)
	if (m_Geom.GetTexturePath(pSceneName, texPath, _MAX_PATH) == 0) {
		DXTexture11::LoadFromFile(pDevice, texPath, &m_pSRV, &tw, &th);
	}

	m_Ready = true;

EXIT:;
	if (pCpuVB != NULL) free(pCpuVB);
	if (pCpuIB != NULL) free(pCpuIB);
	return result;
}

//******************************************************************************
// Draw: each channel keyboard at its base, rising with playback in Y
//   world = Trans(base.x, base.y + playPos, base.z) * RotY(roll)
//******************************************************************************
int MTKeyboardRain11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle
	)
{
	if (!m_Ready) return 0;

	float playY = m_NoteDesign.GetPlayPosX(m_CurTickTime);
	XMMATRIX R = XMMatrixRotationY(XMConvertToRadians(rollAngle));

	for (unsigned long i = 0; i < m_NumKbd; i++) {
		SubKbd* pSub = &m_Subs[i];
		_ApplyKeyStates(pContext, pSub);

		D3DXVECTOR3 base = m_Design.GetKeyboardBasePos(0, (unsigned char)pSub->chNo);

		// pitch bend: shift this channel's keyboard in X (matches MTPianoKeyboardCtrl)
		if (m_pPitchBend != NULL) {
			base.x += m_Design.GetPitchBendShift(
					m_pPitchBend->GetValue(0, (unsigned long)pSub->chNo),
					m_pPitchBend->GetSensitivity(0, (unsigned long)pSub->chNo));
		}

		XMMATRIX world = XMMatrixTranslation(base.x, base.y + playY, base.z) * R;
		pSub->prim.SetWorldMatrix(world);
		pSub->prim.SetTexture(m_pSRV);
		pSub->prim.Draw(pContext, viewProj, lightDir, -1, 0);
	}

	return 0;
}
