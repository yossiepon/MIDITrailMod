//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRainLive11
//
// Piano keyboard controller for Rain (Live).
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
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
