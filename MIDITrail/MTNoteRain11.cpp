//******************************************************************************
//
// MIDITrail / MTNoteRain11
//
// DX11 note rain renderer.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteRain11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define NOTE_VERTEX_NUM  (4)
#define NOTE_INDEX_NUM   (6)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteRain11::MTNoteRain11()
{
	m_pCpuVertex = NULL;
	m_VertexCount = 0;
	m_pContext = NULL;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_pNoteStatus = NULL;
	m_CurPos = 0.0f;
	m_isSkipping = false;
	m_pNotePitchBend = NULL;
}

MTNoteRain11::~MTNoteRain11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteRain11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;
	SMTrack track;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_pContext = pContext;
	m_pNotePitchBend = pNotePitchBend;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	result = track.GetNoteList(&m_NoteList);
	if (result != 0) goto EXIT;

	result = _CreateAllNoteRain(pDevice, pContext);
	if (result != 0) goto EXIT;

	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create all note rain geometry
//******************************************************************************
int MTNoteRain11::_CreateAllNoteRain(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	unsigned long noteCount = m_NoteList.GetSize();
	SMNote note;

	m_VertexCount = NOTE_VERTEX_NUM * noteCount;
	unsigned long indexCount = NOTE_INDEX_NUM * noteCount;

	if (noteCount == 0) goto EXIT;

	result = m_Primitive.CreateVertexBuffer(pDevice, m_VertexCount);
	if (result != 0) goto EXIT;

	result = m_Primitive.CreateIndexBuffer(pDevice, indexCount);
	if (result != 0) goto EXIT;

	try {
		m_pCpuVertex = new DXPRIMITIVE11_VERTEX[m_VertexCount];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	ZeroMemory(m_pCpuVertex, m_VertexCount * sizeof(DXPRIMITIVE11_VERTEX));

	for (unsigned long i = 0; i < noteCount; i++) {
		result = m_NoteList.GetNote(i, &note);
		if (result != 0) goto EXIT;
		_CreateVertexOfNote(note, i);
	}

	// Index buffer
	{
		unsigned long* pIndex = NULL;
		result = m_Primitive.LockIndex(pContext, &pIndex);
		if (result != 0) goto EXIT;

		for (unsigned long i = 0; i < noteCount; i++) {
			unsigned long vo = NOTE_VERTEX_NUM * i;
			pIndex[NOTE_INDEX_NUM * i + 0] = vo + 0;
			pIndex[NOTE_INDEX_NUM * i + 1] = vo + 2;
			pIndex[NOTE_INDEX_NUM * i + 2] = vo + 1;
			pIndex[NOTE_INDEX_NUM * i + 3] = vo + 0;
			pIndex[NOTE_INDEX_NUM * i + 4] = vo + 3;
			pIndex[NOTE_INDEX_NUM * i + 5] = vo + 2;
		}

		m_Primitive.UnlockIndex(pContext);
	}

	// Flush initial vertices to GPU
	result = _FlushToGPU();
	if (result != 0) goto EXIT;

	m_Primitive.SetMaterialAmbient(0.5f, 0.5f, 0.5f);

EXIT:;
	return result;
}

//******************************************************************************
// Create vertex of one note
//******************************************************************************
void MTNoteRain11::_CreateVertexOfNote(
		SMNote note,
		unsigned long noteIndex
	)
{
	DXPRIMITIVE11_VERTEX* pV = &m_pCpuVertex[NOTE_VERTEX_NUM * noteIndex];

	float startY = m_NoteDesign.GetPlayPosX(note.startTime);
	float endY   = m_NoteDesign.GetPlayPosX(note.endTime);

	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
	float posX = moveVec.x + m_KeyboardDesign.GetKeyCenterPosX(note.noteNo);
	float posY = moveVec.y + m_KeyboardDesign.GetWhiteKeyHeight() / 2.0f;
	float posZ = moveVec.z + m_KeyboardDesign.GetNoteDropPosZ(note.noteNo);

	float rainWidth = m_KeyboardDesign.GetBlackKeyWidth();

	float sx = posX - rainWidth / 2.0f;
	float ex = posX + rainWidth / 2.0f;

	// Positions
	float p0[3] = { sx, startY + posY, posZ };
	float p1[3] = { ex, startY + posY, posZ };
	float p2[3] = { ex, endY   + posY, posZ };
	float p3[3] = { sx, endY   + posY, posZ };

	memcpy(pV[0].pos, p0, sizeof(float) * 3);
	memcpy(pV[1].pos, p1, sizeof(float) * 3);
	memcpy(pV[2].pos, p2, sizeof(float) * 3);
	memcpy(pV[3].pos, p3, sizeof(float) * 3);

	// Normals (facing up to match keyboard surface for lighting)
	float nrm[3] = { 0.0f, 1.0f, 0.0f };
	for (int i = 0; i < 4; i++) {
		memcpy(pV[i].normal, nrm, sizeof(float) * 3);
	}

	// Color: note-ON end = opaque, note-OFF end = semi-transparent
	Color color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	pV[0].color = Color(color.R(), color.G(), color.B(), 1.0f).BGRA();
	pV[1].color = Color(color.R(), color.G(), color.B(), 1.0f).BGRA();
	pV[2].color = Color(color.R(), color.G(), color.B(), 0.5f).BGRA();
	pV[3].color = Color(color.R(), color.G(), color.B(), 0.5f).BGRA();

	// No texture
	ZeroMemory(pV[0].uv, sizeof(float) * 2);
	ZeroMemory(pV[1].uv, sizeof(float) * 2);
	ZeroMemory(pV[2].uv, sizeof(float) * 2);
	ZeroMemory(pV[3].uv, sizeof(float) * 2);
}

//******************************************************************************
// Create note status array
//******************************************************************************
int MTNoteRain11::_CreateNoteStatus()
{
	int result = 0;

	try {
		m_pNoteStatus = new NoteStatus[MTNOTERAIN_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	for (unsigned long i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].index = 0;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update
//******************************************************************************
int MTNoteRain11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	int result = 0;
	bool needFlush = false;

	m_CurTickTime = ctx.curTickTime;
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	if (!m_isSkipping && m_pNoteStatus != NULL) {
		unsigned long prevActiveCount = 0;
		for (unsigned long i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
			if (m_pNoteStatus[i].isActive) prevActiveCount++;
		}

		_UpdateStatusOfActiveNotes();

		for (unsigned long i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
			if (m_pNoteStatus[i].isActive) {
				_UpdateVertexOfNote(m_pNoteStatus[i].index, true);
				needFlush = true;
			}
		}

		if (prevActiveCount > 0 || needFlush) {
			result = _FlushToGPU();
			if (result != 0) goto EXIT;
		}
	}

	// World matrix: translate by playback position, rotate by roll angle
	Matrix world = Matrix::CreateTranslation(0.0f, -m_CurPos, 0.0f)
	             * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
	m_Primitive.SetWorldMatrix(world);

EXIT:;
	return result;
}

//******************************************************************************
// Update active note status
//******************************************************************************
void MTNoteRain11::_UpdateStatusOfActiveNotes()
{
	SMNote note;

	// Expire finished notes
	for (unsigned long i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
		if (!m_pNoteStatus[i].isActive) continue;

		if (m_NoteList.GetNote(m_pNoteStatus[i].index, &note) != 0) continue;

		if (note.endTime < m_CurTickTime) {
			_UpdateVertexOfNote(m_pNoteStatus[i].index);
			m_pNoteStatus[i].isActive = false;
			m_pNoteStatus[i].index = 0;
		}
	}

	// Register new active notes (forward scan)
	while (m_CurNoteIndex < m_NoteList.GetSize()) {
		if (m_NoteList.GetNote(m_CurNoteIndex, &note) != 0) break;

		if (m_CurTickTime < note.startTime) break;

		if ((note.startTime <= m_CurTickTime) && (m_CurTickTime <= note.endTime)) {
			bool isFound = false;
			for (unsigned long i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
				if (m_pNoteStatus[i].isActive && m_pNoteStatus[i].index == m_CurNoteIndex) {
					isFound = true;
					break;
				}
			}
			if (!isFound) {
				for (unsigned long i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
					if (!m_pNoteStatus[i].isActive) {
						m_pNoteStatus[i].isActive = true;
						m_pNoteStatus[i].index = m_CurNoteIndex;
						break;
					}
				}
			}
		}
		m_CurNoteIndex++;
	}
}

//******************************************************************************
// Update vertex of note (pitch bend shift)
//******************************************************************************
void MTNoteRain11::_UpdateVertexOfNote(
		unsigned long index,
		bool isEnablePitchBendShift
	)
{
	SMNote note;
	if (m_NoteList.GetNote(index, &note) != 0) return;

	float pitchBendShift = 0.0f;
	if (isEnablePitchBendShift && m_pNotePitchBend != NULL) {
		short pbValue = m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		unsigned char pbSens = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
		pitchBendShift = m_KeyboardDesign.GetPitchBendShift(pbValue, pbSens);
	}

	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
	float posX = moveVec.x + m_KeyboardDesign.GetKeyCenterPosX(note.noteNo) + pitchBendShift;
	float rainWidth = m_KeyboardDesign.GetBlackKeyWidth();

	DXPRIMITIVE11_VERTEX* pV = &m_pCpuVertex[NOTE_VERTEX_NUM * index];
	pV[0].pos[0] = posX - rainWidth / 2.0f;
	pV[1].pos[0] = posX + rainWidth / 2.0f;
	pV[2].pos[0] = posX + rainWidth / 2.0f;
	pV[3].pos[0] = posX - rainWidth / 2.0f;
}

//******************************************************************************
// Flush CPU vertex buffer to GPU
//******************************************************************************
int MTNoteRain11::_FlushToGPU()
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;

	if (m_pContext == NULL || m_pCpuVertex == NULL) goto EXIT;

	result = m_Primitive.LockVertex(m_pContext, &pVertex);
	if (result != 0) goto EXIT;

	memcpy(pVertex, m_pCpuVertex, m_VertexCount * sizeof(DXPRIMITIVE11_VERTEX));

	m_Primitive.UnlockVertex(m_pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteRain11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;
	if (m_VertexCount == 0) goto EXIT;

	m_Primitive.SetDepthWrite(true);

	result = m_Primitive.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteRain11::Release()
{
	m_Primitive.Release();
	m_NoteList.Clear();

	delete[] m_pCpuVertex;
	m_pCpuVertex = NULL;
	m_VertexCount = 0;

	delete[] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// Reset
//******************************************************************************
unsigned long MTNoteRain11::GetNoteCount() const
{
	return const_cast<SMNoteList&>(m_NoteList).GetSize();
}

void MTNoteRain11::Reset()
{
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_CurPos = 0.0f;

	if (m_pNoteStatus != NULL) {
		for (unsigned long i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
			if (m_pNoteStatus[i].isActive) {
				_UpdateVertexOfNote(m_pNoteStatus[i].index);
			}
			m_pNoteStatus[i].isActive = false;
			m_pNoteStatus[i].index = 0;
		}
	}

	if (m_pCpuVertex != NULL) {
		_FlushToGPU();
	}
}
