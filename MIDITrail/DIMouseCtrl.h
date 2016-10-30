//******************************************************************************
//
// MIDITrail / DIMouseCtrl
//
// DirectInput �}�E�X����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DirectInput��p���ă}�E�X�̏�Ԃ��擾����B
// ��ԎQ�ƂƃC�x���g�o�b�t�@�Q�Ƃ̋@�\�����B

// BUG:
// �o�b�t�@�T�C�Y���w�肷��C���^�[�t�F�[�X���Ȃ��B

#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


//******************************************************************************
// DirectInput �}�E�X����N���X
//******************************************************************************
class DIMouseCtrl
{
public:

	//�}�E�X�{�^�����
	enum MouseButton {
		LeftButton,
		RightButton,
		CenterButton
	};

	//�}�E�X�����
	enum MouseAxis {
		AxisX,
		AxisY,
		AxisWheel
	};

	//�}�E�X�C�x���g���
	enum MouseEvent {
		LeftButtonDown,
		LeftButtonUp,
		RightButtonDown,
		RightButtonUp,
		CenterButtonDown,
		CenterButtonUp,
		AxisXMove,
		AxisYMove,
		AxisWheelMove
	};

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DIMouseCtrl(void);
	virtual ~DIMouseCtrl(void);

	//�������^�I��
	int Initialize(HWND hWnd);
	void Terminate();

	//�A�N�Z�X���擾�^���
	int Acquire();
	int Unacquire();

	//�����_�̏�Ԃ��擾
	//  GetMouseStatus�����Ăяo���Ă���
	//  ��Ԃ��擾�������{�^���Ǝ��̐�����IsBtnDown,GetDelta���Ăяo��
	//  BUG: �E�B���h�E����A�N�e�B�u��Ԃ̏ꍇ�Ƀf�o�C�X�փA�N�Z�X�ł���
	//       GetMouseStatus()���G���[�ɂȂ�
	int GetMouseStatus();
	bool IsBtnDown(MouseButton);
	int GetDelta(MouseAxis);

	//�o�b�t�@�f�[�^���擾
	//  pIsExist��false�ɂȂ�܂ŌJ��Ԃ��Ăяo��
	//  �Ăяo�����тɎ擾�����o�b�t�@���폜�����
	int GetBuffer(bool* pIsExist, MouseEvent* pEvent, int* pDeltaAxis = NULL);

private:

	LPDIRECTINPUT8 m_pDI;
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	DIMOUSESTATE2 m_MouseState;

};


