//******************************************************************************
//
// MIDITrail / DXCamera
//
// �J�����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// �J�����N���X
//******************************************************************************
class DXCamera
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DXCamera(void);
	virtual ~DXCamera(void);

	//������
	int Initialize();

	//��{�p�����[�^�ݒ�
	void SetBaseParam(
			float viewAngle,
			float nearPlane,
			float farPlane
		);

	//�J�����ʒu�ݒ�
	void SetPosition(
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 camLookAtVector,
			D3DXVECTOR3 camUpVector
		);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//�J�����̉�p
	float m_ViewAngle;

	//Near�v���[���F0����Z�������䂪���������Ȃ�
	float m_NearPlane;

	//Far�v���[��
	float m_FarPlane;

	//�J�����ʒu
	D3DXVECTOR3 m_CamVector;

	//���ړ_
	D3DXVECTOR3 m_CamLookAtVector;

	//�J�����̏����
	D3DXVECTOR3 m_CamUpVector;

	void _Clear();

	int _GetProjMatrix(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXMATRIX* pViewMatrix
		);
	int _GetViewMatrix(
			D3DXMATRIX* pViewMatrix
		);

};

