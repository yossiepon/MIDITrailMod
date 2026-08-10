//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlBase11
//
// Piano keyboard controller base class (DX11).
// Shared: texture loading, keyboard sub management, Draw, Release.
// Derived: Flat (Playback/Live), Ring (future).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "MTPianoKeyboard11.h"
#include "MTNoteDesign11.h"
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

	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

	void Reset() override;
	void SetSkipStatus(bool isSkipping) { m_isSkipping = isSkipping; }

protected:

	int _LoadTexture(ID3D11Device* pDevice, const TCHAR* pSceneName);
	void _ReleaseSub(MTKbdSub* pSub);

	MTKbdSub m_Subs[SM_MAX_CH_NUM];
	unsigned long m_NumKbd;
	ID3D11DeviceContext* m_pContext;
	ID3D11ShaderResourceView* m_pSRV;
	MTNotePitchBend* m_pNotePitchBend;
	MTNoteDesign11* m_pNoteDesign;
	unsigned long m_KeyDownDurMs;
	unsigned long m_KeyUpDurMs;
	bool m_isSingleKeyboard;
	bool m_isSkipping;
};
