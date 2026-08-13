//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlRain11
//
// Piano keyboard controller for Rain scenes.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrl11.h"


//******************************************************************************
// DX11 piano keyboard controller for Rain scene
//******************************************************************************
class MTPianoKeyboardCtrlRain11 : public MTPianoKeyboardCtrl11
{
public:

	MTPianoKeyboardCtrlRain11();
	virtual ~MTPianoKeyboardCtrlRain11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				bool isSingleKeyboard
			) override;

	void SetPlaybackPosTracking(bool enable) { m_isPlaybackPosTracking = enable; }

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
	unsigned char m_KbdPortNo[MT_KBD_MAX_SUBS];
	unsigned char m_KbdChNo[MT_KBD_MAX_SUBS];
	bool m_isPlaybackPosTracking;
};
