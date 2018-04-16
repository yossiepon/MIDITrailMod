//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlMod
//
// �s�A�m�L�[�{�[�h����Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrl.h"
#include "MTPianoKeyboardDesignMod.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// �s�A�m�L�[�{�[�h����Mod�N���X
//******************************************************************************
class MTPianoKeyboardCtrlMod : public MTPianoKeyboardCtrl
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboardCtrlMod(void);
	virtual ~MTPianoKeyboardCtrlMod(void);

	//����
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend,
			bool isSingleKeyboard
		);

	//�X�V
	int Transform(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 lookVector,
			float rollAngle
		);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���Z�b�g
	void Reset();

protected:

	int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

private:

	//�m�[�g�f�U�C��
	MTNoteDesignMod m_NoteDesignMod;

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

	//�A�N�e�B�u�|�[�g�t���O
	bool m_isActivePort[SM_MAX_PORT_NUM];

	//�L�[�������z��
	float m_KeyDownRateMod[SM_MAX_CH_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	virtual int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long keyDownDuration,
				unsigned long keyUpDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	virtual int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

	float _GetMaxPitchBendShift(int keyboardIndex);
	float _GetMaxPitchBendShift(int keyboardIndex, float max);
};


