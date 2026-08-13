//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRainLive11
//
// Piano keyboard controller for Rain (Live).
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
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
		moveVec.x += m_KeyboardDesign.GetMaxPitchBendShift(m_pNotePitchBend, portNo,
		             GetActiveChannelMask(portNo));
	}
	else if (m_pNotePitchBend != NULL && m_ActiveNoteCountPerKbd[kbdIndex] > 0) {
		short v = m_pNotePitchBend->GetValue(portNo, chNo);
		unsigned char s = m_pNotePitchBend->GetSensitivity(portNo, chNo);
		moveVec.x += m_KeyboardDesign.GetPitchBendShift(v, s);
	}

	return Matrix::CreateTranslation(moveVec)
	     * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
}
