//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRoll11
//
// DX11 piano keyboard controller for PianoRoll scene.
// Multi-port support: creates MTPianoKeyboardFlat11 per active port.
// World matrix is simplified (model orientation baked into keyboard vertices).
// Used by PianoRoll3D / 2D scenes.
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrl11.h"


//******************************************************************************
// DX11 piano keyboard controller for PianoRoll scene
//******************************************************************************
class MTPianoKeyboardCtrlRoll11 : public MTPianoKeyboardCtrl11
{
public:

	MTPianoKeyboardCtrlRoll11();
	virtual ~MTPianoKeyboardCtrlRoll11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				bool isSingleKeyboard
			) override;

protected:

	int _CreateKeyboards(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData
			) override;

	void _GetPerKeyIndexParams(
				unsigned long kbdIndex,
				int& outPortFilter,
				int& outChFilter
			) override;

	void _ApplyActiveKeyColor(
				MTKbdSub* pSub,
				unsigned long kbdIndex
			) override;

	DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				unsigned long kbdIndex,
				const MTSceneUpdateContext& ctx
			) override;

private:

	SMPortList m_PortList;
	int m_KeyboardIndex[SM_MAX_PORT_NUM];
	unsigned char m_KbdPortNo[SM_MAX_CH_NUM];
	unsigned char m_MaxKeyboardIndex;

};
