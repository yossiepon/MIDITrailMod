//******************************************************************************
//
// MIDITrail / MTNoteBox11
//
// DX11 note box renderer.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteBox11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteBox11::MTNoteBox11()
{
	m_pContext = NULL;
	m_pNoteDesign = NULL;
	m_pNoteTracker = NULL;
	m_pNotePitchBend = NULL;
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;
	m_isSkipping = false;
	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));
	ZeroMemory(m_NoteStatus, sizeof(m_NoteStatus));
}

MTNoteBox11::~MTNoteBox11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteBox11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNoteTracker* pNoteTracker,
		MTNotePitchBend* pNotePitchBend,
		MTNoteDesignMod* pNoteDesign
	)
{
	int result = 0;

	Release();

	if (pSeqData == NULL || pNoteTracker == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_pContext = pContext;
	m_pNoteTracker = pNoteTracker;
	m_pNotePitchBend = pNotePitchBend;

	if (pNoteDesign != NULL) {
		m_pNoteDesign = pNoteDesign;
	}
	else {
		result = m_NoteDesignLocal.Initialize(pSceneName, pSeqData);
		if (result != 0) goto EXIT;
		m_pNoteDesign = &m_NoteDesignLocal;
	}

	result = _CreateAllNoteBox(pDevice);
	if (result != 0) goto EXIT;

	result = _CreateActiveNoteBox(pDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteBox11::Release()
{
	m_PrimAllNotes.Release();
	m_PrimActiveNotes.Release();
	m_pContext = NULL;
	m_pNoteTracker = NULL;
	m_pNotePitchBend = NULL;
}

//******************************************************************************
// Create all note boxes
//******************************************************************************
int MTNoteBox11::_CreateAllNoteBox(
		ID3D11Device* pDevice
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	if (m_pNoteTracker == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	unsigned long noteCount = m_pNoteTracker->GetNoteCount();
	if (noteCount == 0) goto EXIT;

	result = m_PrimAllNotes.CreateVertexBuffer(pDevice,
		MTNOTEBOX11_NOTE_VERTEX_NUM * noteCount);
	if (result != 0) goto EXIT;

	result = m_PrimAllNotes.CreateIndexBuffer(pDevice,
		MTNOTEBOX11_NOTE_INDEX_NUM * noteCount);
	if (result != 0) goto EXIT;

	result = m_PrimAllNotes.LockVertex(m_pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimAllNotes.LockIndex(m_pContext, &pIndex);
	if (result != 0) goto EXIT;

	for (unsigned long i = 0; i < noteCount; i++) {
		const NoteData& note = m_pNoteTracker->GetNote(i);
		result = _CreateVertexOfNote(
			note,
			&pVertex[MTNOTEBOX11_NOTE_VERTEX_NUM * i],
			MTNOTEBOX11_NOTE_VERTEX_NUM * i,
			&pIndex[MTNOTEBOX11_NOTE_INDEX_NUM * i]
		);
		if (result != 0) goto EXIT;
	}

	m_PrimAllNotes.UnlockVertex(m_pContext);
	m_PrimAllNotes.UnlockIndex(m_pContext);

	m_PrimAllNotes.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

EXIT:;
	return result;
}

//******************************************************************************
// Create active note box buffer
//******************************************************************************
int MTNoteBox11::_CreateActiveNoteBox(
		ID3D11Device* pDevice
	)
{
	int result = 0;

	result = m_PrimActiveNotes.CreateVertexBuffer(pDevice,
		MTNOTEBOX11_NOTE_VERTEX_NUM * MTNOTEBOX11_MAX_ACTIVENOTE_NUM);
	if (result != 0) goto EXIT;

	result = m_PrimActiveNotes.CreateIndexBuffer(pDevice,
		MTNOTEBOX11_NOTE_INDEX_NUM * MTNOTEBOX11_MAX_ACTIVENOTE_NUM);
	if (result != 0) goto EXIT;

	m_PrimActiveNotes.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

EXIT:;
	return result;
}

//******************************************************************************
// Update
//******************************************************************************
int MTNoteBox11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	int result = 0;

	m_CurTickTime = ctx.curTickTime;
	m_PlayTimeMSec = ctx.playTimeMSec;

	if (m_isSkipping) goto EXIT;

	result = _UpdateStatusOfActiveNotes();
	if (result != 0) goto EXIT;

	result = _UpdateVertexOfActiveNotes();
	if (result != 0) goto EXIT;

	// World matrix: Rotation(rollAngle) * Translation(WorldMoveVector)
	{
		Vector3 moveVec = m_pNoteDesign->GetWorldMoveVector();
		Matrix world = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle))
		             * Matrix::CreateTranslation(moveVec);
		m_PrimAllNotes.SetWorldMatrix(world);
		m_PrimActiveNotes.SetWorldMatrix(world);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update status of active notes (tick-based forward scan)
//******************************************************************************
int MTNoteBox11::_UpdateStatusOfActiveNotes()
{
	int result = 0;

	if (m_pNoteTracker == NULL) goto EXIT;

	unsigned long decayDuration = m_pNoteDesign->GetRippleDecayDuration();
	unsigned long releaseDuration = m_pNoteDesign->GetRippleReleaseDuration();

	// Update existing active notes
	for (unsigned long i = 0; i < MTNOTEBOX11_MAX_ACTIVENOTE_NUM; i++) {
		if (!m_NoteStatus[i].isActive) continue;

		const NoteData& note = m_pNoteTracker->GetNote(m_NoteStatus[i].index);

		if (m_PlayTimeMSec > note.endTimeMs) {
			m_NoteStatus[i].isActive = false;
			m_NoteStatus[i].index = 0;
			m_NoteStatus[i].keyStatus = BeforeNoteON;
			m_NoteStatus[i].keyDownRate = 0.0f;
		}
		else {
			MTNoteEnvelopeResult env = m_pNoteDesign->CalcNoteEnvelope(
				m_PlayTimeMSec, note.startTimeMs, note.endTimeMs);
			m_NoteStatus[i].keyDownRate = env.keyDownRate;
			m_NoteStatus[i].keyStatus = env.keyStatus;
		}
	}

	// Forward scan for new active notes (tick-based)
	unsigned long noteCount = m_pNoteTracker->GetNoteCount();

	while (m_CurNoteIndex < noteCount) {
		const NoteData& note = m_pNoteTracker->GetNote(m_CurNoteIndex);

		if (m_CurTickTime < note.startTimeTick) break;

		if ((note.startTimeTick <= m_CurTickTime) && (m_CurTickTime <= note.endTimeTick)) {
			// Check if already registered
			bool isFound = false;
			for (unsigned long i = 0; i < MTNOTEBOX11_MAX_ACTIVENOTE_NUM; i++) {
				if (m_NoteStatus[i].isActive && m_NoteStatus[i].index == m_CurNoteIndex) {
					isFound = true;
					break;
				}
			}
			// Register in empty slot
			if (!isFound) {
				for (unsigned long i = 0; i < MTNOTEBOX11_MAX_ACTIVENOTE_NUM; i++) {
					if (!m_NoteStatus[i].isActive) {
						m_NoteStatus[i].isActive = true;
						m_NoteStatus[i].index = m_CurNoteIndex;
						m_NoteStatus[i].keyStatus = BeforeNoteON;
						m_NoteStatus[i].keyDownRate = 0.0f;
						break;
					}
				}
			}
		}
		m_CurNoteIndex++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update vertex of active notes
//******************************************************************************
int MTNoteBox11::_UpdateVertexOfActiveNotes()
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndexActive = NULL;
	unsigned long* pIndexAll = NULL;
	unsigned long activeNoteNum = 0;
	unsigned long noteCount = 0;

	if (m_pContext == NULL || m_pNoteTracker == NULL) goto EXIT;

	noteCount = m_pNoteTracker->GetNoteCount();

	// Lock active notes buffer
	result = m_PrimActiveNotes.LockVertex(m_pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimActiveNotes.LockIndex(m_pContext, &pIndexActive);
	if (result != 0) goto EXIT;

	// Lock all notes index buffer (WRITE_DISCARD: rebuild entire buffer each frame)
	result = m_PrimAllNotes.LockIndex(m_pContext, &pIndexAll);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));

	// Rebuild all-notes index buffer: show all notes, hide active ones
	{
		unsigned long baseIndices[MTNOTEBOX11_NOTE_INDEX_NUM] = {
			 0,  1,  2,   2,  1,  3,
			 4,  5,  6,   6,  5,  7,
			 8,  9, 10,  10,  9, 11,
			12, 13, 14,  14, 13, 15,
			16, 17, 18,  18, 17, 19,
			20, 21, 22,  22, 21, 23,
		};

		for (unsigned long n = 0; n < noteCount; n++) {
			unsigned long vertexOffset = MTNOTEBOX11_NOTE_VERTEX_NUM * n;
			unsigned long indexOffset = MTNOTEBOX11_NOTE_INDEX_NUM * n;

			// Check if this note is currently active
			bool isActive = false;
			for (unsigned long i = 0; i < MTNOTEBOX11_MAX_ACTIVENOTE_NUM; i++) {
				if (m_NoteStatus[i].isActive && m_NoteStatus[i].index == n) {
					isActive = true;
					break;
				}
			}

			if (isActive) {
				// Hide: set all indices to 0
				for (int j = 0; j < MTNOTEBOX11_NOTE_INDEX_NUM; j++) {
					pIndexAll[indexOffset + j] = 0;
				}
			}
			else {
				// Show: set proper indices
				for (int j = 0; j < MTNOTEBOX11_NOTE_INDEX_NUM; j++) {
					pIndexAll[indexOffset + j] = vertexOffset + baseIndices[j];
				}
			}
		}
	}

	// Build active notes vertex/index
	for (unsigned long i = 0; i < MTNOTEBOX11_MAX_ACTIVENOTE_NUM; i++) {
		if (!m_NoteStatus[i].isActive) continue;

		const NoteData& note = m_pNoteTracker->GetNote(m_NoteStatus[i].index);

		result = _CreateVertexOfNote(
			note,
			&pVertex[MTNOTEBOX11_NOTE_VERTEX_NUM * activeNoteNum],
			MTNOTEBOX11_NOTE_VERTEX_NUM * activeNoteNum,
			&pIndexActive[MTNOTEBOX11_NOTE_INDEX_NUM * activeNoteNum],
			m_NoteStatus[i].keyDownRate,
			true
		);
		if (result != 0) goto EXIT;

		activeNoteNum++;
		m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_NoteStatus[i].keyDownRate;
	}
	m_ActiveNoteNum = activeNoteNum;

	m_PrimAllNotes.UnlockIndex(m_pContext);
	m_PrimActiveNotes.UnlockVertex(m_pContext);
	m_PrimActiveNotes.UnlockIndex(m_pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Create vertex of a single note (24 vertices, 36 indices)
//******************************************************************************
int MTNoteBox11::_CreateVertexOfNote(
		const NoteData& note,
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		float keyDownRate,
		bool isEnablePitchBend
	)
{
	int result = 0;
	Vector3 vectorStartLU, vectorStartRU, vectorStartLD, vectorStartRD;
	Vector3 vectorEndLU, vectorEndRU, vectorEndLD, vectorEndRD;
	short pbValue = 0;
	unsigned char pbSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	if (isEnablePitchBend && m_pNotePitchBend != NULL) {
		pbValue = m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		pbSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
	}

	if (keyDownRate == 0.0f) {
		m_pNoteDesign->GetNoteBoxVirtexPos(
			note.startTimeTick, note.portNo, note.chNo, note.noteNo,
			&vectorStartLU, &vectorStartRU, &vectorStartLD, &vectorStartRD,
			pbValue, pbSensitivity);
		m_pNoteDesign->GetNoteBoxVirtexPos(
			note.endTimeTick, note.portNo, note.chNo, note.noteNo,
			&vectorEndLU, &vectorEndRU, &vectorEndLD, &vectorEndRD,
			pbValue, pbSensitivity);
	}
	else {
		m_pNoteDesign->GetActiveNoteBoxVirtexPos(
			note.startTimeTick, note.portNo, note.chNo, note.noteNo,
			&vectorStartLU, &vectorStartRU, &vectorStartLD, &vectorStartRD,
			pbValue, pbSensitivity, keyDownRate);
		m_pNoteDesign->GetActiveNoteBoxVirtexPos(
			note.endTimeTick, note.portNo, note.chNo, note.noteNo,
			&vectorEndLU, &vectorEndRU, &vectorEndLD, &vectorEndRD,
			pbValue, pbSensitivity, keyDownRate);
	}

	//     +   1+----+3   +
	//    /|   / top /    /|         y x
	//   + | 0+----+2   + | right   |/
	// L | +   7+----+5 | +      z--+0
	//   |/    / bot /   |/
	//   +   6+----+4   +

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
		Vector3( 0.0f,  1.0f,  0.0f), // top
		Vector3( 0.0f, -1.0f,  0.0f), // bottom
		Vector3( 0.0f,  0.0f, -1.0f), // right
		Vector3( 0.0f,  0.0f,  1.0f), // left
		Vector3(-1.0f,  0.0f,  0.0f), // front
		Vector3( 1.0f,  0.0f,  0.0f), // back
	};
	for (int face = 0; face < 6; face++) {
		for (int v = 0; v < 4; v++) {
			int idx = face * 4 + v;
			pVertex[idx].normal[0] = normals[face].x;
			pVertex[idx].normal[1] = normals[face].y;
			pVertex[idx].normal[2] = normals[face].z;
		}
	}

	// Color
	unsigned long c;
	if (keyDownRate == 0.0f) {
		c = m_pNoteDesign->GetNoteBoxColor(note.portNo, note.chNo, note.noteNo).BGRA();
	}
	else {
		c = m_pNoteDesign->GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, keyDownRate).BGRA();
	}
	for (int i = 0; i < MTNOTEBOX11_NOTE_VERTEX_NUM; i++) {
		pVertex[i].color = c;
		pVertex[i].uv[0] = 0.0f;
		pVertex[i].uv[1] = 0.0f;
	}

	// Indices
	unsigned long baseIndices[MTNOTEBOX11_NOTE_INDEX_NUM] = {
		 0,  1,  2,   2,  1,  3,  // top
		 4,  5,  6,   6,  5,  7,  // bottom
		 8,  9, 10,  10,  9, 11,  // right
		12, 13, 14,  14, 13, 15,  // left
		16, 17, 18,  18, 17, 19,  // front
		20, 21, 22,  22, 21, 23,  // back
	};
	for (int i = 0; i < MTNOTEBOX11_NOTE_INDEX_NUM; i++) {
		pIndex[i] = vertexOffset + baseIndices[i];
	}

	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteBox11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	// All notes (non-active rendered with original color/size)
	m_PrimAllNotes.SetLightEnable(true);
	m_PrimAllNotes.SetMaterialAmbient(0.5f, 0.5f, 0.5f);
	result = m_PrimAllNotes.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	// Active notes (with envelope-based size/color variation)
	if (m_ActiveNoteNum > 0) {
		m_PrimActiveNotes.SetLightEnable(true);
		m_PrimActiveNotes.SetMaterialAmbient(0.5f, 0.5f, 0.5f);
		result = m_PrimActiveNotes.Draw(pContext, viewProj, lightDir,
			(int)(m_ActiveNoteNum * MTNOTEBOX11_NOTE_INDEX_NUM / 3));
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTNoteBox11::Reset()
{
	for (unsigned long i = 0; i < MTNOTEBOX11_MAX_ACTIVENOTE_NUM; i++) {
		m_NoteStatus[i].isActive = false;
		m_NoteStatus[i].index = 0;
		m_NoteStatus[i].keyStatus = BeforeNoteON;
		m_NoteStatus[i].keyDownRate = 0.0f;
	}

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;
}

//******************************************************************************
// Note count
//******************************************************************************
unsigned long MTNoteBox11::GetNoteCount() const
{
	if (m_pNoteTracker != NULL) {
		return m_pNoteTracker->GetNoteCount();
	}
	return 0;
}
