//******************************************************************************
//
// MIDITrail / MTPictBoardRing11
//
// DX11 picture board ring renderer.
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPictBoardRing11.h"
#include "DXH.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPictBoardRing11::MTPictBoardRing11()
{
	m_pSRV = NULL;
	m_CurTickTime = 0;
	m_ImgWidth = 0;
	m_ImgHeight = 0;
}

MTPictBoardRing11::~MTPictBoardRing11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTPictBoardRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	Release();

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = _LoadTexture(pDevice, pSceneName);
	if (result != 0) goto EXIT;

	// (SM_MAX_NOTE_NUM + 1) * 2 vertices for cylinder
	vertexNum = (SM_MAX_NOTE_NUM + 1) * 2;
	result = m_Primitive.CreateVertexBuffer(pDevice, vertexNum);
	if (result != 0) goto EXIT;

	// SM_MAX_NOTE_NUM * 6 indices (2 triangles per segment)
	indexNum = SM_MAX_NOTE_NUM * 6;
	result = m_Primitive.CreateIndexBuffer(pDevice, indexNum);
	if (result != 0) goto EXIT;

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	result = _CreateVertexOfBoard(pVertex, pIndex);
	if (result != 0) goto EXIT;

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

	m_Primitive.SetMaterialAmbient(0.5f, 0.5f, 0.5f);

EXIT:;
	return result;
}

//******************************************************************************
// Update
//******************************************************************************
int MTPictBoardRing11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	m_CurTickTime = ctx.curTickTime;
	float curPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
	Matrix world = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle))
	             * Matrix::CreateTranslation(moveVec.x + curPos, moveVec.y, moveVec.z);
	m_Primitive.SetWorldMatrix(world);

	return 0;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTPictBoardRing11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	m_Primitive.SetTexture(m_pSRV);
	result = m_Primitive.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTPictBoardRing11::Release()
{
	m_Primitive.Release();

	if (m_pSRV != NULL) {
		m_pSRV->Release();
		m_pSRV = NULL;
	}
}

//******************************************************************************
// Reset
//******************************************************************************
void MTPictBoardRing11::Reset()
{
	m_CurTickTime = 0;
}

//******************************************************************************
// Board vertex creation (cylinder around X-axis)
//******************************************************************************
int MTPictBoardRing11::_CreateVertexOfBoard(
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	float chStep = m_NoteDesign.GetChStep();

	Vector3 basePos(
		m_NoteDesign.GetPlayPosX(0),
		m_NoteDesign.GetPortOriginY(0) + (chStep * (float)SM_MAX_CH_NUM) + chStep + 0.01f,
		m_NoteDesign.GetPortOriginZ(0)
	);

	float boardHeight = 2.0f * 3.1415926f * basePos.y;
	float boardWidth = boardHeight * ((float)m_ImgWidth / (float)m_ImgHeight);
	basePos.x -= (boardWidth * m_NoteDesign.GetPictBoardRelativePos());

	float nrm[3] = { -1.0f, 0.0f, 0.0f };
	unsigned long color = 0xFFFFFFFF;
	unsigned long vi = 0;

	// First pair of vertices (angle = 0)
	{
		float p0[3] = { basePos.x, basePos.y, basePos.z };
		float p1[3] = { basePos.x + boardWidth, basePos.y, basePos.z };
		float uv0[2] = { 1.0f, 0.0f };
		float uv1[2] = { 0.0f, 0.0f };

		memcpy(pVertex[vi].pos, p0, sizeof(float) * 3);
		memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
		pVertex[vi].color = color;
		memcpy(pVertex[vi].uv, uv0, sizeof(float) * 2);
		vi++;
		memcpy(pVertex[vi].pos, p1, sizeof(float) * 3);
		memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
		pVertex[vi].color = color;
		memcpy(pVertex[vi].uv, uv1, sizeof(float) * 2);
	}

	for (unsigned long i = 1; i < SM_MAX_NOTE_NUM; i++) {
		vi++;
		float angle = (360.0f / (float)SM_MAX_NOTE_NUM) * (float)i;
		Vector3 rotated = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);

		float p0[3] = { rotated.x, rotated.y, rotated.z };
		float p1[3] = { rotated.x + boardWidth, rotated.y, rotated.z };
		float v = (float)i / (float)SM_MAX_NOTE_NUM;
		float uv0[2] = { 1.0f, v };
		float uv1[2] = { 0.0f, v };

		memcpy(pVertex[vi].pos, p0, sizeof(float) * 3);
		memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
		pVertex[vi].color = color;
		memcpy(pVertex[vi].uv, uv0, sizeof(float) * 2);
		vi++;
		memcpy(pVertex[vi].pos, p1, sizeof(float) * 3);
		memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
		pVertex[vi].color = color;
		memcpy(pVertex[vi].uv, uv1, sizeof(float) * 2);

		// Two triangles per segment: 0-1-3, 0-3-2
		pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;
		pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;
		pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;
		pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;
		pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;
		pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;
	}

	// Close the ring: duplicate first vertices with UV at v=1.0
	vi++;
	pVertex[vi] = pVertex[0];
	pVertex[vi].uv[1] = 1.0f;
	vi++;
	pVertex[vi] = pVertex[1];
	pVertex[vi].uv[1] = 1.0f;

	// Final segment indices
	unsigned long i = SM_MAX_NOTE_NUM;
	pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;
	pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;
	pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;
	pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;
	pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;
	pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;

	return result;
}

//******************************************************************************
// Load texture
//******************************************************************************
int MTPictBoardRing11::_LoadTexture(
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
	result = confFile.GetStr(_T("Board"), bmpFileName, _MAX_PATH, MT_IMGFILE_BOARD);
	if (result != 0) goto EXIT;

	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	result = DXTexture11::LoadFromFile(pDevice, imgFilePath, &m_pSRV, &m_ImgWidth, &m_ImgHeight);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}
