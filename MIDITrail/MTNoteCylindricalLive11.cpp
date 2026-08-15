//******************************************************************************
//
// MIDITrail / MTNoteCylindricalLive11
//
// Cylindrical note renderer (Live).
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTNoteCylindricalLive11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define NOTE_VERTEX_NUM  (4)
#define NOTE_INDEX_NUM   (6)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteCylindricalLive11::MTNoteCylindricalLive11()
{
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
	m_NoteVertexNum = NOTE_VERTEX_NUM;
	m_NoteIndexNum = NOTE_INDEX_NUM;

	result = m_NoteDesignLocal.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	m_pNoteDesign = &m_NoteDesignLocal;

	m_LiveMonitorDisplayDuration = m_NoteDesignLocal.GetLiveMonitorDisplayDuration();

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
// Compute world matrix
//******************************************************************************
Matrix MTNoteCylindricalLive11::_ComputeWorldMatrix(const MTSceneUpdateContext& ctx)
{
	Vector3 moveVec = m_NoteDesignLocal.GetWorldMoveVector();
	return Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle))
	     * Matrix::CreateTranslation(moveVec);
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

	unsigned long startElapsed = curTimeMs - note.startTime;
	if (startElapsed > m_LiveMonitorDisplayDuration) {
		startElapsed = m_LiveMonitorDisplayDuration;
	}
	unsigned long endElapsed = 0;
	if (note.endTime != 0) {
		endElapsed = curTimeMs - note.endTime;
	}

	m_NoteDesignLocal.GetNoteBoxVirtexPosLive(
		startElapsed, note.portNo, note.chNo, note.noteNo,
		&vectorStartLU, &vectorStartRU, &vectorStartLD, &vectorStartRD,
		pbValue, pbSensitivity);

	m_NoteDesignLocal.GetNoteBoxVirtexPosLive(
		endElapsed, note.portNo, note.chNo, note.noteNo,
		&vectorEndLU, &vectorEndRU, &vectorEndLD, &vectorEndRD,
		pbValue, pbSensitivity);

	unsigned long c;
	if (note.endTime != 0) {
		c = m_NoteDesignLocal.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo).BGRA();
	}
	else {
		unsigned long elapsedTime = curTimeMs - note.startTime;
		float rate = 0.0f;
		unsigned long decayDuration = m_NoteDesignLocal.GetRippleDecayDuration();
		if (elapsedTime < decayDuration) {
			rate = 1.0f - ((float)elapsedTime / (float)decayDuration);
		}
		c = m_NoteDesignLocal.GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, rate).BGRA();
	}

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
