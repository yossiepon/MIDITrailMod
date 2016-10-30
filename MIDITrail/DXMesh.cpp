//******************************************************************************
//
// MIDITrail / DXMesh
//
// ���b�V���`��N���X
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXMesh.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
DXMesh::DXMesh(void)
{
	m_pMesh = NULL;
	m_NumMaterials = 0;
	m_pMeshMaterials = NULL;
	m_pMeshTextures = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
DXMesh::~DXMesh(void)
{
	Release();
}

//******************************************************************************
// ���
//******************************************************************************
void DXMesh::Release()
{
	unsigned long i = 0;

	for (i = 0; i < m_NumMaterials; i++) {
		if (m_pMeshTextures[i] != NULL) {
			(m_pMeshTextures[i])->Release();
			m_pMeshTextures[i] = NULL;
		}
	}
	delete [] m_pMeshMaterials;
	delete [] m_pMeshTextures;
	m_pMeshMaterials = NULL;
	m_pMeshTextures = NULL;
	m_NumMaterials = 0;

	if (m_pMesh != NULL) {
		m_pMesh->Release();
		m_pMesh = NULL;
	}

	return;
}

//******************************************************************************
// ������
//******************************************************************************
int DXMesh::Initialize(
		LPDIRECT3DDEVICE9 pD3DDevice,
		TCHAR* pMeshFilePath
	)
{
	int result = 0;

	//�t�@�C�����w��Ȃ牽�����Ȃ�
	if (_tcslen(pMeshFilePath) == 0) goto EXIT;

	//���b�V���t�@�C���ǂݍ���
	result = _LoadMeshFile(pD3DDevice, pMeshFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���b�V���t�@�C���ǂݍ���
//******************************************************************************
int DXMesh::_LoadMeshFile(
		LPDIRECT3DDEVICE9 pD3DDevice,
		TCHAR* pMeshFilePath
	)
{
	int result = 0;
	unsigned long i = 0;
	HRESULT hresult = D3D_OK;
	LPD3DXBUFFER pMaterialBuffer = NULL;
	TCHAR textureFilePath[_MAX_PATH];
	D3DXMATERIAL* pMaterials = NULL;

	//���b�V���t�@�C���ǂݍ���
	hresult = D3DXLoadMeshFromX(
					pMeshFilePath,		//���b�V���t�@�C���p�X
					D3DXMESH_MANAGED,	//���b�V���쐬�I�v�V����
					pD3DDevice,			//�f�o�C�X�I�u�W�F�N�g
					NULL,				//�אڐ��f�[�^
					&pMaterialBuffer,	//�}�e���A��
					NULL,				//�G�t�F�N�g�C���X�^���X
					&m_NumMaterials,	//�}�e���A����
					&m_pMesh			//���b�V��
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�}�e���A���擪�ʒu
	pMaterials = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();

	//�z��o�b�t�@����
	try {
		m_pMeshMaterials = new D3DMATERIAL9[m_NumMaterials];
		m_pMeshTextures = new LPDIRECT3DTEXTURE9[m_NumMaterials];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�o�b�t�@�N���A
	for (i = 0; i < m_NumMaterials; i++) {
		m_pMeshTextures[i] = NULL;
	}

	//�e�N�X�`���t�@�C���ǂݍ���
	for (i = 0; i < m_NumMaterials; i++) {
		m_pMeshMaterials[i] = pMaterials[i].MatD3D;
		//m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;
		
		if (pMaterials[i].pTextureFilename == NULL) continue;
		
		//�e�N�X�`���t�@�C���p�X�쐬
		result = _GetTextureFilePath(
						pMeshFilePath,
						pMaterials[i].pTextureFilename,
						textureFilePath,
						_MAX_PATH
					);
		if (result != 0) goto EXIT;
		
		//�e�N�X�`���t�@�C���ǂݍ���
		hresult = D3DXCreateTextureFromFile(
						pD3DDevice,
						textureFilePath,
						&(m_pMeshTextures[i])
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	if (pMaterialBuffer != NULL) {
		pMaterialBuffer->Release();
	}
	return result;
}

//******************************************************************************
// �e�N�X�`���t�@�C���p�X�擾
//******************************************************************************
int DXMesh::_GetTextureFilePath(
		TCHAR* pMeshFilePath,
		TCHAR* pTextureFileName,
		TCHAR* pBuf,
		unsigned long bufSize
	)
{
	int result = 0;
	DWORD apiresult = 0;
	errno_t eresult = 0;
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	//�p�X�v�f�̕���
	eresult = _tsplitpath_s(
					pMeshFilePath,	//���b�V���t�@�C���p�X
					drive,			//�h���C�u������o�b�t�@
					_MAX_DRIVE,		//�o�b�t�@�T�C�Y
					dir,			//�f�B���N�g��������o�b�t�@
					_MAX_DIR,		//�o�b�t�@�T�C�Y
					fname,			//�t�@�C����������o�b�t�@
					_MAX_FNAME,		//�o�b�t�@�T�C�Y
					ext,			//�g���q������o�b�t�@
					_MAX_EXT		//�o�b�t�@�T�C�Y
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�p�X�쐬
	eresult = _tmakepath_s(
					pBuf,			//�p�X�i�[��o�b�t�@
					bufSize,		//�o�b�t�@�T�C�Y
					drive,			//�h���C�u������
					dir,			//�f�B���N�g��������
					pTextureFileName,	//�t�@�C����������
					NULL			//�g���q������
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
void DXMesh::Transform(
		D3DXMATRIX worldMatrix
	)
{
	m_WorldMatrix = worldMatrix;
}

//******************************************************************************
// �`��
//******************************************************************************
int DXMesh::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long i = 0;

	//���b�V����ǂݍ���ł��Ȃ���Ή������Ȃ�
	if (m_pMesh == NULL) goto EXIT;

	//�ړ��}�g���N�X���Z�b�g
	hresult = pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// �A���t�@���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//�e�N�X�`���t�B���^
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//���b�V���`��
	for (i = 0; i < m_NumMaterials; i++) {
		//�}�e���A���ݒ�
		hresult = pD3DDevice->SetMaterial(&(m_pMeshMaterials[i]));
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
		
		//�e�N�X�`���ݒ�F�X�e�[�W0
		hresult = pD3DDevice->SetTexture(0, m_pMeshTextures[i]);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
		
		//���b�V���T�u�Z�b�g�`��
		hresult = m_pMesh->DrawSubset(i);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}


