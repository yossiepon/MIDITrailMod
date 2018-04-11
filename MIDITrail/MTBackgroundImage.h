//******************************************************************************
//
// MIDITrail / MTBackgroundImage
//
// �w�i�摜�`��N���X
//
// Copyright (C) 2016 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �w�i�摜��`�悷��B
// �摜�t�@�C���� .bmp .dds .dib .jpg .png .tga ���w��\�B
// �iD3DXCreateTextureFromFile ���T�|�[�g���Ă���摜�j

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "YNBaseLib.h"
#include "DXPrimitive.h"

using namespace YNBaseLib;


//******************************************************************************
//  �w�i�摜�`��N���X
//******************************************************************************
class MTBackgroundImage
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTBackgroundImage(void);
	virtual ~MTBackgroundImage(void);

	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, HWND hWnd);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	void Release();

	//���Z�b�g
	void Reset();

	//�\���ݒ�
	void SetEnable(bool isEnable);

private:

	HWND m_hWnd;
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DXIMAGE_INFO m_ImgInfo;
	bool m_isEnable;
	bool m_isFilterLinear;

	//�ݒ�t�@�C��
	YNConfFile m_ConfFile;

	//���_�o�b�t�@�\����
	struct MTBACKGROUNDIMAGE_VERTEX {
		D3DXVECTOR3 p;		//���_���W
		float		rhw;	//���Z��
		DWORD		c;		//�f�B�t���[�Y�F
		D3DXVECTOR2 t;		//�e�N�X�`���摜�ʒu
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	//���_����
	int _CreateVertexOfBackground(
			MTBACKGROUNDIMAGE_VERTEX* pVertex,
			unsigned long* pIbIndex
		);

	//�ݒ�t�@�C��������
	int _InitConfFile();

	//�e�N�X�`���摜�ǂݍ���
	int _LoadTexture(LPDIRECT3DDEVICE9 pD3DDevice);

};

