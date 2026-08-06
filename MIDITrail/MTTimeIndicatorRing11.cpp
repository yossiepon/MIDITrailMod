//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRing11
//
// DX11 time indicator ring renderer.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTTimeIndicatorRing11.h"
#include "DXH.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define RING_SEGMENTS  (128)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTTimeIndicatorRing11::MTTimeIndicatorRing11()
{
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
}

MTTimeIndicatorRing11::~MTTimeIndicatorRing11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTTimeIndicatorRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = _CreateVertexOfRing(pDevice, pContext);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create ring vertices
//******************************************************************************
int MTTimeIndicatorRing11::_CreateVertexOfRing(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	result = m_Primitive.CreateVertexBuffer(pDevice, RING_SEGMENTS);
	if (result != 0) goto EXIT;

	result = m_Primitive.CreateIndexBuffer(pDevice, RING_SEGMENTS * 2);
	if (result != 0) goto EXIT;

	m_Primitive.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_Primitive.SetLightEnable(false);

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	{
		Vector3 basePos;
		m_NoteDesign.GetGridRingBasePos(0, &basePos);

		unsigned long color = m_NoteDesign.GetGridLineColor().BGRA();
		float nrm[3] = { -1.0f, 0.0f, 0.0f };

		memcpy(pVertex[0].pos, &basePos.x, sizeof(float) * 3);
		memcpy(pVertex[0].normal, nrm, sizeof(float) * 3);
		pVertex[0].color = color;
		ZeroMemory(pVertex[0].uv, sizeof(float) * 2);

		for (unsigned long i = 1; i < RING_SEGMENTS; i++) {
			float angle = (360.0f / (float)RING_SEGMENTS) * (float)i;
			Vector3 rotated = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);

			memcpy(pVertex[i].pos, &rotated.x, sizeof(float) * 3);
			memcpy(pVertex[i].normal, nrm, sizeof(float) * 3);
			pVertex[i].color = color;
			ZeroMemory(pVertex[i].uv, sizeof(float) * 2);

			pIndex[(i - 1) * 2]     = i - 1;
			pIndex[(i - 1) * 2 + 1] = i;
		}

		// Close the ring
		pIndex[(RING_SEGMENTS - 1) * 2]     = RING_SEGMENTS - 1;
		pIndex[(RING_SEGMENTS - 1) * 2 + 1] = 0;
	}

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

	m_Primitive.SetMaterialAmbient(0.5f, 0.5f, 0.5f);

EXIT:;
	return result;
}

//******************************************************************************
// Update
//******************************************************************************
int MTTimeIndicatorRing11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	m_CurTickTime = ctx.curTickTime;
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
	Matrix world = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle))
	             * Matrix::CreateTranslation(moveVec.x + m_CurPos, moveVec.y, moveVec.z);
	m_Primitive.SetWorldMatrix(world);

	return 0;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTTimeIndicatorRing11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	result = m_Primitive.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTTimeIndicatorRing11::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// Reset
//******************************************************************************
void MTTimeIndicatorRing11::Reset()
{
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
}
