//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlMod11
//
// DX11 piano keyboard controller Mod.
// Multi-port support: creates MTPianoKeyboardMod11 per active port.
// World matrix includes Mod orientation (scale + rotation + playback tracking).
// Used by PianoRoll3D / Ring scenes.
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrl11.h"
#include "MTPianoKeyboardDesignMod.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// DX11 piano keyboard controller Mod
//******************************************************************************
class MTPianoKeyboardCtrlMod11 : public MTPianoKeyboardCtrl11
{
public:

	MTPianoKeyboardCtrlMod11();
	virtual ~MTPianoKeyboardCtrlMod11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				bool isSingleKeyboard
			) override;

	int Update(const MTSceneUpdateContext& ctx) override;
	void Reset() override;

protected:

	int _CreateKeyboards(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData
			) override;

private:

	MTPianoKeyboardDesignMod m_DesignMod;
	MTNoteDesignMod m_NoteDesignMod;
	SMPortList m_PortList;
	int m_KeyboardIndex[SM_MAX_PORT_NUM];
	unsigned char m_MaxKeyboardIndex;

	float _GetMaxPitchBendShift(unsigned char portNo);
};
