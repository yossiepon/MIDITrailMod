//******************************************************************************
//
// MIDITrail / MTSceneTitle
//
// �^�C�g���V�[���`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************


#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTSceneTitle.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTSceneTitle::MTSceneTitle(void)
{
	m_CamPosZ = MTSCENETITLE_CAMERA_POSZ;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTSceneTitle::~MTSceneTitle(void)
{
}

//******************************************************************************
// ���̎擾
//******************************************************************************
const TCHAR* MTSceneTitle::GetName()
{
	return _T("Title");
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTSceneTitle::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//----------------------------------
	// �J����
	//----------------------------------
	//�J����������
	result = m_Camera.Initialize();
	if (result != 0) goto EXIT;

	//��{�p�����[�^�ݒ�
	m_Camera.SetBaseParam(
			45.0f,		//��p
			1.0f,		//Near�v���[��
			1000.0f		//Far�v���[��
		);

	//�J�����ʒu�ݒ�
	m_Camera.SetPosition(
			D3DXVECTOR3(0.0f, 0.0f, m_CamPosZ),	//�J�����ʒu
			D3DXVECTOR3(0.0f, 0.0f, 0.0f), 		//���ړ_
			D3DXVECTOR3(0.0f, 1.0f, 0.0f)		//�J���������
		);
	
	//----------------------------------
	// ���C�g
	//----------------------------------
	//���C�g������
	result = m_DirLight.Initialize();
	if (result != 0) goto EXIT;

	//���C�g����
	m_DirLight.SetDirection(D3DXVECTOR3(1.0f, -1.0f, 2.0f));

	//���C�g�̃f�o�C�X�o�^
//	result = m_DirLight.SetDevice(pD3DDevice, TRUE);  //���C�g����
	result = m_DirLight.SetDevice(pD3DDevice, FALSE); //���C�g�Ȃ�
	if (result != 0) goto EXIT;

	//----------------------------------
	// �`��I�u�W�F�N�g
	//----------------------------------
	//���S����
	result = m_Logo.Create(pD3DDevice);
	if (result != 0) goto EXIT;

	//----------------------------------
	// �����_�����O�X�e�[�g
	//----------------------------------
	//��ʕ`�惂�[�h
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//Z�[�x��r�FON
	 pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	//�f�B�U�����O:ON ���i���`��
	pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);

	//�}���`�T���v�����O�A���`�G�C���A�X�F�L��
	pD3DDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	//�����_�����O�X�e�[�g�ݒ�F�ʏ�̃A���t�@����
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

EXIT:;
	return result;
}

//******************************************************************************
// �ϊ�����
//******************************************************************************
int MTSceneTitle::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�J�����ʒu�ݒ�
	m_CamPosZ += MTSCENETITLE_CAMERA_POSZ_DELTA;
	m_Camera.SetPosition(
			D3DXVECTOR3(0.0f, 0.0f, m_CamPosZ),	//�J�����ʒu
			D3DXVECTOR3(0.0f, 0.0f, 0.0f), 		//���ړ_
			D3DXVECTOR3(0.0f, 1.0f, 0.0f)		//�J���������
		);

	//�J�����X�V
	result = m_Camera.Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//���S�X�V
	result = m_Logo.Transform(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTSceneTitle::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�X�V
	result = Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//���S�`��
	result = m_Logo.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �j��
//******************************************************************************
void MTSceneTitle::Release()
{
	m_Logo.Release();
}

