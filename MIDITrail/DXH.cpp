//******************************************************************************
//
// MIDITrail / DXH
//
// Helper function class.
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXH.h"
#include <cmath>

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Rotate point on YZ plane
//******************************************************************************
Vector3 DXH::RotateYZ(
		float centerY,
		float centerZ,
		Vector3 p1,
		float angle
	)
{
	float rad = XMConvertToRadians(angle);
	Vector3 p2;
	p2.x = p1.x;
	p2.y = centerY + sinf(rad) * (p1.z - centerZ) + cosf(rad) * (p1.y - centerY);
	p2.z = centerZ + cosf(rad) * (p1.z - centerZ) - sinf(rad) * (p1.y - centerY);
	return p2;
}
