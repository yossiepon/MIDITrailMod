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
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���Z�b�g
	virtual void Reset();

protected:

	virtual int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

private:

	//�m�[�g�f�U�C��
	MTNoteDesignMod m_NoteDesignMod;

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

	//�|�[�g���X�g
	SMPortList m_PortList;
	int m_PortIndex[SM_MAX_PORT_NUM];
	unsigned char m_MaxPortIndex;

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

};


