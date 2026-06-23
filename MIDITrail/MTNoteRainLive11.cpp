//******************************************************************************
//
// MIDITrail / MTNoteRainLive11
//
// DX11 live-monitor falling-note (Rain) renderer - port of MTNoteRainLive.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTNoteRainLive11.h"
#include <stdlib.h>
#include <new>

using namespace YNBaseLib;
using namespace DirectX;

#define MTNRL11_VERTS_PER_NOTE  (4)
#define MTNRL11_TRIS_PER_NOTE   (2)

// per-note quad index pattern (matches MTNoteRainLive: 0,2,1, 0,3,2)
static const unsigned long MTNRL11_QUAD_IDX[6] = { 0, 2, 1, 0, 3, 2 };


MTNoteRainLive11::MTNoteRainLive11()
{
	m_pExtPitchBend = NULL;
	m_pCpuBuf = NULL;
	m_VertCapacity = 0;
	m_pNoteStatus = NULL;
	m_DisplayDuration = 3000;
	m_MinNoteElapsed = 60;
	m_Ready = false;
}

MTNoteRainLive11::~MTNoteRainLive11()
{
	Release();
}

void MTNoteRainLive11::Release()
{
	m_Prim.Release();
	if (m_pNoteStatus != NULL) { delete [] m_pNoteStatus; m_pNoteStatus = NULL; }
	if (m_pCpuBuf != NULL) { free(m_pCpuBuf); m_pCpuBuf = NULL; }
	m_VertCapacity = 0;
	m_Ready = false;
}

int MTNoteRainLive11::Create(
		ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName, SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long i = 0, s = 0;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;
	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	m_PitchBend.Initialize();
	m_DisplayDuration = m_NoteDesign.GetLiveMonitorDisplayDuration();
	{
		float perSec = m_NoteDesign.GetLivePosX(1000);
		float minLenX = m_KeyboardDesign.GetBlackKeyWidth();
		if (minLenX < 0.1f) minLenX = 0.1f;
		m_MinNoteElapsed = (perSec > 0.0001f) ? (unsigned long)(minLenX * 1000.0f / perSec) : 60;
	}

	try {
		m_pNoteStatus = new NoteStatus[MTNOTERAINLIVE11_MAX_NOTES];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	Reset();

	m_VertCapacity = MTNRL11_VERTS_PER_NOTE * MTNOTERAINLIVE11_MAX_NOTES;
	result = m_Prim.CreateVertexBuffer(pDevice, m_VertCapacity);
	if (result != 0) goto EXIT;
	{
		unsigned long* pi = NULL;
		result = m_Prim.CreateIndexBuffer(pDevice, 6 * MTNOTERAINLIVE11_MAX_NOTES);
		if (result != 0) goto EXIT;
		result = m_Prim.LockIndex(pContext, &pi);
		if (result != 0) goto EXIT;
		for (s = 0; s < MTNOTERAINLIVE11_MAX_NOTES; s++)
			for (i = 0; i < 6; i++) pi[s * 6 + i] = s * MTNRL11_VERTS_PER_NOTE + MTNRL11_QUAD_IDX[i];
		m_Prim.UnlockIndex(pContext);
	}

	m_pCpuBuf = malloc((size_t)m_VertCapacity * sizeof(DXP11_VERTEX));
	if (m_pCpuBuf == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);
	m_Prim.SetAdditiveBlend(false);   // normal alpha blend (notes fade 1.0 -> 0.5)

	m_Ready = true;

EXIT:;
	return result;
}

void MTNoteRainLive11::Reset()
{
	if (m_pNoteStatus == NULL) return;
	for (unsigned long i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].portNo = 0; m_pNoteStatus[i].chNo = 0; m_pNoteStatus[i].noteNo = 0;
		m_pNoteStatus[i].startTime = 0; m_pNoteStatus[i].endTime = 0;
	}
}

void MTNoteRainLive11::SetNoteOn(unsigned char portNo, unsigned char chNo, unsigned char noteNo, unsigned char velocity)
{
	unsigned long i = 0, slot = 0;
	bool isFind = false;
	unsigned long curTime = timeGetTime();
	(void)velocity;
	if (m_pNoteStatus == NULL) return;

	// retrigger / dropped note-off: close any still-held note for this key first
	for (i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime == 0)
			&& (m_pNoteStatus[i].portNo == portNo) && (m_pNoteStatus[i].chNo == chNo)
			&& (m_pNoteStatus[i].noteNo == noteNo)) {
			m_pNoteStatus[i].endTime = curTime;
		}
	}
	for (i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive) {
			if ((m_pNoteStatus[i].endTime != 0) && ((curTime - m_pNoteStatus[i].endTime) > m_DisplayDuration)) { slot = i; isFind = true; break; }
		} else { slot = i; isFind = true; break; }
	}
	if (!isFind) _ClearOldest(&slot);

	m_pNoteStatus[slot].isActive = true;
	m_pNoteStatus[slot].portNo = portNo; m_pNoteStatus[slot].chNo = chNo; m_pNoteStatus[slot].noteNo = noteNo;
	m_pNoteStatus[slot].startTime = curTime; m_pNoteStatus[slot].endTime = 0;
}

void MTNoteRainLive11::SetNoteOff(unsigned char portNo, unsigned char chNo, unsigned char noteNo)
{
	if (m_pNoteStatus == NULL) return;
	unsigned long curTime = timeGetTime();
	for (unsigned long i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].portNo == portNo) && (m_pNoteStatus[i].chNo == chNo)
			&& (m_pNoteStatus[i].noteNo == noteNo) && (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = curTime; break;
		}
	}
}

void MTNoteRainLive11::AllNoteOff()
{
	if (m_pNoteStatus == NULL) return;
	unsigned long curTime = timeGetTime();
	for (unsigned long i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++)
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime == 0)) m_pNoteStatus[i].endTime = curTime;
}

void MTNoteRainLive11::AllNoteOffOnCh(unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteStatus == NULL) return;
	unsigned long curTime = timeGetTime();
	for (unsigned long i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++)
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime == 0) && (m_pNoteStatus[i].portNo == portNo) && (m_pNoteStatus[i].chNo == chNo))
			m_pNoteStatus[i].endTime = curTime;
}

void MTNoteRainLive11::_ClearOldest(unsigned long* pIndex)
{
	unsigned long oldest = 0; bool isFind = false;
	for (unsigned long i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++) {
		if (!m_pNoteStatus[i].isActive) continue;
		if (!isFind) { oldest = i; isFind = true; }
		else if (m_pNoteStatus[i].startTime < m_pNoteStatus[oldest].startTime) oldest = i;
	}
	m_pNoteStatus[oldest].isActive = false;
	*pIndex = oldest;
}

void MTNoteRainLive11::_ExpireOld(unsigned long curTime)
{
	for (unsigned long i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++)
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime != 0) && ((curTime - m_pNoteStatus[i].endTime) > m_DisplayDuration))
			m_pNoteStatus[i].isActive = false;
}

//******************************************************************************
// one falling-note quad (matches MTNoteRainLive::_CreateVertexOfNote)
//******************************************************************************
void MTNoteRainLive11::_BuildNote(const NoteStatus& note, unsigned long curTime, DXP11_VERTEX* pv)
{
	short pbValue = 0;
	unsigned char pbSens = SM_DEFAULT_PITCHBEND_SENSITIVITY;
	float pbShift = 0.0f;
	if (note.endTime == 0) {
		pbValue = _Bend()->GetValue(note.portNo, note.chNo);
		pbSens  = _Bend()->GetSensitivity(note.portNo, note.chNo);
		pbShift = m_KeyboardDesign.GetPitchBendShift(pbValue, pbSens);
	}

	unsigned long startElapsed = curTime - note.startTime;
	unsigned long endElapsed   = (note.endTime != 0) ? (curTime - note.endTime) : 0;
	if (startElapsed < endElapsed + m_MinNoteElapsed) startElapsed = endElapsed + m_MinNoteElapsed;
	if (startElapsed > m_DisplayDuration) startElapsed = m_DisplayDuration;

	float startY = m_NoteDesign.GetLivePosX(startElapsed);
	float endY   = m_NoteDesign.GetLivePosX(endElapsed);

	D3DXVECTOR3 base = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
	float cx = base.x + m_KeyboardDesign.GetKeyCenterPosX(note.noteNo) + pbShift;
	float by = base.y + m_KeyboardDesign.GetWhiteKeyHeight() / 2.0f;
	float cz = base.z + m_KeyboardDesign.GetNoteDropPosZ(note.noteNo);
	float hw = m_KeyboardDesign.GetBlackKeyWidth() / 2.0f;

	D3DXCOLOR col = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	unsigned long R = (unsigned long)(col.r * 255.0f + 0.5f); if (R > 255) R = 255;
	unsigned long G = (unsigned long)(col.g * 255.0f + 0.5f); if (G > 255) G = 255;
	unsigned long B = (unsigned long)(col.b * 255.0f + 0.5f); if (B > 255) B = 255;
	unsigned long cFull = (0xFFu << 24) | (R << 16) | (G << 8) | B;   // note-on edge
	unsigned long cHalf = (0x80u << 24) | (R << 16) | (G << 8) | B;   // note-off edge

	// v0,v1 = note-on edge (startY); v2,v3 = note-off edge (endY)
	pv[0].pos[0]=cx-hw; pv[0].pos[1]=by+startY; pv[0].pos[2]=cz; pv[0].color=cFull;
	pv[1].pos[0]=cx+hw; pv[1].pos[1]=by+startY; pv[1].pos[2]=cz; pv[1].color=cFull;
	pv[2].pos[0]=cx+hw; pv[2].pos[1]=by+endY;   pv[2].pos[2]=cz; pv[2].color=cHalf;
	pv[3].pos[0]=cx-hw; pv[3].pos[1]=by+endY;   pv[3].pos[2]=cz; pv[3].color=cHalf;
	for (int i = 0; i < 4; i++) {
		pv[i].normal[0] = 0.0f; pv[i].normal[1] = 1.0f; pv[i].normal[2] = 0.0f;
		pv[i].uv[0] = 0.0f; pv[i].uv[1] = 0.0f;
	}
}

unsigned long MTNoteRainLive11::_BuildVertices(unsigned long curTime, DXP11_VERTEX* pBuf)
{
	unsigned long num = 0;
	for (unsigned long i = 0; i < MTNOTERAINLIVE11_MAX_NOTES; i++) {
		if (!m_pNoteStatus[i].isActive) continue;
		_BuildNote(m_pNoteStatus[i], curTime, &pBuf[num * MTNRL11_VERTS_PER_NOTE]);
		num++;
	}
	return num;
}

int MTNoteRainLive11::DrawDX11(ID3D11DeviceContext* pContext, const XMMATRIX& viewProj, const XMFLOAT4& lightDir, float rollAngle)
{
	unsigned long curTime = 0, activeNum = 0;
	DXP11_VERTEX* pv = NULL;
	if (!m_Ready) return 0;

	curTime = timeGetTime();
	_ExpireOld(curTime);
	activeNum = _BuildVertices(curTime, (DXP11_VERTEX*)m_pCpuBuf);
	if (activeNum == 0) return 0;
	if (activeNum > MTNOTERAINLIVE11_MAX_NOTES) activeNum = MTNOTERAINLIVE11_MAX_NOTES;

	if (m_Prim.LockVertex(pContext, &pv) != 0) return 0;
	memcpy(pv, m_pCpuBuf, (size_t)activeNum * MTNRL11_VERTS_PER_NOTE * sizeof(DXP11_VERTEX));
	m_Prim.UnlockVertex(pContext);

	// Rain world = RotY(roll) (notes are placed in absolute coords; camera is fixed in live)
	XMMATRIX world = XMMatrixRotationY(XMConvertToRadians(rollAngle));
	m_Prim.SetWorldMatrix(world);
	return m_Prim.Draw(pContext, viewProj, lightDir, (int)(MTNRL11_TRIS_PER_NOTE * activeNum), 0);
}
