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
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	unsigned char portNo = 0;
	unsigned char chNo = (unsigned char)kbdIndex;
	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(portNo, chNo);
	return Matrix::CreateTranslation(moveVec)
	     * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
}
