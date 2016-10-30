//******************************************************************************
//
// MIDITrail / DIKeyCtrl
//
// DirectInput �L�[���͐���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DirectInput��p���ăL�[�{�[�h�̏�Ԃ��擾����B
// ����̓C�x���g�o�b�t�@�Q�Ƌ@�\�������Ȃ��B

#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


//******************************************************************************
// DirectInput �L�[���͐���N���X
//******************************************************************************
class DIKeyCtrl
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DIKeyCtrl(void);
	virtual ~DIKeyCtrl(void);

	//�������^�I��
	int Initialize(HWND hWnd);
	void Terminate();

	//�A�N�Z�X���擾�^���
	int Acquire();
	int Unacquire();

	//�����_�̏�Ԃ��擾
	//  GetKeyStatus�����Ăяo���Ă���
	//  ��Ԃ��擾�������L�[�̐�����IsKeyDown���Ăяo��
	//  BUG: �E�B���h�E����A�N�e�B�u��Ԃ̏ꍇ�Ƀf�o�C�X�փA�N�Z�X�ł���
	//       GetKeyStatus()���G���[�ɂȂ�
	int GetKeyStatus();
	bool IsKeyDown(unsigned char key);

private:

	LPDIRECTINPUT8 m_pDI;
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	unsigned char m_KeyStatus[256];

};


