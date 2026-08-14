//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRollLive11
//
// Piano keyboard controller for PianoRoll (Live).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
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
				unsigned long kbdIndex,
				const MTSceneUpdateContext& ctx) override;
};
