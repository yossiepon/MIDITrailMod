//******************************************************************************
//
// MIDITrail / MTGridBox11Base
//
// Grid box base class (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTGridBox11Base.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTGridBox11Base::MTGridBox11Base()
{
	m_isReady = false;
}

MTGridBox11Base::~MTGridBox11Base()
{
	Release();
}

//******************************************************************************
// Release
//******************************************************************************
void MTGridBox11Base::Release()
{
	m_Primitive.Release();
	m_isReady = false;
}

//******************************************************************************
// Setup primitive state (called by derived Create)
//******************************************************************************
void MTGridBox11Base::_SetupPrimitive()
{
	m_Primitive.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_Primitive.SetLightEnable(false);
	m_Primitive.SetDepthWrite(false);
}

//******************************************************************************
// Update
//******************************************************************************
int MTGridBox11Base::Update(const MTSceneUpdateContext& ctx)
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
int MTGridBox11Base::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir,
		float rollAngle
	)
{
	if (!m_isEnable || !m_isReady) return 0;
	return m_Primitive.Draw(pContext, viewProj, lightDir);
}
