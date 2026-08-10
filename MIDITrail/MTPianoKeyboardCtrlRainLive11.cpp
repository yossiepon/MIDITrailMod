//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRainLive11
//
// Rain Live keyboard controller (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTPianoKeyboardCtrlRainLive11.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Compute world matrix (Rain-style: translation + Y rotation)
//******************************************************************************
Matrix MTPianoKeyboardCtrlRainLive11::_ComputeWorldMatrix(
		const MTSceneUpdateContext& ctx
	)
{
	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(0, 0);
	return Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle))
	     * Matrix::CreateTranslation(moveVec);
}
