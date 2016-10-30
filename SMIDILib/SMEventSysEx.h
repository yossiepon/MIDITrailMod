//******************************************************************************
//
// Simple MIDI Library / SMEventSysEx
//
// SysEx�C�x���g�N���X
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
// SysEx�C�x���g�N���X
//******************************************************************************
class SMIDILIB_API SMEventSysEx
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMEventSysEx();
	virtual ~SMEventSysEx(void);

	//�C�x���g�A�^�b�`
	void Attach(SMEvent* pEvent);

	//MIDI�o�̓��b�Z�[�W�擾
	int GetMIDIOutLongMsg(unsigned char** pPtrMsg, unsigned long* pSize);

private:

	SMEvent* m_pEvent;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMEventSysEx&);
	SMEventSysEx(const SMEventSysEx&);

};

} // end of namespace

