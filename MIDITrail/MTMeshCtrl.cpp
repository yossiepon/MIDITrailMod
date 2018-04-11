//******************************************************************************
//
// MIDITrail / MTMeshCtrl
//
// ���b�V������N���X
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTConfFile.h"
#include "MTMeshCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTMeshCtrl::MTMeshCtrl(void)
{
	m_MeshFilePath[0] = _T('\0');
	m_PositionX = 0.0f;
	m_PositionY = 0.0f;
	m_PositionZ = 0.0f;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTMeshCtrl::~MTMeshCtrl(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTMeshCtrl::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;

	//�p�����[�^�ݒ�t�@�C���ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = m_Mesh.Initialize(pD3DDevice, m_MeshFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTMeshCtrl::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 moveVector
	)
{
	int result = 0;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;
	float rollAngle = 0.0f;

	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��
	D3DXMatrixRotationY(&rotateMatrix, D3DXToRadian(rollAngle));

	//�ړ��s��
	D3DXMatrixTranslation(
			&moveMatrix,
			m_PositionX + moveVector.x,
			m_PositionY + moveVector.y,
			m_PositionZ + moveVector.z
		);

	//�s��̍����F�ړ�����]
	D3DXMatrixMultiply(&worldMatrix, &moveMatrix, &rotateMatrix);

	//�ϊ��s��ݒ�
	m_Mesh.Transform(worldMatrix);

//EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTMeshCtrl::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//���b�V���̕`��
	result = m_Mesh.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTMeshCtrl::Release()
{
	m_Mesh.Release();
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTMeshCtrl::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;
	TCHAR xFileName[_MAX_PATH] = {_T('\0')};
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};

	m_MeshFilePath[0] = _T('\0');

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Mesh"));
	if (result != 0) goto EXIT;

	//X�t�@�C����
	result = confFile.GetStr(_T("XFile"), xFileName, _MAX_PATH, _T(""));
	if (result != 0) goto EXIT;

	//X�t�@�C���p�X�쐬
	if (_tcslen(xFileName) > 0) {
		//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
		result = YNPathUtil::GetModuleDirPath(m_MeshFilePath, _MAX_PATH);
		if (result != 0) goto EXIT;
		//���b�V���t�@�C���p�X
		_tcscat_s(m_MeshFilePath, _MAX_PATH, xFileName);
	}

	//�`��ʒu
	result = confFile.GetFloat(_T("PositionX"), &m_PositionX, 0.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("PositionY"), &m_PositionY, 0.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("PositionZ"), &m_PositionZ, 0.0f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


