//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRainLive11
//
// Rain Live keyboard controller (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrlFlatLive11.h"


//******************************************************************************
// Rain Live keyboard controller
//******************************************************************************
class MTPianoKeyboardCtrlRainLive11 : public MTPianoKeyboardCtrlFlatLive11
{
protected:

	DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				unsigned long kbdIndex,
				const MTSceneUpdateContext& ctx) override;
};
