//******************************************************************************
//
// MIDITrail / DXH
//
// Helper function class.
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <directxtk/SimpleMath.h>


//******************************************************************************
// Helper function class
//******************************************************************************
class DXH
{
public:

	// Rotate a point around the YZ plane
	static DirectX::SimpleMath::Vector3 RotateYZ(
			float centerY,
			float centerZ,
			DirectX::SimpleMath::Vector3 p1,
			float angle
		);
};
