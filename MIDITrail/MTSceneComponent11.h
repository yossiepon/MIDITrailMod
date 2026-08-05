//******************************************************************************
//
// MIDITrail / MTSceneComponent11
//
// Common base for DX11 scene components.
// Provides a standard Update interface and shared enable/disable state.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <directxtk/SimpleMath.h>


//******************************************************************************
// Scene update context (passed to all components each frame)
//******************************************************************************
struct MTSceneUpdateContext {
	unsigned long curTickTime;
	unsigned long playTimeMSec;
	float rollAngle;
	DirectX::SimpleMath::Vector3 camPos;
};

//******************************************************************************
// DX11 scene component base
//******************************************************************************
class MTSceneComponent11
{
public:

	virtual ~MTSceneComponent11() = default;

	virtual int Update(const MTSceneUpdateContext& ctx) { return 0; }

	void SetEnable(bool isEnable) { m_isEnable = isEnable; }
	bool IsEnable() const { return m_isEnable; }

protected:

	bool m_isEnable = true;
};
