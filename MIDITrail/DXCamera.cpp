//******************************************************************************
//
// MIDITrail / DXCamera
//
// Camera class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXCamera.h"
#include "YNBaseLib.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
DXCamera::DXCamera()
{
	_Clear();
}

//******************************************************************************
// Destructor
//******************************************************************************
DXCamera::~DXCamera()
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int DXCamera::Initialize()
{
	_Clear();
	return 0;
}

//******************************************************************************
// Set base parameters
//******************************************************************************
void DXCamera::SetBaseParam(
		float viewAngle,
		float nearPlane,
		float farPlane
	)
{
	m_ViewAngle = viewAngle;
	m_NearPlane = nearPlane;
	m_FarPlane  = farPlane;
}

//******************************************************************************
// Set camera position
//******************************************************************************
void DXCamera::SetPosition(
		Vector3 camVector,
		Vector3 camLookAtVector,
		Vector3 camUpVector
	)
{
	m_CamVector       = camVector;
	m_CamLookAtVector = camLookAtVector;
	m_CamUpVector     = camUpVector;
}

//******************************************************************************
// Get view and projection matrices
//******************************************************************************
int DXCamera::GetMatrices(
		float aspect,
		Matrix* pView,
		Matrix* pProj
	)
{
	if (pView == nullptr || pProj == nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}

	// Projection matrix (left-handed perspective)
	float fovRad = XMConvertToRadians(m_ViewAngle);
	*pProj = XMMatrixPerspectiveFovLH(fovRad, aspect, m_NearPlane, m_FarPlane);

	// View matrix (left-handed look-at)
	XMVECTOR eye    = XMLoadFloat3(&m_CamVector);
	XMVECTOR lookAt = XMLoadFloat3(&m_CamLookAtVector);
	XMVECTOR up     = XMLoadFloat3(&m_CamUpVector);
	*pView = XMMatrixLookAtLH(eye, lookAt, up);

	return 0;
}

//******************************************************************************
// Clear
//******************************************************************************
void DXCamera::_Clear()
{
	m_ViewAngle = 45.0f;
	m_NearPlane = 1.0f;
	m_FarPlane  = 1000.0f;
	m_CamVector       = Vector3(0.0f, 0.0f, 0.0f);
	m_CamLookAtVector = Vector3(0.0f, 0.0f, 1.0f);
	m_CamUpVector     = Vector3(0.0f, 1.0f, 0.0f);
}
