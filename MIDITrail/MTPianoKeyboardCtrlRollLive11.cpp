//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRollLive11
//
// Piano keyboard controller for PianoRoll (Live).
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTPianoKeyboardCtrlRollLive11.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Compute world matrix (Roll-style, same rotation as Playback Roll)
//******************************************************************************
Matrix MTPianoKeyboardCtrlRollLive11::_ComputeWorldMatrix(
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();

	Vector3 basePos = m_KeyboardDesign.GetKeyboardBasePos(kbdIndex, ctx.rollAngle);

	basePos.x += m_pNoteDesign->GetMaxPitchBendShift(m_pNotePitchBend, 0,
	             GetActiveChannelMask(0));

	float scale = m_KeyboardDesign.GetKeyboardResizeRatio();

	float rollAngle = ctx.rollAngle;
	if (rollAngle < 0.0f) rollAngle += 360.0f;

	Matrix rotateMatrix1, rotateMatrix2, rotateMatrix3;
	if ((rollAngle > 120.0f) && (rollAngle < 300.0f)) {
		rotateMatrix1 = Matrix::CreateRotationX(XM_PI / 2.0f);
		rotateMatrix2 = Matrix::CreateRotationZ(XM_PI / 2.0f);
	}
	else {
		rotateMatrix1 = Matrix::CreateRotationX(-XM_PI / 2.0f);
		rotateMatrix2 = Matrix::CreateRotationZ(XM_PI / 2.0f);
	}
	rotateMatrix3 = Matrix::CreateRotationX(XMConvertToRadians(rollAngle));

	return Matrix::CreateScale(scale)
	     * Matrix::CreateTranslation(basePos)
	     * rotateMatrix1
	     * rotateMatrix2
	     * rotateMatrix3
	     * Matrix::CreateTranslation(moveVec);
}
