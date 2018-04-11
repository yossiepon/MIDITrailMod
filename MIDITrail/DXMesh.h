//******************************************************************************
//
// MIDITrail / DXMesh
//
// ���b�V���`��N���X
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// ���b�V���`��N���X
//******************************************************************************
class DXMesh
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DXMesh(void);
	virtual ~DXMesh(void);

	//���\�[�X���
	void Release();

	//������
	int Initialize(LPDIRECT3DDEVICE9 pD3DDevice, TCHAR* pMeshFilePath);

	//�ړ�����
	void Transform(D3DXMATRIX worldMatrix);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	D3DXMATRIX m_WorldMatrix;
	LPD3DXMESH m_pMesh;
	unsigned long m_NumMaterials;
	D3DMATERIAL9* m_pMeshMaterials;
	LPDIRECT3DTEXTURE9* m_pMeshTextures;

	int _LoadMeshFile(
			LPDIRECT3DDEVICE9 pD3DDevice,
			TCHAR* pMeshFilePath
		);

	int _GetTextureFilePath(
			TCHAR* pMeshFilePath,
			TCHAR* pTextureFileName,
			TCHAR* pBuf,
			unsigned long bufSize
		);

};


