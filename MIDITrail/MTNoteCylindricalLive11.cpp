//******************************************************************************
//
// MIDITrail / MTNoteCylindricalLive11
//
// Cylindrical (Ring) live note renderer (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTNoteCylindricalLive11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

//******************************************************************************
// Vertex/index counts (Box3D only for Ring)
//******************************************************************************
#define NOTE_VERTEX_NUM  (4)
#define NOTE_INDEX_NUM   (6)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteCylindricalLive11::MTNoteCylindricalLive11()
{
	m_pContext = NULL;
	m_isLightEnable = true;
}

MTNoteCylindricalLive11::~MTNoteCylindricalLive11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteCylindricalLive11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;

	Release();
	m_pContext = pContext;
	m_pNotePitchBend = pNotePitchBend;

	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;

	m_LiveMonitorDisplayDuration = m_NoteDesign.GetLiveMonitorDisplayDuration();

	result = _CreateNoteBuffer(pDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create note buffer (DYNAMIC)
//******************************************************************************
int MTNoteCylindricalLive11::_CreateNoteBuffer(ID3D11Device* pDevice)
{
	int result = 0;

	result = m_PrimNotes.CreateVertexBuffer(pDevice,
		NOTE_VERTEX_NUM * MTNOTELIVENOTE_MAX_NUM);
	if (result != 0) goto EXIT;

	result = m_PrimNotes.CreateIndexBuffer(pDevice,
		NOTE_INDEX_NUM * MTNOTELIVENOTE_MAX_NUM);
	if (result != 0) goto EXIT;

	m_PrimNotes.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

EXIT:;
	return result;
}

//******************************************************************************
// Update
//******************************************************************************
int MTNoteCylindricalLive11::Update(const MTSceneUpdateContext& ctx)
{
	int result = 0;

	m_LiveTimeMSec = ctx.liveTimeMSec;

	_UpdateStatusOfNotes(m_LiveTimeMSec);

	result = _UpdateVertexOfNotes(m_LiveTimeMSec);
	if (result != 0) goto EXIT;

	// World matrix
	{
		Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
		Matrix world = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle))
		             * Matrix::CreateTranslation(moveVec);
		m_PrimNotes.SetWorldMatrix(world);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update vertex of notes
//******************************************************************************
int MTNoteCylindricalLive11::_UpdateVertexOfNotes(unsigned long curTimeMs)
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
				&pVertex[NOTE_VERTEX_NUM * noteNum],
				NOTE_VERTEX_NUM * noteNum,
				&pIndex[NOTE_INDEX_NUM * noteNum],
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
int MTNoteCylindricalLive11::_CreateVertexOfNote(
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

	// Elapsed time calculation (both values adjusted before vertex generation)
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

	// Start position
	m_NoteDesign.GetNoteBoxVirtexPosLive(
		startElapsed, note.portNo, note.chNo, note.noteNo,
		&vectorStartLU, &vectorStartRU, &vectorStartLD, &vectorStartRD,
		pbValue, pbSensitivity);

	// End position
	m_NoteDesign.GetNoteBoxVirtexPosLive(
		endElapsed, note.portNo, note.chNo, note.noteNo,
		&vectorEndLU, &vectorEndRU, &vectorEndLD, &vectorEndRD,
		pbValue, pbSensitivity);

	// Color
	unsigned long c;
	if (note.endTime != 0) {
		c = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo).BGRA();
	}
	else {
		unsigned long elapsedTime = curTimeMs - note.startTime;
		float rate = 0.0f;
		unsigned long decayDuration = m_NoteDesign.GetRippleDecayDuration();
		if (elapsedTime < decayDuration) {
			rate = 1.0f - ((float)elapsedTime / (float)decayDuration);
		}
		c = m_NoteDesign.GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, rate).BGRA();
	}

	// Flat arc: 4 vertices, 6 indices
	pVertex[0].pos[0] = vectorStartLU.x; pVertex[0].pos[1] = vectorStartLU.y; pVertex[0].pos[2] = vectorStartLU.z;
	pVertex[1].pos[0] = vectorStartRU.x; pVertex[1].pos[1] = vectorStartRU.y; pVertex[1].pos[2] = vectorStartRU.z;
	pVertex[2].pos[0] = vectorEndLU.x;   pVertex[2].pos[1] = vectorEndLU.y;   pVertex[2].pos[2] = vectorEndLU.z;
	pVertex[3].pos[0] = vectorEndRU.x;   pVertex[3].pos[1] = vectorEndRU.y;   pVertex[3].pos[2] = vectorEndRU.z;

	for (int i = 0; i < 4; i++) {
		Vector3 n(0.0f, pVertex[i].pos[1], pVertex[i].pos[2]);
		n.Normalize();
		pVertex[i].normal[0] = n.x;
		pVertex[i].normal[1] = n.y;
		pVertex[i].normal[2] = n.z;
		pVertex[i].color = c;
		pVertex[i].uv[0] = 0.0f;
		pVertex[i].uv[1] = 0.0f;
	}

	unsigned long indices[6] = { 0, 2, 1, 1, 2, 3 };
	for (int i = 0; i < 6; i++) {
		pIndex[i] = vertexOffset + indices[i];
	}

	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteCylindricalLive11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	if (m_NoteNum > 0) {
		m_PrimNotes.SetLightEnable(m_isLightEnable);
		m_PrimNotes.SetMaterialAmbient(0.2f, 0.2f, 0.2f);
		result = m_PrimNotes.Draw(pContext, viewProj, lightDir,
			(int)(m_NoteNum * NOTE_INDEX_NUM / 3));
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteCylindricalLive11::Release()
{
	m_PrimNotes.Release();
	m_pContext = NULL;
	m_pNotePitchBend = NULL;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTNoteCylindricalLive11::Reset()
{
	MTNoteLiveBase11::Reset();
}
