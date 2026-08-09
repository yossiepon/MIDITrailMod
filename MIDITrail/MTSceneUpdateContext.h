//******************************************************************************
//
// MIDITrail / MTSceneUpdateContext
//
// Per-frame context passed to all scene-managed components during Update.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <directxtk/SimpleMath.h>


//******************************************************************************
// Scene update context
//******************************************************************************
struct MTSceneUpdateContext {
	unsigned long curTickTime;
	unsigned long playTimeMSec;
	float rollAngle;
	DirectX::SimpleMath::Vector3 camPos;
	unsigned long liveTimeMSec;
};
