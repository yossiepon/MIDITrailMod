//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlBase11
//
// Piano keyboard controller base class.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "MTPianoKeyboard11.h"
#include "MTNoteDesign11.h"
#include "MTPianoKeyboardDesign11.h"
#include "MTNotePitchBend.h"
#include "DXTexture11.h"


//******************************************************************************
// Per-key note record (compact, used by Playback Flat)
//******************************************************************************
struct MTKbdNote {
	unsigned long startTimeMs;
	unsigned long endTimeMs;
	unsigned long color;
	unsigned char chNo;
};

//******************************************************************************
// Sub-keyboard state
//******************************************************************************
struct MTKbdSub {
	MTPianoKeyboard11* pKeyboard;

	// per-key index (Playback Flat only; Live leaves these unused)
	MTKbdNote* pNotes;
	unsigned long noteCount;
	unsigned long keyOffset[SM_MAX_NOTE_NUM + 1];
	unsigned long keyCursor[SM_MAX_NOTE_NUM];

	// evaluation result (passed to Keyboard::Update)
	MTKeyboardKeyState keyStates[SM_MAX_NOTE_NUM];
};


//******************************************************************************
// Piano keyboard controller base class
//******************************************************************************
class MTPianoKeyboardCtrlBase11 : public MTSceneComponent11
{
public:

	MTPianoKeyboardCtrlBase11();
	virtual ~MTPianoKeyboardCtrlBase11();

	void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

	void Reset() override;
	void SetSkipStatus(bool isSkipping) { m_isSkipping = isSkipping; }

protected:

	virtual void _UpdateKeyStates(unsigned long kbdIndex, const MTSceneUpdateContext& ctx) = 0;
	virtual DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				unsigned long kbdIndex,
				const MTSceneUpdateContext& ctx) = 0;

	int _LoadTexture(ID3D11Device* pDevice, const TCHAR* pSceneName);
	void _ReleaseSub(MTKbdSub* pSub);

	static const unsigned long MT_KBD_MAX_SUBS = 128;
	MTKbdSub m_Subs[MT_KBD_MAX_SUBS];
	unsigned long m_NumKbd;
	ID3D11DeviceContext* m_pContext;
	ID3D11ShaderResourceView* m_pSRV;
	MTNotePitchBend* m_pNotePitchBend;
	MTNoteDesign11 m_NoteDesign;
	MTNoteDesign11* m_pNoteDesign;
	MTPianoKeyboardDesign11 m_KeyboardDesign;
	unsigned long m_KeyDownDurMs;
	unsigned long m_KeyUpDurMs;
	bool m_isSingleKeyboard;
	bool m_isSkipping;
};
