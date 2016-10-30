//******************************************************************************
//
// MIDITrail / MTNotePitchBend
//
// �s�b�`�x���h���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �|�[�g�^�`�����l���P�ʂ̃s�b�`�x���h����ێ�����B

#pragma once

#include "SMCommon.h"


//******************************************************************************
// �s�b�`�x���h���N���X
//******************************************************************************
class MTNotePitchBend
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNotePitchBend(void);
	virtual ~MTNotePitchBend(void);

	//������
	int Initialize();

	//�s�b�`�x���h�o�^
	int SetPitchBend(
			unsigned char portNo,
			unsigned char chNo,
			short value,
			unsigned char sensitivity
		);

	//�s�b�`�x���h�l�擾
	short GetValue(unsigned long portNo, unsigned long chNo);

	//�s�b�`�x���h���x�擾
	unsigned char GetSensitivity(unsigned long portNo, unsigned long chNo);

	//���Z�b�g
	void Reset();

	//�s�b�`�x���h�\�����ʐݒ�
	void SetEnable(bool isEnable);

private:

	//�s�b�`�x���h���
	struct MTNOTEPITCHBEND_PITCHBEND_INFO {
		short value;
		unsigned char sensitivity;
	};

private:

	//�s�b�`�x���h�\������
	bool m_isEnable;

	//�s�b�`�x���h���
	MTNOTEPITCHBEND_PITCHBEND_INFO m_PitchBend[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];

};

