//******************************************************************************
//
// MIDITrail / MTSceneUpdateContext
//
// Scene update context structure.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
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
