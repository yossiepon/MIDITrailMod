//******************************************************************************
//
// Simple MIDI Library / SMEventSysMsg
//
// �V�X�e�����b�Z�[�W�C�x���g�N���X
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �C�x���g�N���X����h��������݌v�����z�����Anew�̎��{�񐔂�����������
// ���߁A�X�^�b�N�ŏ����ł���f�[�^��̓��[�e�B���e�B�N���X�Ƃ��Ď�������B

// �{�N���X�̑ΏۂƂ���V�X�e�����b�Z�[�W�ꗗ
//   F1 dd     �V�X�e���R�������b�Z�[�W�F�N�I�[�^�[�t���[��(MTC)
//   F2 dl dm  �V�X�e���R�������b�Z�[�W�F�\���O�|�W�V�����|�C���^
//   F3 dd     �V�X�e���R�������b�Z�[�W�F�\���O�Z���N�g
//   F4 ����`
//   F5 ����`
//   F6 �V�X�e���R�������b�Z�[�W�F�`���[�����N�G�X�g
//   F8 �V�X�e�����A���^�C�����b�Z�[�W�F�^�C�~���O�N���b�N
//   F9 ����`
//   FA �V�X�e�����A���^�C�����b�Z�[�W�F�X�^�[�g
//   FB �V�X�e�����A���^�C�����b�Z�[�W�F�R���e�B�j���[
//   FC �V�X�e�����A���^�C�����b�Z�[�W�F�X�g�b�v
//   FD ����`
//   FE �V�X�e�����A���^�C�����b�Z�[�W�F�A�N�e�B�u�Z���V���O
//   FF �V�X�e�����A���^�C�����b�Z�[�W�F�V�X�e�����Z�b�g
// ���L���b�Z�[�W�͖{�N���X�̑ΏۊO�Ƃ���
//   F0 ... F7 �V�X�e���G�N�X�N���[�V�u
//   F7 �G���h�I�u�V�X�e���G�N�X�N���[�V�u

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"


namespace SMIDILib {

//******************************************************************************
// �V�X�e�����b�Z�[�W�C�x���g�N���X
//******************************************************************************
class SMIDILIB_API SMEventSysMsg
{
public:

	//�V�X�e�����b�Z�[�W���
	enum SysMsg {
		None						= 0x00, // none
		//�V�X�e���R�������b�Z�[�W
		Common_QuarterFrame			= 0xF1, // F1 dd
		Common_SongPositionPointer	= 0xF2, // F2 dl dm
		Common_SongSelect			= 0xF3, // F3 dd
		Common_TuneRequest			= 0xF6, // F6
		//�V�X�e�����A���^�C�����b�Z�[�W
		RealTime_TimingClock		= 0xF8, // F8
		RealTime_Start				= 0xFA, // FA
		RealTime_Continue			= 0xFB, // FB
		RealTime_Stop				= 0xFC, // FC
		RealTime_ActiveSensing		= 0xFE, // FE
		RealTime_SystemReset		= 0xFF  // FF
	};

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMEventSysMsg();
	virtual ~SMEventSysMsg(void);
	
	//�C�x���g�A�^�b�`
	void Attach(SMEvent* pEvent);
	
	//MIDI�o�̓��b�Z�[�W�擾
	int GetMIDIOutShortMsg(unsigned long* pMsg, unsigned long* pSize);
	
	//�V�X�e�����b�Z�[�W�擾
	SysMsg GetSysMsg();
	
private:
	
	SMEvent* m_pEvent;
	
	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMEventSysMsg&);
	SMEventSysMsg(const SMEventSysMsg&);
	
};

} // end of namespace


