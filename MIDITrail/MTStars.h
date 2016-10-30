//******************************************************************************
//
// MIDITrail / MTStars
//
// ���`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ���������_���ɔz�u���ĕ`�悷��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "DXPrimitive.h"


//******************************************************************************
// ���`��N���X
//******************************************************************************
class MTStars
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTStars(void);
	virtual ~MTStars(void);

	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, DXDirLight* pLight);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�j��
	void Release();

	//�\���ݒ�
	void SetEnable(bool isEnable);

private:

	int m_NumOfStars;
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;

	//�\����
	bool m_isEnable;

	//���_�o�b�t�@�\����
	struct MTSTARS_VERTEX {
		D3DXVECTOR3 p;		//���_���W
		D3DXVECTOR3 n;		//�@��
		DWORD		c;		//�f�B�t���[�Y�F
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateVertexOfStars(MTSTARS_VERTEX* pVertex, DXDirLight* pLight);
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	int _LoadConfFile(const TCHAR* pSceneName);

};


