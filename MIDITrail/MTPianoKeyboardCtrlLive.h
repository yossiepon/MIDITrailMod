//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlLive
//
// ���C�u���j�^�p�s�A�m�L�[�{�[�h����N���X
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
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


//******************************************************************************
// ���C�u���j�^�p�s�A�m�L�[�{�[�h����N���X
//******************************************************************************
class MTPianoKeyboardCtrlLive
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboardCtrlLive(void);
	virtual ~MTPianoKeyboardCtrlLive(void);
	
	//����
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			MTNotePitchBend* pNotePitchBend,
			bool isSingleKeyboard
		);
	
	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
	
	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//���
	void Release();
	
	//���Z�b�g
	void Reset();
	
	//�\���ݒ�
	void SetEnable(bool isEnable);
	
	//�m�[�gON�o�^
	void SetNoteOn(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo,
			unsigned char velocity
		);
	
	//�m�[�gOFF�o�^
	void SetNoteOff(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo
		);
	
	//�S�m�[�gOFF
	void AllNoteOff();
	void AllNoteOffOnCh(unsigned char portNo, unsigned char chNo);
	
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
		unsigned long startTime;
		unsigned long endTime;
		float keyDownRate;
	};
	
private:
	
	//�m�[�g�f�U�C��
	MTNoteDesign m_NoteDesign;
	
	//�L�[�{�[�h�`��I�u�W�F�N�g�F�|�C���^�z��
	MTPianoKeyboard* m_pPianoKeyboard[SM_MAX_CH_NUM];
	
	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesign m_KeyboardDesign;
	
	//�m�[�g���
	NoteStatus m_NoteStatus[SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];
	
	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;
	
	//�\����
	bool m_isEnable;
	
	//�V���O���L�[�{�[�h�t���O
	bool m_isSingleKeyboard;
	
	void _ClearNoteStatus();
	int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	
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


