//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl11
//
// Piano keyboard controller (Flat, Playback).
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrlBase11.h"
#include "MTNoteTracker.h"


//******************************************************************************
// DX11 piano keyboard controller (Flat, Playback)
//******************************************************************************
class MTPianoKeyboardCtrl11 : public MTPianoKeyboardCtrlBase11
{
public:

	MTPianoKeyboardCtrl11();
	virtual ~MTPianoKeyboardCtrl11();

	virtual int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				bool isSingleKeyboard
			);

	void Release();
	void Reset() override;

protected:

	// -- Base11 hooks --

	void _UpdateKeyStates(unsigned long kbdIndex, const MTSceneUpdateContext& ctx) override;

	// -- Pure virtual hooks (derived classes must implement) --

	virtual int _CreateKeyboards(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData
			) = 0;

	virtual void _GetPerKeyIndexParams(
				unsigned long kbdIndex,
				int& outPortFilter,
				int& outChFilter
			) = 0;

	virtual void _ApplyActiveKeyColor(
				MTKbdSub* pSub,
				unsigned long kbdIndex
			) = 0;

	// -- Shared Playback logic --

	int _BuildPerKeyIndex(MTKbdSub* pSub, int portFilter, int chFilter = -1);
	void _EvaluateKeyStates(MTKbdSub* pSub, unsigned long playTimeMSec);
	float _GetMaxPitchBendShiftAllPorts(SMPortList* pPortList);

	MTNoteTracker* m_pNoteTracker;
};
