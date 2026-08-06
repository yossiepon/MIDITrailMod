//******************************************************************************
//
// MIDITrail / MTGridRing11
//
// DX11 grid ring renderer.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridRing11.h"
#include "DXH.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define RING_SEGMENTS  (128)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTGridRing11::MTGridRing11()
{
	m_isVisible = true;
}

MTGridRing11::~MTGridRing11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTGridRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMBarList barList;
	unsigned long barNum = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long totalTickTime = 0;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	totalTickTime = pSeqData->GetTotalTickTime();

	result = pSeqData->GetBarList(&barList);
	if (result != 0) goto EXIT;

	barNum = barList.GetSize();

	// Vertex: 128 per ring * (bars + end)
	vertexNum = RING_SEGMENTS * (barNum + 1);
	result = m_Primitive.CreateVertexBuffer(pDevice, vertexNum);
	if (result != 0) goto EXIT;

	// Index: 128 segments * 2 (start/end) * (bars + end)
	indexNum = RING_SEGMENTS * 2 * (barNum + 1);
	result = m_Primitive.CreateIndexBuffer(pDevice, indexNum);
	if (result != 0) goto EXIT;

	m_Primitive.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_Primitive.SetLightEnable(false);

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	result = _CreateVertexOfGrid(pVertex, pIndex, totalTickTime, &barList);
	if (result != 0) goto EXIT;

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

	m_Primitive.SetMaterialAmbient(0.5f, 0.5f, 0.5f);

	// Check grid color transparency
	{
		Color lineColor = m_NoteDesign.GetGridLineColor();
		if (lineColor.A() < 0.01f) {
			m_isVisible = false;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update
//******************************************************************************
int MTGridRing11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
	Matrix world = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle))
	             * Matrix::CreateTranslation(moveVec);
	m_Primitive.SetWorldMatrix(world);
	return 0;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTGridRing11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable || !m_isVisible) goto EXIT;

	result = m_Primitive.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTGridRing11::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// Grid vertex creation
//******************************************************************************
int MTGridRing11::_CreateVertexOfGrid(
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long totalTickTime,
		SMBarList* pBarList
	)
{
	int result = 0;
	unsigned long vertexIndex = 0;
	unsigned long tickTime = 0;
	Vector3 basePos;

	unsigned long color = m_NoteDesign.GetGridLineColor().BGRA();

	for (unsigned long i = 0; i < pBarList->GetSize(); i++) {
		result = pBarList->GetBar(i, &tickTime);
		if (result != 0) goto EXIT;

		m_NoteDesign.GetGridRingBasePos(tickTime, &basePos);
		_CreateVertexOfRing(pVertex, &vertexIndex, pIndex, basePos, color);
	}

	// End ring
	m_NoteDesign.GetGridRingBasePos(totalTickTime, &basePos);
	_CreateVertexOfRing(pVertex, &vertexIndex, pIndex, basePos, color);

EXIT:;
	return result;
}

//******************************************************************************
// Ring vertex creation (128-segment circle)
//******************************************************************************
void MTGridRing11::_CreateVertexOfRing(
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long* pVertexIndex,
		unsigned long* pIndex,
		Vector3 basePos,
		unsigned long color
	)
{
	unsigned long startIdx = *pVertexIndex;
	unsigned long vi = *pVertexIndex;
	float nrm[3] = { -1.0f, 0.0f, 0.0f };

	// First vertex
	memcpy(pVertex[vi].pos, &basePos.x, sizeof(float) * 3);
	memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
	pVertex[vi].color = color;
	ZeroMemory(pVertex[vi].uv, sizeof(float) * 2);

	for (unsigned long i = 1; i < RING_SEGMENTS; i++) {
		vi++;
		float angle = (360.0f / (float)RING_SEGMENTS) * (float)i;
		Vector3 rotated = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);

		memcpy(pVertex[vi].pos, &rotated.x, sizeof(float) * 3);
		memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
		pVertex[vi].color = color;
		ZeroMemory(pVertex[vi].uv, sizeof(float) * 2);

		// Line from previous to current
		pIndex[(vi - 1) * 2]     = vi - 1;
		pIndex[(vi - 1) * 2 + 1] = vi;
	}

	// Close the ring
	pIndex[vi * 2]     = vi;
	pIndex[vi * 2 + 1] = startIdx;

	vi++;
	*pVertexIndex = vi;
}
