//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRollLive11
//
// Roll Live keyboard controller (DX11).
// Overrides world matrix for Roll scene positioning.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrlFlatLive11.h"


//******************************************************************************
// Roll Live keyboard controller
//******************************************************************************
class MTPianoKeyboardCtrlRollLive11 : public MTPianoKeyboardCtrlFlatLive11
{
protected:

	DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				const MTSceneUpdateContext& ctx) override;
};
