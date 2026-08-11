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
// Compute world matrix (Rain: translation + Y rotation + PB shift)
//******************************************************************************
Matrix MTPianoKeyboardCtrlRainLive11::_ComputeWorldMatrix(
		unsigned long kbdIndex,
		const MTSceneUpdateContext& ctx
	)
{
	unsigned char portNo = 0;
	unsigned char chNo = (unsigned char)kbdIndex;
	Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(portNo, chNo);

	if (m_isSingleKeyboard) {
		moveVec.x += m_KeyboardDesign.GetMaxPitchBendShift(m_pNotePitchBend, portNo);
	}
	else if (m_pNotePitchBend != NULL) {
		short v = m_pNotePitchBend->GetValue(portNo, chNo);
		unsigned char s = m_pNotePitchBend->GetSensitivity(portNo, chNo);
		moveVec.x += m_KeyboardDesign.GetPitchBendShift(v, s);
	}

	return Matrix::CreateTranslation(moveVec)
	     * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
}
