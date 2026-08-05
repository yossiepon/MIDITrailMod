//******************************************************************************
//
// MIDITrail / DXCamera
//
// Camera class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <directxtk/SimpleMath.h>


//******************************************************************************
// Camera class
//******************************************************************************
class DXCamera
{
public:

	DXCamera();
	virtual ~DXCamera();

	int Initialize();

	void SetBaseParam(
			float viewAngle,
			float nearPlane,
			float farPlane
		);

	void SetPosition(
			DirectX::SimpleMath::Vector3 camVector,
			DirectX::SimpleMath::Vector3 camLookAtVector,
			DirectX::SimpleMath::Vector3 camUpVector
		);

	// Returns view and projection matrices for the given aspect ratio.
	int GetMatrices(
			float aspect,
			DirectX::SimpleMath::Matrix* pView,
			DirectX::SimpleMath::Matrix* pProj
		);

private:

	float m_ViewAngle;
	float m_NearPlane;
	float m_FarPlane;

	DirectX::SimpleMath::Vector3 m_CamVector;
	DirectX::SimpleMath::Vector3 m_CamLookAtVector;
	DirectX::SimpleMath::Vector3 m_CamUpVector;

	void _Clear();
};
