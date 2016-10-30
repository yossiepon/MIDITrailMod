//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl
//
// �s�A�m�L�[�{�[�h����N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �����̃s�A�m�L�[�{�[�h���Ǘ�����N���X�B
// �e�L�[�{�[�h�̔z�u�ƃL�[�̉�����Ԃ𐧌䂷��B
// �����1�|�[�g(16ch)�̕`��݂̂ɑΉ����Ă���B
// 2�|�[�g�ڈȍ~�̕`��ɂ͑Ή����Ă��Ȃ��B

#pragma once

#include "SMIDILib.h"
#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNotePitchBend.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m�L�[�{�[�h����N���X
//******************************************************************************
class MTPianoKeyboardCtrl
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboardCtrl(void);
	virtual ~MTPianoKeyboardCtrl(void);

	//����
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	void Release();

	//���t�`�b�N�^�C���o�^
	void SetCurTickTime(unsigned long curTickTime);

	//���t���Ԑݒ�
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//���Z�b�g
	void Reset();

	//�\���ݒ�
	void SetEnable(bool isEnable);

	//�X�L�b�v���
	void SetSkipStatus(bool isSkipping);

private:

	//�L�[���
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//�����m�[�g���\����
	struct NoteStatus {
		bool isActive;
		KeyStatus keyStatus;
		unsigned long index;
		float keyDownRate;
	};

private:

	//�m�[�g�f�U�C��
	MTNoteDesign m_NoteDesign;

	//�L�[�{�[�h�`��I�u�W�F�N�g�F�|�C���^�z��
	MTPianoKeyboard* m_pPianoKeyboard[SM_MAX_CH_NUM];

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesign m_KeyboardDesign;

	//�m�[�g���X�g
	SMNoteList m_NoteListRT;

	//�������m�[�g�Ǘ�
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	NoteStatus* m_pNoteStatus;
	float m_KeyDownRate[SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//�X�L�b�v���
	bool m_isSkipping;

	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;

	//�\����
	bool m_isEnable;

	int _CreateNoteStatus();
	int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long keyDownDuration,
				unsigned long keyUpDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	float _GetPichBendShiftPosX(unsigned char portNo, unsigned char chNo);

};


