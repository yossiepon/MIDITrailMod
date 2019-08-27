//******************************************************************************
//
// MIDITrail / MTGamePadCtrl
//
// �Q�[���p�b�h����N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// XInput��p���ăQ�[���p�b�h�̏�Ԃ��擾����B

#pragma once

#include <XInput.h>


//******************************************************************************
// �p�����[�^��`
//******************************************************************************

//�g���K�[ON臒l(0-255)
#define MT_GAME_PAD_TRRIGER_ON_THRESHOLD		(250)


//******************************************************************************
// �Q�[���p�b�h����N���X
//******************************************************************************
class MTGamePadCtrl
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTGamePadCtrl(void);
	virtual ~MTGamePadCtrl();
	
	//������
	int Initialize(int userIndex);
	
	//��ԍX�V
	int UpdateState();
	
	//�{�^����Ԏ擾
	bool GetState_DPadUp();
	bool GetState_DPadDown();
	bool GetState_DPadLeft();
	bool GetState_DPadRight();
	bool GetState_A();
	bool GetState_B();
	bool GetState_X();
	bool GetState_Y();
	bool GetState_LShoulder();
	bool GetState_RShoulder();
	bool GetState_LTrigger();
	bool GetState_RTrigger();
	bool GetState_Start();
	bool GetState_Back();
	
	//�X�e�B�b�N��Ԏ擾(0.0 - 1.0)
	float GetState_ThumbLX();
	float GetState_ThumbLY();
	float GetState_ThumbRX();
	float GetState_ThumbRY();
	
	//�{�^�������m�F
	bool DidPressNow_A();
	bool DidPressNow_B();
	bool DidPressNow_X();
	bool DidPressNow_Y();
	bool DidPressNow_LShoulder();
	bool DidPressNow_RShoulder();
	bool DidPressNow_LTrigger();
	bool DidPressNow_RTrigger();
	bool DidPressNow_Start();
	bool DidPressNow_Back();
	
private:
	
	//���[�U�C���f�b�N�X
	int m_UserIndex;
	
	//�Q�[���p�b�h�ڑ����
	bool m_isConnected;
	
	//�Q�[���p�b�h���
	XINPUT_GAMEPAD m_PrevGamePadState;
	XINPUT_GAMEPAD m_CurGamePadState;

	//�X�e�B�b�N���
	float m_ThumbLX;
	float m_ThumbLY;
	float m_ThumbRX;
	float m_ThumbRY;

	void _NormalizeLStickState();
	void _NormalizeRStickState();

};






