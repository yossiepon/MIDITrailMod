//******************************************************************************
//
// MIDITrail / MTFirstPersonCam
//
// ��l�̃J�����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// FPS�Q�[�����C�N�Ȏ��_�ړ�����������B
// �{�N���X���ŃL�[�{�[�h�^�}�E�X�̏�Ԃ��擾����B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DIKeyCtrl.h"
#include "DIMouseCtrl.h"
#include "DXCamera.h"
#include "SMIDILib.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�J�����ʒu�ő�͈�
#define MTFIRSTPERSONCAM_CAMVECTOR_LIMIT  (1000000.0f)


//******************************************************************************
// ��l�̃J�����N���X
//******************************************************************************
class MTFirstPersonCam
{
public:

	enum MTProgressDirection {
		DirX,
		DirY,
		DirZ
	};

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTFirstPersonCam(void);
	virtual ~MTFirstPersonCam(void);

	//�N���A
	int Clear();

	//������
	int Initialize(HWND hWnd, const TCHAR* pSceneName, SMSeqData* pSeqData);

	//�J�����ʒu�ݒ�
	void SetPosition(
			D3DXVECTOR3 camVector
		);

	//�J���������ݒ�
	//  ���ʊp�FXZ���ʏ��X���Ƃ̊p�x +X������=0�x +Z������=90�x
	//  �V���p�FY���Ƃ̊p�x           +Y������=0�x XZ���ʏ�=90�x
	void SetDirection(
			float phi,		//���ʊp
			float theta		//�V���p
		);

	//�J�����ʒu�擾
	void GetPosition(D3DXVECTOR3* pCamVector);

	//�J���������擾
	void GetDirection(
			float* pPhi,
			float* pTheta
		);

	//�}�E�X�����ړ����[�h�o�^
	void SetMouseCamMode(bool isEnable);

	//������]���[�h�o�^
	void SetAutoRollMode(bool isEnable);
	void SwitchAutoRllDirecton();

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//���t�`�b�N�^�C���o�^
	void SetCurTickTime(unsigned long curTickTime);

	//���Z�b�g
	void Reset();

	//��]�p�x�擾
	float GetManualRollAngle();
	float GetAutoRollVelocity();

	//��]�p�x�ݒ�
	void SetManualRollAngle(float rollAngle);
	void SetAutoRollVelocity(float rollVelocity);

	//�i�s�����ݒ�
	void SetProgressDirection(MTProgressDirection dir);

private:

	DXCamera m_Camera;
	D3DXVECTOR3 m_CamVector;
	float m_CamDirPhi;
	float m_CamDirTheta;
	MTProgressDirection m_ProgressDirection;

	DIKeyCtrl m_DIKeyCtrl;
	DIMouseCtrl m_DIMouseCtrl;
	bool m_IsMouseCamMode;
	bool m_IsAutoRollMode;
	HWND m_hWnd;
	MTNoteDesign m_NoteDesign;

	//�ړ����x
	float m_VelocityFB;		//�O��ړ��� m/sec.
	float m_VelocityLR;		//���E�ړ��� m/sec.
	float m_VelocityUD;		//�㉺�ړ��� m/sec.
	float m_VelocityPT;		//�����ړ��� degrees/sec.
	float m_AcceleRate;		//�����{��

	//��]����n
	float m_RollAngle;
	float m_VelocityAutoRoll;
	float m_VelocityManualRoll;

	unsigned long m_PrevTime;
	unsigned long m_DeltaTime;

	unsigned long m_PrevTickTime;
	unsigned long m_CurTickTime;

	int _TransformEyeDirection(int dX, int dY);
	int _TransformCamPosition();
	int _TransformRolling(int dW);
	int _SetCamPosition();
	int _ClipCursor(bool isClip);
	void _CalcDeltaTime();
	int _LoadConfFile(const TCHAR* pSceneName);
	void _ClipCamVector(D3DXVECTOR3* pVector);

};


