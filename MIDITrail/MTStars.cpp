//******************************************************************************
//
// MIDITrail / MTStars
//
// ���`��N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTStars.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTStars::MTStars(void)
{
	m_pTexture = NULL;
	m_NumOfStars = 2000;
	m_isEnable = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTStars::~MTStars(void)
{
	Release();
}

//******************************************************************************
// ������
//******************************************************************************
int MTStars::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		DXDirLight* pLight
	)
{
	int result = 0;
	MTSTARS_VERTEX* pVertex = NULL;
	D3DMATERIAL9 material;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�p�����[�^�ݒ�t�@�C���ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTSTARS_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),		//���_FVF�t�H�[�}�b�g
					D3DPT_POINTLIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, m_NumOfStars);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_����������
	result = _CreateVertexOfStars(pVertex, pLight);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

	//�}�e���A���쐬
	_MakeMaterial(&material);
	m_Primitive.SetMaterial(material);

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTStars::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector
	)
{
	int result = 0;
	D3DXMATRIX worldMatrix;
	
	//�������E�̐��͔��ɉ����ɑ��݂���̂�
	//����������ς����ɃJ�������ړ����Ă����͓����Ȃ��悤�Ɍ�����
	//������[���I�ɍČ����邽�߃J�����ɍ��킹�Đ������������ړ�������
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixTranslation(&worldMatrix, camVector.x, camVector.y, camVector.z);
	m_Primitive.Transform(worldMatrix);
	
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTStars::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	result = m_Primitive.Draw(pD3DDevice, m_pTexture);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTStars::Release()
{
	m_Primitive.Release();
	
	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

//******************************************************************************
// �����_����
//******************************************************************************
int MTStars::_CreateVertexOfStars(
		MTSTARS_VERTEX* pVertex,
		DXDirLight* pLight
	)
{
	int result = 0;
	float r = 0.0f;
	float phi = 0.0f;
	//float theta = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float cr = 0.0f;
	float cg = 0.0f;
	float cb = 0.0f;
	int i = 0;
	D3DXVECTOR3 normal;

	//���C�g�̕����ɂ���Đ��̐F���ς�邱�Ƃ�h������
	//�@���x�N�g�������C�g�̕����x�N�g���̋t�����ɂ���
	normal = -(pLight->GetDirection());

	//���_���W
	for (i = 0; i < m_NumOfStars; i++) {

		//�ɍ��W(theta,phi)�ɗ�����K�p����Ɩk�ɂƓ�ɂŐ��̕��z���x�������Ȃ��Ă��܂�
		//
		//�ɍ��W�F���ʏ�ɐ���z�u
		//r     = 500.0f;
		//phi   = ((float)rand() / RAND_MAX) * 360.0f;
		//theta = ((float)rand() / RAND_MAX) * 180.0f;
		//���s���W�ɕϊ�
		//x = r * sin(D3DXToRadian(theta)) * cos(D3DXToRadian(phi));
		//y = r * cos(D3DXToRadian(theta));
		//z = r * sin(D3DXToRadian(theta)) * sin(D3DXToRadian(phi));

		//���ʏ�ɐ�����l���z�����邽��(y, phi)�ɗ�����K�p����
		r   = 500.0f;
		phi = ((float)rand() / RAND_MAX) * 2.0f * 3.1415926f;
		y   = ((float)rand() / RAND_MAX) * 2.0f * r - r;
		x = sqrt((r * r) - (y * y)) * cos(phi);
		z = sqrt((r * r) - (y * y)) * sin(phi);

		//�F�F�O���[�X�P�[���ɂ���ꍇ
		cr = ((float)rand() / RAND_MAX);
		cg = cr;
		cb = cr;
		//�F�F�J���t���ɂ���ꍇ
		//cr = ((float)rand() / RAND_MAX);
		//cg = ((float)rand() / RAND_MAX);
		//cb = ((float)rand() / RAND_MAX);

		//���_���W
		pVertex[i].p = D3DXVECTOR3(x, y, z);
		//�@��
		pVertex[i].n = normal;
		//�f�B�t���[�Y�F
		pVertex[i].c = D3DXCOLOR(cr, cg, cb, 1.0f);
	}

	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTStars::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));

	//�g�U��
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//�����F�e�̐F
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//���ʔ��ˌ�
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//���ʔ��ˌ��̑N���x
	pMaterial->Power = 100.0f;
	//�����F
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTStars::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//���̐�
	result = confFile.SetCurSection(_T("Stars"));
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("NumberOfStars"), &m_NumOfStars, 2000);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTStars::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


