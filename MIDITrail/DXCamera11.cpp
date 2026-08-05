//******************************************************************************
//
// MIDITrail / DXCamera11
//
// DX11 fixed camera.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXCamera11.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
DXCamera11::DXCamera11()
{
	m_ViewAngle = 45.0f;
	m_NearPlane = 1.0f;
	m_FarPlane = 1000.0f;
	m_Eye = Vector3(0.0f, 0.0f, 0.0f);
	m_LookAt = Vector3(0.0f, 0.0f, 1.0f);
	m_Up = Vector3(0.0f, 1.0f, 0.0f);
}

DXCamera11::~DXCamera11()
{
}

//******************************************************************************
// Base parameters
//******************************************************************************
void DXCamera11::SetBaseParam(
		float viewAngle,
		float nearPlane,
		float farPlane
	)
{
	m_ViewAngle = viewAngle;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
}

//******************************************************************************
// Position
//******************************************************************************
void DXCamera11::SetPosition(
		Vector3 eye,
		Vector3 lookAt,
		Vector3 up
	)
{
	m_Eye = eye;
	m_LookAt = lookAt;
	m_Up = up;
}

//******************************************************************************
// View / Projection
//******************************************************************************
void DXCamera11::GetViewProjection(
		float aspect,
		Matrix* pView,
		Matrix* pProj
	)
{
	float fovRad = XMConvertToRadians(m_ViewAngle);
	XMVECTOR eye    = XMLoadFloat3(&m_Eye);
	XMVECTOR lookAt = XMLoadFloat3(&m_LookAt);
	XMVECTOR up     = XMLoadFloat3(&m_Up);
	*pView = XMMatrixLookAtLH(eye, lookAt, up);
	*pProj = XMMatrixPerspectiveFovLH(fovRad, aspect, m_NearPlane, m_FarPlane);
}
