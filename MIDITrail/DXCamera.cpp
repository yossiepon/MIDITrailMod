//******************************************************************************
//
// MIDITrail / DXCamera
//
// �J�����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXCamera.h"
#include "YNBaseLib.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
DXCamera::DXCamera(void)
{
	_Clear();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
DXCamera::~DXCamera(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int DXCamera::Initialize()
{
	_Clear();
	return 0;
}

//******************************************************************************
// ��{�p�����[�^�ݒ�
//******************************************************************************
void DXCamera::SetBaseParam(
		float viewAngle,
		float nearPlane,
		float farPlane
	)
{
	m_ViewAngle = viewAngle;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
}

//******************************************************************************
// �J�����ʒu�ݒ�
//******************************************************************************
void DXCamera::SetPosition(
		D3DXVECTOR3 camVector,
		D3DXVECTOR3 camLookAtVector,
		D3DXVECTOR3 camUpVector
	)
{
	m_CamVector = camVector;
	m_CamLookAtVector = camLookAtVector;
	m_CamUpVector = camUpVector;
}

//******************************************************************************
// �ϊ�
//******************************************************************************
int DXCamera::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DXMATRIX viewMatrix;
	D3DXMATRIX projMatrix;

	//�ˉe�s����擾
	result = _GetProjMatrix(pD3DDevice, &projMatrix);
	if (result != 0) goto EXIT;

	//�ˉe�s��������_�����O�p�C�v���C���ɐݒ�
	hresult = pD3DDevice->SetTransform(D3DTS_PROJECTION, &projMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�r���[�C���O�s����擾
	result = _GetViewMatrix(&viewMatrix);
	if (result != 0) goto EXIT;

	//�r���[�C���O�s��������_�����O�p�C�v���C���ɐݒ�
	hresult = pD3DDevice->SetTransform(D3DTS_VIEW, &viewMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �N���A
//******************************************************************************
void DXCamera::_Clear()
{
	m_ViewAngle = 45.0f;
	m_NearPlane = 1.0f;
	m_FarPlane = 1000.0f;
	m_CamVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_CamLookAtVector = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_CamUpVector = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
}

//******************************************************************************
// �ˉe��擾
//******************************************************************************
int DXCamera::_GetProjMatrix(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXMATRIX* pViewMatrix
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DVIEWPORT9 viewPort;
	float aspect = 0.0f;

	//�s�񏉊���
	D3DXMatrixIdentity(pViewMatrix);

	//�r���[�|�[�g�擾
	hresult = pD3DDevice->GetViewport(&viewPort);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�A�X�y�N�g��
	aspect = (float)viewPort.Width / (float)viewPort.Height;
	
	//����n�ˉe�}�g���b�N�X�쐬
	D3DXMatrixPerspectiveFovLH(
			pViewMatrix,				//�������ꂽ�s��
			D3DXToRadian(m_ViewAngle),	//�J�����̉�p
			aspect,						//�A�X�y�N�g��
			m_NearPlane,				//near�v���[��
			m_FarPlane					//far�v���[��
		);

EXIT:;
	return result;
}

//******************************************************************************
// �r���[�ϊ��s��擾
//******************************************************************************
int DXCamera::_GetViewMatrix(
		D3DXMATRIX* pViewMatrix
	)
{
	int result = 0;

	//�r���[�ϊ��s�񐶐�
	D3DXMatrixLookAtLH(
			pViewMatrix,		//�쐬���ꂽ�s��
			&m_CamVector,		//�J�����ʒu
			&m_CamLookAtVector,	//���ړ_
			&m_CamUpVector		//�J�����̏����
		);

	return result;
}

