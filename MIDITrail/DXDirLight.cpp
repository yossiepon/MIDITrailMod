//******************************************************************************
//
// MIDITrail / DXDirLight
//
// �f�B���N�V���i�����C�g�N���X
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXDirLight.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
DXDirLight::DXDirLight(void)
{
	ZeroMemory(&m_Light, sizeof(D3DLIGHT9));
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
DXDirLight::~DXDirLight(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int DXDirLight::Initialize()
{
	int result = 0;

	ZeroMemory(&m_Light, sizeof(D3DLIGHT9));

	//���C�g�^�C�v�F���s����
	m_Light.Type = D3DLIGHT_DIRECTIONAL;

	//�g�U��
	m_Light.Diffuse.r  = 1.0f;
	m_Light.Diffuse.g  = 1.0f;
	m_Light.Diffuse.b  = 1.0f;
	m_Light.Diffuse.a  = 1.0f;

	//���ʔ��ˌ�
	m_Light.Specular.r = 0.0f;
	m_Light.Specular.g = 0.0f;
	m_Light.Specular.b = 0.0f;
	m_Light.Specular.a = 0.0f;

	//����
	m_Light.Ambient.r  = 0.2f;
	m_Light.Ambient.g  = 0.2f;
	m_Light.Ambient.b  = 0.2f;
	m_Light.Ambient.a  = 1.0f;

	//�����F�x�N�g���͐��K������Ă��Ȃ���΂Ȃ�Ȃ�
	m_Light.Direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	return result;
}

//******************************************************************************
// ���C�g�F�ݒ�
//******************************************************************************
void DXDirLight::SetColor(
		D3DXCOLOR diffuse,
		D3DXCOLOR specular,
		D3DXCOLOR ambient
	)
{
	m_Light.Diffuse  = diffuse;
	m_Light.Specular = specular;
	m_Light.Ambient  = ambient;
}

//******************************************************************************
// ���C�g�����ݒ�
//******************************************************************************
void DXDirLight::SetDirection(
		D3DXVECTOR3 dirVector
	)
{
	D3DXVECTOR3 normalizedVector;

	//�x�N�g�����K��
	D3DXVec3Normalize(&normalizedVector, &dirVector);

	//���C�g���\���̂ɓo�^
	m_Light.Direction = normalizedVector;
}

//******************************************************************************
// ���C�g�����擾
//******************************************************************************
D3DXVECTOR3 DXDirLight::GetDirection()
{
	return m_Light.Direction;
}

//******************************************************************************
// �f�o�C�X�o�^�F�C���f�b�N�X0
//******************************************************************************
int DXDirLight::SetDevice(
		LPDIRECT3DDEVICE9 pD3DDevice,
		BOOL isLightON
	)
{
	return SetDevice(pD3DDevice, 0, isLightON);
}

//******************************************************************************
// �f�o�C�X�o�^�F�C���f�b�N�X�w��
//******************************************************************************
int DXDirLight::SetDevice(
		LPDIRECT3DDEVICE9 pD3DDevice,
		DWORD index,
		BOOL isLightON
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	//���C�e�B���O���[�h
	hresult = pD3DDevice->SetRenderState(D3DRS_LIGHTING, isLightON);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, isLightON);
		goto EXIT;
	}

	//�X�y�L������
	//  �X�y�L������L���ɂ���ƒʏ�̃��C�g�ɔ�ׂ�2�{�̕��ׂ������邽�ߖ����ɂ���
	//  TODO: �O������ݒ�ł���悤�ɂ���
	hresult = pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, isLightON);
		goto EXIT;
	}

	// ���C�g�������_�����O�p�C�v���C���ɐݒ�
	hresult = pD3DDevice->SetLight(index, &m_Light);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//���C�g�L����
	hresult = pD3DDevice->LightEnable(index, isLightON);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, isLightON);
		goto EXIT;
	}

EXIT:;
	return result;
}

