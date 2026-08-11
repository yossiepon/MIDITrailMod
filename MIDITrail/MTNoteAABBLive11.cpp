//******************************************************************************
//
// MIDITrail / MTNoteAABBLive11
//
// AABB live note renderer (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTNoteAABBLive11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

//******************************************************************************
// Vertex/index counts per mode
//******************************************************************************
#define NOTE_VERTEX_NUM_BOX   (4 * 6)
#define NOTE_INDEX_NUM_BOX    (3 * 2 * 6)
#define NOTE_VERTEX_NUM_FLAT  (4)
#define NOTE_INDEX_NUM_FLAT   (6)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteAABBLive11::MTNoteAABBLive11()
{
	m_pContext = NULL;
	m_Mode = MTAABBLiveMode::Roll3D;
	m_isLightEnable = true;
	m_NoteVertexNum = NOTE_VERTEX_NUM_BOX;
	m_NoteIndexNum = NOTE_INDEX_NUM_BOX;
}

MTNoteAABBLive11::~MTNoteAABBLive11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteAABBLive11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		MTNotePitchBend* pNotePitchBend,
		MTAABBLiveMode mode
	)
{
	int result = 0;

	Release();
	m_pContext = pContext;
	m_pNotePitchBend = pNotePitchBend;
	m_Mode = mode;

	if (m_Mode == MTAABBLiveMode::Roll3D) {
		m_NoteVertexNum = NOTE_VERTEX_NUM_BOX;
		m_NoteIndexNum = NOTE_INDEX_NUM_BOX;
	}
	else {
		m_NoteVertexNum = NOTE_VERTEX_NUM_FLAT;
		m_NoteIndexNum = NOTE_INDEX_NUM_FLAT;
	}

	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;

	if (m_Mode == MTAABBLiveMode::Rain) {
		result = m_KeyboardDesign.Initialize(pSceneName, NULL);
		if (result != 0) goto EXIT;
	}

	m_LiveMonitorDisplayDuration = m_NoteDesign.GetLiveMonitorDisplayDuration();

	result = _CreateNoteBuffer(pDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create note buffer (DYNAMIC)
//******************************************************************************
int MTNoteAABBLive11::_CreateNoteBuffer(ID3D11Device* pDevice)
{
	int result = 0;

	result = m_PrimNotes.CreateVertexBuffer(pDevice,
		m_NoteVertexNum * MTNOTELIVENOTE_MAX_NUM);
	if (result != 0) goto EXIT;

	result = m_PrimNotes.CreateIndexBuffer(pDevice,
		m_NoteIndexNum * MTNOTELIVENOTE_MAX_NUM);
	if (result != 0) goto EXIT;

	m_PrimNotes.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

EXIT:;
	return result;
}

//******************************************************************************
// Update
//******************************************************************************
int MTNoteAABBLive11::Update(const MTSceneUpdateContext& ctx)
{
	int result = 0;

	m_LiveTimeMSec = ctx.liveTimeMSec;

	_UpdateStatusOfNotes(m_LiveTimeMSec);

	result = _UpdateVertexOfNotes(m_LiveTimeMSec);
	if (result != 0) goto EXIT;

	// World matrix
	{
		Matrix world;
		if (m_Mode == MTAABBLiveMode::Rain) {
			world = Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
		}
		else {
			Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
			world = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle))
			      * Matrix::CreateTranslation(moveVec);
		}
		m_PrimNotes.SetWorldMatrix(world);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update vertex of notes
//******************************************************************************
int MTNoteAABBLive11::_UpdateVertexOfNotes(unsigned long curTimeMs)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long noteNum = 0;

	result = m_PrimNotes.LockVertex(m_pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimNotes.LockIndex(m_pContext, &pIndex);
	if (result != 0) goto EXIT;

	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			result = _CreateVertexOfNote(
				m_NoteStatus[i],
				&pVertex[m_NoteVertexNum * noteNum],
				m_NoteVertexNum * noteNum,
				&pIndex[m_NoteIndexNum * noteNum],
				curTimeMs
			);
			if (result != 0) goto EXIT;
			noteNum++;
		}
	}
	m_NoteNum = noteNum;

	m_PrimNotes.UnlockVertex(m_pContext);
	m_PrimNotes.UnlockIndex(m_pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Create vertex of note
//******************************************************************************
int MTNoteAABBLive11::_CreateVertexOfNote(
		const NoteStatus& note,
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		unsigned long curTimeMs
	)
{
	int result = 0;
	Vector3 vectorStartLU, vectorStartRU, vectorStartLD, vectorStartRD;
	Vector3 vectorEndLU, vectorEndRU, vectorEndLD, vectorEndRD;
	short pbValue = 0;
	unsigned char pbSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	if ((note.endTime == 0) && (m_pNotePitchBend != NULL)) {
		pbValue = m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		pbSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
	}

	// Start/End elapsed time
	unsigned long startElapsed = curTimeMs - note.startTime;
	if (startElapsed > m_LiveMonitorDisplayDuration) {
		startElapsed = m_LiveMonitorDisplayDuration;
	}
	unsigned long endElapsed = 0;
	if (note.endTime != 0) {
		endElapsed = curTimeMs - note.endTime;
	}
	if (startElapsed < endElapsed + 50) {
		startElapsed = endElapsed + 50;
	}

	// Color
	Color color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	if (note.endTime == 0) {
		unsigned long elapsedTime = curTimeMs - note.startTime;
		float rate = 0.0f;
		unsigned long decayDuration = m_NoteDesign.GetRippleDecayDuration();
		if (elapsedTime < decayDuration) {
			rate = 1.0f - ((float)elapsedTime / (float)decayDuration);
		}
		color = m_NoteDesign.GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, rate);
	}

	if (m_Mode == MTAABBLiveMode::Rain) {
		// Rain mode: keyboard-based X positioning, time on Y axis
		float rainWidth = m_KeyboardDesign.GetBlackKeyWidth();
		float pitchBendShift = 0.0f;
		if ((note.endTime == 0) && (m_pNotePitchBend != NULL)) {
			pitchBendShift = m_KeyboardDesign.GetPitchBendShift(pbValue, pbSensitivity);
		}

		Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
		moveVec.x += m_KeyboardDesign.GetKeyCenterPosX(note.noteNo);
		moveVec.y += m_KeyboardDesign.GetWhiteKeyHeight() / 2.0f;
		moveVec.z += m_KeyboardDesign.GetNoteDropPosZ(note.noteNo);

		Vector3 startPos(pitchBendShift + moveVec.x, m_NoteDesign.GetLivePosX(startElapsed) + moveVec.y, moveVec.z);
		Vector3 endPos(pitchBendShift + moveVec.x, m_NoteDesign.GetLivePosX(endElapsed) + moveVec.y, moveVec.z);

		pVertex[0].pos[0] = startPos.x - rainWidth / 2.0f; pVertex[0].pos[1] = startPos.y; pVertex[0].pos[2] = startPos.z;
		pVertex[1].pos[0] = startPos.x + rainWidth / 2.0f; pVertex[1].pos[1] = startPos.y; pVertex[1].pos[2] = startPos.z;
		pVertex[2].pos[0] = endPos.x   + rainWidth / 2.0f; pVertex[2].pos[1] = endPos.y;   pVertex[2].pos[2] = endPos.z;
		pVertex[3].pos[0] = endPos.x   - rainWidth / 2.0f; pVertex[3].pos[1] = endPos.y;   pVertex[3].pos[2] = endPos.z;

		unsigned long cStart = Color(color.R(), color.G(), color.B(), 1.0f).BGRA();
		unsigned long cEnd   = Color(color.R(), color.G(), color.B(), 0.5f).BGRA();
		float nrm[3] = { 0.0f, 1.0f, 0.0f };
		for (int i = 0; i < 4; i++) {
			memcpy(pVertex[i].normal, nrm, sizeof(float) * 3);
			pVertex[i].uv[0] = 0.0f;
			pVertex[i].uv[1] = 0.0f;
		}
		pVertex[0].color = cStart;
		pVertex[1].color = cStart;
		pVertex[2].color = cEnd;
		pVertex[3].color = cEnd;

		unsigned long rainIndices[6] = { 0, 2, 1, 0, 3, 2 };
		for (int i = 0; i < 6; i++) {
			pIndex[i] = vertexOffset + rainIndices[i];
		}
	}
	else if (m_Mode == MTAABBLiveMode::Roll2D) {
		// Roll2D: flat quad in XY plane (time x note height)
		m_NoteDesign.GetNoteBoxVirtexPosLive(
			startElapsed, note.portNo, note.chNo, note.noteNo,
			&vectorStartLU, &vectorStartRU, &vectorStartLD, &vectorStartRD,
			pbValue, pbSensitivity);
		m_NoteDesign.GetNoteBoxVirtexPosLive(
			endElapsed, note.portNo, note.chNo, note.noteNo,
			&vectorEndLU, &vectorEndRU, &vectorEndLD, &vectorEndRD,
			pbValue, pbSensitivity);

		unsigned long c = color.BGRA();

		// Start-Upper, End-Upper, Start-Lower, End-Lower
		pVertex[0].pos[0] = vectorStartLU.x; pVertex[0].pos[1] = vectorStartLU.y; pVertex[0].pos[2] = vectorStartLU.z;
		pVertex[1].pos[0] = vectorEndLU.x;   pVertex[1].pos[1] = vectorEndLU.y;   pVertex[1].pos[2] = vectorEndLU.z;
		pVertex[2].pos[0] = vectorStartLD.x; pVertex[2].pos[1] = vectorStartLD.y; pVertex[2].pos[2] = vectorStartLD.z;
		pVertex[3].pos[0] = vectorEndLD.x;   pVertex[3].pos[1] = vectorEndLD.y;   pVertex[3].pos[2] = vectorEndLD.z;

		float nrm[3] = { 0.0f, 0.0f, -1.0f };
		for (int i = 0; i < 4; i++) {
			memcpy(pVertex[i].normal, nrm, sizeof(float) * 3);
			pVertex[i].color = c;
			pVertex[i].uv[0] = 0.0f;
			pVertex[i].uv[1] = 0.0f;
		}

		unsigned long flatIndices[6] = { 0, 1, 2, 2, 1, 3 };
		for (int i = 0; i < 6; i++) {
			pIndex[i] = vertexOffset + flatIndices[i];
		}
	}
	else {
		// Box3D mode: NoteDesign-based positioning
		m_NoteDesign.GetNoteBoxVirtexPosLive(
			startElapsed, note.portNo, note.chNo, note.noteNo,
			&vectorStartLU, &vectorStartRU, &vectorStartLD, &vectorStartRD,
			pbValue, pbSensitivity);
		m_NoteDesign.GetNoteBoxVirtexPosLive(
			endElapsed, note.portNo, note.chNo, note.noteNo,
			&vectorEndLU, &vectorEndRU, &vectorEndLD, &vectorEndRD,
			pbValue, pbSensitivity);

		unsigned long c = color.BGRA();

		// Box3D: 6 faces, 24 vertices, 36 indices
		// Top face
		pVertex[0].pos[0] = vectorStartLU.x; pVertex[0].pos[1] = vectorStartLU.y; pVertex[0].pos[2] = vectorStartLU.z;
		pVertex[1].pos[0] = vectorEndLU.x;   pVertex[1].pos[1] = vectorEndLU.y;   pVertex[1].pos[2] = vectorEndLU.z;
		pVertex[2].pos[0] = vectorStartRU.x; pVertex[2].pos[1] = vectorStartRU.y; pVertex[2].pos[2] = vectorStartRU.z;
		pVertex[3].pos[0] = vectorEndRU.x;   pVertex[3].pos[1] = vectorEndRU.y;   pVertex[3].pos[2] = vectorEndRU.z;
		// Bottom face
		pVertex[4].pos[0] = vectorStartRD.x; pVertex[4].pos[1] = vectorStartRD.y; pVertex[4].pos[2] = vectorStartRD.z;
		pVertex[5].pos[0] = vectorEndRD.x;   pVertex[5].pos[1] = vectorEndRD.y;   pVertex[5].pos[2] = vectorEndRD.z;
		pVertex[6].pos[0] = vectorStartLD.x; pVertex[6].pos[1] = vectorStartLD.y; pVertex[6].pos[2] = vectorStartLD.z;
		pVertex[7].pos[0] = vectorEndLD.x;   pVertex[7].pos[1] = vectorEndLD.y;   pVertex[7].pos[2] = vectorEndLD.z;
		// Right face
		pVertex[8]  = pVertex[2]; pVertex[9]  = pVertex[3];
		pVertex[10] = pVertex[4]; pVertex[11] = pVertex[5];
		// Left face
		pVertex[12] = pVertex[6]; pVertex[13] = pVertex[7];
		pVertex[14] = pVertex[0]; pVertex[15] = pVertex[1];
		// Front face
		pVertex[16] = pVertex[0]; pVertex[17] = pVertex[2];
		pVertex[18] = pVertex[6]; pVertex[19] = pVertex[4];
		// Back face
		pVertex[20] = pVertex[3]; pVertex[21] = pVertex[1];
		pVertex[22] = pVertex[5]; pVertex[23] = pVertex[7];

		// Normals
		Vector3 normals[6] = {
			Vector3( 0.0f,  1.0f,  0.0f),
			Vector3( 0.0f, -1.0f,  0.0f),
			Vector3( 0.0f,  0.0f, -1.0f),
			Vector3( 0.0f,  0.0f,  1.0f),
			Vector3(-1.0f,  0.0f,  0.0f),
			Vector3( 1.0f,  0.0f,  0.0f),
		};
		for (int face = 0; face < 6; face++) {
			for (int v = 0; v < 4; v++) {
				int idx = face * 4 + v;
				pVertex[idx].normal[0] = normals[face].x;
				pVertex[idx].normal[1] = normals[face].y;
				pVertex[idx].normal[2] = normals[face].z;
			}
		}

		for (int i = 0; i < 24; i++) {
			pVertex[i].color = c;
			pVertex[i].uv[0] = 0.0f;
			pVertex[i].uv[1] = 0.0f;
		}

		unsigned long boxIndices[36] = {
			 0,  1,  2,   2,  1,  3,
			 4,  5,  6,   6,  5,  7,
			 8,  9, 10,  10,  9, 11,
			12, 13, 14,  14, 13, 15,
			16, 17, 18,  18, 17, 19,
			20, 21, 22,  22, 21, 23,
		};
		for (int i = 0; i < 36; i++) {
			pIndex[i] = vertexOffset + boxIndices[i];
		}
	}

	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteAABBLive11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	if (m_NoteNum > 0) {
		m_PrimNotes.SetLightEnable(m_isLightEnable);
		result = m_PrimNotes.Draw(pContext, viewProj, lightDir,
			(int)(m_NoteNum * m_NoteIndexNum / 3));
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteAABBLive11::Release()
{
	m_PrimNotes.Release();
	m_pContext = NULL;
	m_pNotePitchBend = NULL;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTNoteAABBLive11::Reset()
{
	MTNoteLiveBase11::Reset();
}
