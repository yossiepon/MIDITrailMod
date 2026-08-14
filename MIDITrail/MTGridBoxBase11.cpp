//******************************************************************************
//
// MIDITrail / MTGridBoxBase11
//
// Grid box base class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTGridBoxBase11.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTGridBoxBase11::MTGridBoxBase11()
{
	m_isReady = false;
	m_isVisible = true;
}

MTGridBoxBase11::~MTGridBoxBase11()
{
	Release();
}

//******************************************************************************
// Release
//******************************************************************************
void MTGridBoxBase11::Release()
{
	m_Primitive.Release();
	m_isReady = false;
}

//******************************************************************************
// Setup primitive state (called by derived Create)
//******************************************************************************
void MTGridBoxBase11::_SetupPrimitive()
{
	m_Primitive.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_Primitive.SetLightEnable(false);
	m_Primitive.SetDepthWrite(false);
}

//******************************************************************************
// Update
//******************************************************************************
int MTGridBoxBase11::Update(const MTSceneUpdateContext& ctx)
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
int MTGridBoxBase11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	if (!m_isEnable || !m_isReady || !m_isVisible) return 0;
	return m_Primitive.Draw(pContext, viewProj, lightDir);
}
