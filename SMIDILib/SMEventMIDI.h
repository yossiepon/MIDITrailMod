//******************************************************************************
//
// Simple MIDI Library / SMEventMIDI
//
// MIDI�C�x���g�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �C�x���g�N���X����h��������݌v�����z�����Anew�̎��{�񐔂�����������
// ���߁A�X�^�b�N�ŏ����ł���f�[�^��̓��[�e�B���e�B�N���X�Ƃ��Ď�������B

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"


namespace SMIDILib {

//******************************************************************************
// MIDI�C�x���g�N���X
//******************************************************************************
class SMIDILIB_API SMEventMIDI
{
public:

	//�`�����l�����b�Z�[�W���
	enum ChMsg {
		None					= 0x00, // none
		NoteOff					= 0x80, // 8n kk vv
		NoteOn					= 0x90, // 9n kk vv
		PolyphonicKeyPressure	= 0xA0, // An kk vv
		ControlChange			= 0xB0, // Bn cc vv
		ProgramChange			= 0xC0, // Cn pp   
		ChannelPressure			= 0xD0, // Dn vv   
		PitchBend				= 0xE0  // En mm ll
	};

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMEventMIDI();
	virtual ~SMEventMIDI(void);

	//�C�x���g�A�^�b�`
	void Attach(SMEvent* pEvent);

	//MIDI�o�̓��b�Z�[�W�擾
	int GetMIDIOutShortMsg(unsigned long* pMsg);

	//�`�����l�����b�Z�[�W
	ChMsg GetChMsg();

	//�`�����l���ԍ��擾
	unsigned char GetChNo();

	//�m�[�g�ԍ��擾
	unsigned char GetNoteNo();

	//�x���V�e�B�擾
	unsigned char GetVelocity();

	//�R���g���[���`�F���W�ԍ��擾
	unsigned char GetCCNo();

	//�R���g���[���`�F���W�l�擾
	unsigned char GetCCValue();

	//�v���O�����ԍ��擾
	unsigned char GetProgramNo();

	//�`�����l���v���b�V���[�l�擾
	unsigned char GetPressureValue();

	//�s�b�`�x���h�l�擾
	short GetPitchBendValue();

private:

	SMEvent* m_pEvent;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMEventMIDI&);
	SMEventMIDI(const SMEventMIDI&);

};

} // end of namespace

