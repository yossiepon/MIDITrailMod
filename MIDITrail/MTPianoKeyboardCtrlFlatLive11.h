//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlFlatLive11
//
// Piano keyboard controller for Live mode (Flat, DX11).
// Receives note events from NoteTrackerLive via listener interface.
// Manages per-note key states directly (no per-key index, no NoteTracker data).
// Creates one keyboard per channel (SM_MAX_CH_NUM) when !isSingleKeyboard.
// Serves both Roll and Rain Live scenes.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrlBase11.h"
#include "MTNoteTrackerBase.h"


//******************************************************************************
// Piano keyboard controller (Flat, Live)
//******************************************************************************
class MTPianoKeyboardCtrlFlatLive11 : public MTPianoKeyboardCtrlBase11,
                                      public IMTNoteTrackerListener
{
public:

	MTPianoKeyboardCtrlFlatLive11();
	virtual ~MTPianoKeyboardCtrlFlatLive11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				MTNotePitchBend* pNotePitchBend,
				bool isSingleKeyboard
			);
	void Release();
	void Reset() override;

	// IMTNoteTrackerListener
	void OnNoteActivate(const NoteData& note, unsigned long index) override;
	void OnNoteDeactivate(const NoteData& note, unsigned long index) override;
	void OnReset() override;

	void SetPlaybackPosTracking(bool enable) { m_isPlaybackPosTracking = enable; }

protected:

	// Base11 hooks
	void _UpdateKeyStates(unsigned long kbdIndex, const MTSceneUpdateContext& ctx) override;
	DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				unsigned long kbdIndex,
				const MTSceneUpdateContext& ctx) override;

	struct LiveKeyState {
		bool isDown;
		unsigned long startTimeMs;
		unsigned long endTimeMs;
		unsigned long color;
		unsigned char chNo;
	};

	LiveKeyState m_LiveKeys[SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];
	unsigned long m_LiveTimeMSec;
	bool m_isPlaybackPosTracking;

	int _CreateKeyboards(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	                     const TCHAR* pSceneName);
	void _EvaluateLiveKeyStates(unsigned long kbdIndex);
};
