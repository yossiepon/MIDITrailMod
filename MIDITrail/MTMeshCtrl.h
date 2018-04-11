//******************************************************************************
//
// MIDITrail / MTMeshCtrl
//
// ���b�V������N���X
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXMesh.h"


//******************************************************************************
// ���b�V������N���X
//******************************************************************************
class MTMeshCtrl
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTMeshCtrl(void);
	virtual ~MTMeshCtrl(void);

	//����
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName
		);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 moveVector);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	void Release();

private:

	DXMesh m_Mesh;
	TCHAR m_MeshFilePath[_MAX_PATH];
	float m_PositionX;
	float m_PositionY;
	float m_PositionZ;

	int _LoadConfFile(const TCHAR* pSceneName);

};


