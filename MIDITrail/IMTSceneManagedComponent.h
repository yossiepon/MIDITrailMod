//******************************************************************************
//
// MIDITrail / IMTSceneManagedComponent
//
// Registration interface for scene-managed objects.
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneUpdateContext.h"


//******************************************************************************
// Scene-managed component interface
//******************************************************************************
class IMTSceneManagedComponent
{
public:

	virtual ~IMTSceneManagedComponent() = default;

	virtual int  Update(const MTSceneUpdateContext& ctx) { return 0; }
	virtual void Reset() {}
};
