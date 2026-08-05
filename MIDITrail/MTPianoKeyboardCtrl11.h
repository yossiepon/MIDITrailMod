//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl11
//
// DX11 piano keyboard controller.
// Manages multiple keyboards (one per channel), evaluates key states via
// per-key index built from NoteTracker data, and dispatches to each
// MTPianoKeyboard11 via Update(keyStates, world).
// Used by Rain scene (non-Mod keyboard, simple world transform).
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "MTPianoKeyboard11.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNoteDesign.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"
#include "DXTexture11.h"


//******************************************************************************
// Per-key note record (compact)
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

	// per-key index (built from NoteTracker)
	MTKbdNote* pNotes;
	unsigned long noteCount;
	unsigned long keyOffset[SM_MAX_NOTE_NUM + 1];
	unsigned long keyCursor[SM_MAX_NOTE_NUM];

	// evaluation result (passed to Keyboard::Update)
	MTKeyboardKeyState keyStates[SM_MAX_NOTE_NUM];
};


//******************************************************************************
// DX11 piano keyboard controller
//******************************************************************************
class MTPianoKeyboardCtrl11 : public MTSceneComponent11
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

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

	void Reset();
	void SetSkipStatus(bool isSkipping) { m_isSkipping = isSkipping; }

protected:

	virtual int _CreateKeyboards(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData
			);
	int _BuildPerKeyIndex(MTKbdSub* pSub, int portFilter);
	void _EvaluateKeyStates(MTKbdSub* pSub, unsigned long playTimeMSec);

	MTKbdSub m_Subs[SM_MAX_CH_NUM];
	unsigned long m_NumKbd;
	ID3D11DeviceContext* m_pContext;
	ID3D11ShaderResourceView* m_pSRV;
	MTNoteTracker* m_pNoteTracker;
	MTNotePitchBend* m_pNotePitchBend;
	MTPianoKeyboardDesign m_KeyboardDesign;
	MTNoteDesign m_NoteDesign;
	unsigned long m_KeyDownDurMs;
	unsigned long m_KeyUpDurMs;
	bool m_isSingleKeyboard;
	bool m_isSkipping;

private:

	int _LoadTexture(ID3D11Device* pDevice, const TCHAR* pSceneName);
	void _ReleaseSub(MTKbdSub* pSub);
};
