//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRollLive11
//
// Roll Live keyboard controller (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTPianoKeyboardCtrlRollLive11.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Compute world matrix (Roll-style)
//******************************************************************************
Matrix MTPianoKeyboardCtrlRollLive11::_ComputeWorldMatrix(
		const MTSceneUpdateContext& ctx
	)
{
	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();

	Vector3 basePos = m_KeyboardDesign.GetKeyboardBasePos(0, ctx.rollAngle);
	float scale = m_KeyboardDesign.GetKeyboardResizeRatio();

	float rollAngle = ctx.rollAngle;
	if (rollAngle < 0.0f) rollAngle += 360.0f;
	float effectiveRoll = XMConvertToRadians(rollAngle);
	if ((rollAngle > 120.0f) && (rollAngle < 300.0f)) {
		effectiveRoll += XM_PI;
	}

	return Matrix::CreateScale(scale)
	     * Matrix::CreateTranslation(basePos)
	     * Matrix::CreateRotationX(effectiveRoll)
	     * Matrix::CreateTranslation(moveVec);
}
