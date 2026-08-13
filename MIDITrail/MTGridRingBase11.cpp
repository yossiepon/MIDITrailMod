//******************************************************************************
//
// MIDITrail / MTGridRingBase11
//
// Grid ring base class.
//
// Copyright (C) 2019-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTGridRingBase11.h"
#include "DXH.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTGridRingBase11::MTGridRingBase11()
{
	m_isVisible = true;
}

MTGridRingBase11::~MTGridRingBase11()
{
	Release();
}

//******************************************************************************
// Release
//******************************************************************************
void MTGridRingBase11::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// Setup primitive state
//******************************************************************************
void MTGridRingBase11::_SetupPrimitive()
{
	m_Primitive.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_Primitive.SetLightEnable(false);
}

//******************************************************************************
// Update
//******************************************************************************
int MTGridRingBase11::Update(const MTSceneUpdateContext& ctx)
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
int MTGridRingBase11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	if (!m_isEnable || !m_isVisible) return 0;
	return m_Primitive.Draw(pContext, viewProj, lightDir);
}

//******************************************************************************
// Ring vertex creation (128-segment circle)
//******************************************************************************
void MTGridRingBase11::_CreateVertexOfRing(
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

	memcpy(pVertex[vi].pos, &basePos.x, sizeof(float) * 3);
	memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
	pVertex[vi].color = color;
	ZeroMemory(pVertex[vi].uv, sizeof(float) * 2);

	for (unsigned long i = 1; i < GRID_RING_SEGMENTS; i++) {
		vi++;
		float angle = (360.0f / (float)GRID_RING_SEGMENTS) * (float)i;
		Vector3 rotated = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);

		memcpy(pVertex[vi].pos, &rotated.x, sizeof(float) * 3);
		memcpy(pVertex[vi].normal, nrm, sizeof(float) * 3);
		pVertex[vi].color = color;
		ZeroMemory(pVertex[vi].uv, sizeof(float) * 2);

		pIndex[(vi - 1) * 2]     = vi - 1;
		pIndex[(vi - 1) * 2 + 1] = vi;
	}

	pIndex[vi * 2]     = vi;
	pIndex[vi * 2 + 1] = startIdx;

	vi++;
	*pVertexIndex = vi;
}
