//******************************************************************************
//
// Simple MIDI Library / SMEventMeta
//
// ���^�C�x���g�N���X
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
#include <string>

namespace SMIDILib {


//******************************************************************************
// ���^�C�x���g�N���X
//******************************************************************************
class SMIDILIB_API SMEventMeta
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMEventMeta();
	~SMEventMeta(void);

	//�C�x���g�A�^�b�`
	void Attach(SMEvent* pEvent);

	//���^�^�C�v�擾
	unsigned char GetType();

	//�e���|�擾
	unsigned long GetTempo();

	//�e���|�擾(BPM)
	unsigned long GetTempoBPM();

	//�e�L�X�g�擾
	int GetText(std::string* pText);

	//�|�[�g�ԍ��擾
	unsigned char GetPortNo();

	//���q�L���擾
	void GetTimeSignature(unsigned long* pNumerator, unsigned long* pDenominator);

private:

	SMEvent* m_pEvent;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMEventMeta&);
	SMEventMeta(const SMEventMeta&);

};

} // end of namespace

