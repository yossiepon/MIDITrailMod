//******************************************************************************
//
// MIDITrail / MTNoteBoxLive11
//
// DX11 live-monitor note-box renderer - port of MTNoteBoxLive.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTNoteBoxLive11.h"
#include <stdlib.h>
#include <new>

using namespace YNBaseLib;
using namespace DirectX;

// 24 vertices / 36 indices per note box (6 faces, matches MTNoteBoxLive)
#define MTNBL11_VERTS_PER_NOTE  (24)
#define MTNBL11_TRIS_PER_NOTE   (12)

// local index pattern for one box (matches MTNoteBoxLive::_GetVertexIndexOfNote)
static const unsigned long MTNBL11_BOX_IDX[36] = {
	 0,  1,  2,   2,  1,  3,   // top
	 4,  5,  6,   6,  5,  7,   // bottom
	 8,  9, 10,  10,  9, 11,   // right
	12, 13, 14,  14, 13, 15,   // left
	16, 17, 18,  18, 17, 19,   // front
	20, 21, 22,  22, 21, 23    // back
};


MTNoteBoxLive11::MTNoteBoxLive11()
{
	m_pExtPitchBend = NULL;
	m_pDesign = NULL;
	m_pCpuBuf = NULL;
	m_VertCapacity = 0;
	m_pNoteStatus = NULL;
	m_DisplayDuration = 3000;
	m_MinNoteElapsed = 60;
	m_NowLineOffsetX = 0.0f;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Ready = false;
}

MTNoteBoxLive11::~MTNoteBoxLive11()
{
	Release();
}

void MTNoteBoxLive11::Release()
{
	m_Prim.Release();
	if (m_pNoteStatus != NULL) { delete [] m_pNoteStatus; m_pNoteStatus = NULL; }
	if (m_pCpuBuf != NULL) { free(m_pCpuBuf); m_pCpuBuf = NULL; }
	m_VertCapacity = 0;
	m_Ready = false;
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteBoxLive11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		bool ringMode
	)
{
	int result = 0;
	D3DXVECTOR3 mv;
	unsigned long i = 0, s = 0;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	//live mode has no song; the design only needs the scene conf (pSeqData may be empty)
	if (ringMode) {
		m_NoteDesignRing.SetLiveMode();
		result = m_NoteDesignRing.Initialize(pSceneName, pSeqData);
		m_pDesign = &m_NoteDesignRing;
	} else {
		result = m_NoteDesign.Initialize(pSceneName, pSeqData);
		m_pDesign = &m_NoteDesign;
	}
	if (result != 0) goto EXIT;

	m_PitchBend.Initialize();
	m_DisplayDuration = m_pDesign->GetLiveMonitorDisplayDuration();

	// minimum note length: enough world-X to be visible (~the note box height) the
	// instant a key is pressed, instead of a zero-length sliver that only becomes
	// visible after it has scrolled out a little ("delayed" look). Derived from the
	// live scroll rate (GetLivePosX(1000) == LiveNoteLengthPerSecond) so it scales.
	{
		float perSec = m_pDesign->GetLivePosX(1000);   // world-X per second
		float minLenX = m_pDesign->GetNoteBoxHeight();
		if (minLenX < 0.1f) minLenX = 0.1f;
		m_MinNoteElapsed = (perSec > 0.0001f) ? (unsigned long)(minLenX * 1000.0f / perSec) : 60;
	}

	// the live now-line (note leading edge) is at x=0, but the keyboard's playing
	// face is one key-length in front of that, so notes appear to overshoot to the
	// right (past the keyboard). Shift the field left by the key length so the
	// leading edge meets the keyboard front. (Ring places notes radially, so no
	// flat X shift there.)
	m_NowLineOffsetX = 0.0f;
	if (!ringMode) {
		MTPianoKeyboardDesign kbdDesign;
		if (kbdDesign.Initialize(pSceneName, pSeqData) == 0) {
			m_NowLineOffsetX = -kbdDesign.GetWhiteKeyLen();
		}
	}

	mv = m_pDesign->GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	try {
		m_pNoteStatus = new NoteStatus[MTNOTEBOXLIVE11_MAX_NOTES];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	Reset();

	m_VertCapacity = MTNBL11_VERTS_PER_NOTE * MTNOTEBOXLIVE11_MAX_NOTES;
	result = m_Prim.CreateVertexBuffer(pDevice, m_VertCapacity);
	if (result != 0) goto EXIT;

	// static index buffer: one box pattern per slot, offset by slot*24
	{
		unsigned long* pi = NULL;
		result = m_Prim.CreateIndexBuffer(pDevice, 36 * MTNOTEBOXLIVE11_MAX_NOTES);
		if (result != 0) goto EXIT;
		result = m_Prim.LockIndex(pContext, &pi);
		if (result != 0) goto EXIT;
		for (s = 0; s < MTNOTEBOXLIVE11_MAX_NOTES; s++) {
			for (i = 0; i < 36; i++) {
				pi[s * 36 + i] = s * MTNBL11_VERTS_PER_NOTE + MTNBL11_BOX_IDX[i];
			}
		}
		m_Prim.UnlockIndex(pContext);
	}

	m_pCpuBuf = malloc((size_t)m_VertCapacity * sizeof(DXP11_VERTEX));
	if (m_pCpuBuf == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	// ced 20260713: DX9 lights the live note boxes only in the 3D scene (MTScenePianoRoll3DLive
	// enables the two directional lights; the 2D and ring live scenes clear m_IsEnableLight).
	// The port used to force ambient 1.0 to keep every scene unlit, which is why the 3D live
	// notes were as flat as the 2D ones. Light the 3D scene, leave the others alone.
	m_Prim.SetLightEnable((pSceneName != NULL) && (_tcsncmp(pSceneName, _T("PianoRoll3D"), 11) == 0));
	m_Prim.SetMaterialAmbient(0.1f, 0.1f, 0.1f);   // note material ambient 0.5 * light ambient 0.2
	m_Prim.SetAdditiveBlend(false);

	m_Ready = true;

EXIT:;
	return result;
}

void MTNoteBoxLive11::Reset()
{
	if (m_pNoteStatus == NULL) return;
	for (unsigned long i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].portNo = 0;
		m_pNoteStatus[i].chNo = 0;
		m_pNoteStatus[i].noteNo = 0;
		m_pNoteStatus[i].startTime = 0;
		m_pNoteStatus[i].endTime = 0;
	}
}

//******************************************************************************
// note-on / note-off (timeGetTime stamped)
//******************************************************************************
void MTNoteBoxLive11::SetNoteOn(
		unsigned char portNo, unsigned char chNo, unsigned char noteNo, unsigned char velocity
	)
{
	unsigned long i = 0, slot = 0;
	bool isFind = false;
	unsigned long curTime = timeGetTime();
	(void)velocity;

	if (m_pNoteStatus == NULL) return;

	// a key can only sound once at a time: if the same (port,ch,note) is still
	// held (a retrigger / a dropped note-off), close it now so it scrolls away as
	// a finished note instead of staying pinned at the now-line forever.
	for (i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime == 0)
			&& (m_pNoteStatus[i].portNo == portNo) && (m_pNoteStatus[i].chNo == chNo)
			&& (m_pNoteStatus[i].noteNo == noteNo)) {
			m_pNoteStatus[i].endTime = curTime;
		}
	}

	// reuse an expired/free slot
	for (i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive) {
			if ((m_pNoteStatus[i].endTime != 0)
				&& ((curTime - m_pNoteStatus[i].endTime) > m_DisplayDuration)) {
				slot = i; isFind = true; break;
			}
		} else {
			slot = i; isFind = true; break;
		}
	}
	if (!isFind) _ClearOldest(&slot);

	m_pNoteStatus[slot].isActive = true;
	m_pNoteStatus[slot].portNo = portNo;
	m_pNoteStatus[slot].chNo = chNo;
	m_pNoteStatus[slot].noteNo = noteNo;
	m_pNoteStatus[slot].startTime = curTime;
	m_pNoteStatus[slot].endTime = 0;
}

void MTNoteBoxLive11::SetNoteOff(
		unsigned char portNo, unsigned char chNo, unsigned char noteNo
	)
{
	if (m_pNoteStatus == NULL) return;
	unsigned long curTime = timeGetTime();
	for (unsigned long i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive
			&& (m_pNoteStatus[i].portNo == portNo)
			&& (m_pNoteStatus[i].chNo == chNo)
			&& (m_pNoteStatus[i].noteNo == noteNo)
			&& (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = curTime;
			break;
		}
	}
}

void MTNoteBoxLive11::AllNoteOff()
{
	if (m_pNoteStatus == NULL) return;
	unsigned long curTime = timeGetTime();
	for (unsigned long i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = curTime;
		}
	}
}

void MTNoteBoxLive11::AllNoteOffOnCh(unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteStatus == NULL) return;
	unsigned long curTime = timeGetTime();
	for (unsigned long i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime == 0)
			&& (m_pNoteStatus[i].portNo == portNo) && (m_pNoteStatus[i].chNo == chNo)) {
			m_pNoteStatus[i].endTime = curTime;
		}
	}
}

void MTNoteBoxLive11::_ClearOldest(unsigned long* pIndex)
{
	unsigned long oldest = 0;
	bool isFind = false;
	for (unsigned long i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (!m_pNoteStatus[i].isActive) continue;
		if (!isFind) { oldest = i; isFind = true; }
		else if (m_pNoteStatus[i].startTime < m_pNoteStatus[oldest].startTime) oldest = i;
	}
	m_pNoteStatus[oldest].isActive = false;
	*pIndex = oldest;
}

void MTNoteBoxLive11::_ExpireOld(unsigned long curTime)
{
	for (unsigned long i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (m_pNoteStatus[i].isActive && (m_pNoteStatus[i].endTime != 0)
			&& ((curTime - m_pNoteStatus[i].endTime) > m_DisplayDuration)) {
			m_pNoteStatus[i].isActive = false;
		}
	}
}

//******************************************************************************
// build one note box (24 verts) - mirrors MTNoteBoxLive::_CreateVertexOfNote
//******************************************************************************
void MTNoteBoxLive11::_BuildNoteBox(
		const NoteStatus& note, unsigned long curTime, DXP11_VERTEX* pv
	)
{
	D3DXVECTOR3 sLU, sRU, sLD, sRD;   // start (leading) face
	D3DXVECTOR3 eLU, eRU, eLD, eRD;   // end (trailing) face
	short pbValue = 0;
	unsigned char pbSens = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	if (note.endTime == 0) {   // held note bends in pitch
		pbValue = _Bend()->GetValue(note.portNo, note.chNo);
		pbSens  = _Bend()->GetSensitivity(note.portNo, note.chNo);
	}

	unsigned long startElapsed = curTime - note.startTime;
	unsigned long endElapsed   = (note.endTime != 0) ? (curTime - note.endTime) : 0;
	// keep at least a minimum length so a just-pressed note shows immediately at
	// the now-line (the leading edge) rather than emerging late.
	if (startElapsed < endElapsed + m_MinNoteElapsed) startElapsed = endElapsed + m_MinNoteElapsed;
	if (startElapsed > m_DisplayDuration) startElapsed = m_DisplayDuration;

	m_pDesign->GetNoteBoxVirtexPosLive(startElapsed, note.portNo, note.chNo, note.noteNo,
			&sLU, &sRU, &sLD, &sRD, pbValue, pbSens);
	m_pDesign->GetNoteBoxVirtexPosLive(endElapsed, note.portNo, note.chNo, note.noteNo,
			&eLU, &eRU, &eLD, &eRD, pbValue, pbSens);

	// vertex positions (DX9 24-vertex box layout)
	D3DXVECTOR3 P[24];
	P[0]=sLU; P[1]=eLU; P[2]=sRU; P[3]=eRU;                 // top
	P[4]=sRD; P[5]=eRD; P[6]=sLD; P[7]=eLD;                 // bottom
	P[8]=P[2]; P[9]=P[3]; P[10]=P[4]; P[11]=P[5];           // right
	P[12]=P[6]; P[13]=P[7]; P[14]=P[0]; P[15]=P[1];         // left
	P[16]=P[0]; P[17]=P[2]; P[18]=P[6]; P[19]=P[4];         // front
	P[20]=P[3]; P[21]=P[1]; P[22]=P[5]; P[23]=P[7];         // back

	static const float N[6][3] = {
		{0,1,0},{0,-1,0},{0,0,-1},{0,0,1},{-1,0,0},{1,0,0}
	};

	// color: held -> time-varying active color; finished -> base note color
	D3DXCOLOR color;
	if (note.endTime != 0) {
		color = m_pDesign->GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	} else {
		color = m_pDesign->GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo,
				curTime - note.startTime);
	}
	unsigned long c = (unsigned long)color;

	for (int i = 0; i < 24; i++) {
		pv[i].pos[0] = P[i].x + m_NowLineOffsetX; pv[i].pos[1] = P[i].y; pv[i].pos[2] = P[i].z;
		pv[i].normal[0] = N[i/4][0]; pv[i].normal[1] = N[i/4][1]; pv[i].normal[2] = N[i/4][2];
		pv[i].color = c;
		pv[i].uv[0] = 0.0f; pv[i].uv[1] = 0.0f;
	}
}

//******************************************************************************
// build the vertex buffer for all active notes (packed at the front)
//******************************************************************************
unsigned long MTNoteBoxLive11::_BuildVertices(unsigned long curTime, DXP11_VERTEX* pBuf)
{
	unsigned long num = 0;
	for (unsigned long i = 0; i < MTNOTEBOXLIVE11_MAX_NOTES; i++) {
		if (!m_pNoteStatus[i].isActive) continue;
		_BuildNoteBox(m_pNoteStatus[i], curTime, &pBuf[num * MTNBL11_VERTS_PER_NOTE]);
		num++;
	}
	return num;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteBoxLive11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle
	)
{
	unsigned long curTime = 0;
	unsigned long activeNum = 0;
	DXP11_VERTEX* pv = NULL;

	if (!m_Ready) return 0;

	curTime = timeGetTime();
	_ExpireOld(curTime);

	activeNum = _BuildVertices(curTime, (DXP11_VERTEX*)m_pCpuBuf);
	if (activeNum == 0) return 0;
	if (activeNum > MTNOTEBOXLIVE11_MAX_NOTES) activeNum = MTNOTEBOXLIVE11_MAX_NOTES;

	if (m_Prim.LockVertex(pContext, &pv) != 0) return 0;
	memcpy(pv, m_pCpuBuf, (size_t)activeNum * MTNBL11_VERTS_PER_NOTE * sizeof(DXP11_VERTEX));
	m_Prim.UnlockVertex(pContext);

	// world = RotX(roll) * Trans(worldMove)  (same frame as the note field)
	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
	               * XMMatrixTranslation(m_WorldMove.x, m_WorldMove.y, m_WorldMove.z);
	m_Prim.SetWorldMatrix(world);

	return m_Prim.Draw(pContext, viewProj, lightDir, (int)(MTNBL11_TRIS_PER_NOTE * activeNum), 0);
}
