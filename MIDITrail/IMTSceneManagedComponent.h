//******************************************************************************
//
// MIDITrail / IMTSceneManagedComponent
//
// Registration interface for scene-managed objects.
// Any object that implements this interface can be registered with
// MTSceneBase11 for automatic Update and Reset dispatch.
// Visual components (MTSceneComponent11) add enable/disable on top of this.
// Data managers (NoteTracker, NotePitchBend) implement this directly.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
