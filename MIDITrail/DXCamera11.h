//******************************************************************************
//
// MIDITrail / DXCamera11
//
// DX11 fixed camera.
// Computes view/projection matrices from eye/lookAt/up parameters.
// DX11 port of DXCamera (DX9 fixed-function pipeline removed).
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <directxtk/SimpleMath.h>


//******************************************************************************
// DX11 fixed camera
//******************************************************************************
class DXCamera11
{
public:

	DXCamera11();
	~DXCamera11();

	void SetBaseParam(float viewAngle, float nearPlane, float farPlane);

	void SetPosition(
			DirectX::SimpleMath::Vector3 eye,
			DirectX::SimpleMath::Vector3 lookAt,
			DirectX::SimpleMath::Vector3 up
		);

	void GetViewProjection(
			float aspect,
			DirectX::SimpleMath::Matrix* pView,
			DirectX::SimpleMath::Matrix* pProj
		);

private:

	float m_ViewAngle;
	float m_NearPlane;
	float m_FarPlane;
	DirectX::SimpleMath::Vector3 m_Eye;
	DirectX::SimpleMath::Vector3 m_LookAt;
	DirectX::SimpleMath::Vector3 m_Up;
};
