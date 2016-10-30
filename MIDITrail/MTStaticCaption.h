//******************************************************************************
//
// MIDITrail / MTStaticCaption
//
// �ÓI�L���v�V�����`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �ÓI�ȕ�����̍����`�����������B
// ID3DXFont��GDI�𗘗p���Ă��邽�ߎg�p���Ȃ��B
// �\�����镶���̃e�N�X�`�����쐬���Ă����A�l�p�`�|���S���ɂ��̂܂ܓ\��t����B
// �ォ�當�����ύX���邱�Ƃ͂ł��Ȃ��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFontTexture.h"


//******************************************************************************
// �ÓI�L���v�V�����`��N���X
//******************************************************************************
class MTStaticCaption
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTStaticCaption(void);
	virtual ~MTStaticCaption(void);

	//����
	//  pFontName   �t�H���g����
	//  fontSize    �t�H���g�T�C�Y�i�|�C���g�j
	//  pCaption    �L���v�V����������
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			TCHAR* pFontName,
			unsigned long fontSize,
			TCHAR* pCaptin
		);
	
	//�e�N�X�`���T�C�Y�擾
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);

	//�F�ݒ�
	void SetColor(D3DXCOLOR color);

	//�`��
	//  �`��ʒu�͍��W�ϊ��ςݒ��_�Ƃ��Ĉ����B�E�B���h�E���オ(0,0)�B
	//  �e�N�X�`���T�C�Y���Q�Ƃ�����ŉ�ʕ\���{�����w�肷��B
	//  magRate=1.0 �Ȃ�e�N�X�`���T�C�Y�̂܂ܕ`�悳���B
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice, float x, float y, float magRate);

	//���\�[�X�j��
	void Release();

private:

	//���_�o�b�t�@�\����
	struct MTSTATICCAPTION_VERTEX {
		D3DXVECTOR3 p;		//���_���W
		float		rhw;	//���Z��
		DWORD		c;		//�f�B�t���[�Y�F
		D3DXVECTOR2 t;		//�e�N�X�`���摜�ʒu
	};

	//���_�o�b�t�@�[�̃t�H�[�}�b�g�̒�`�F���W�ϊ��ς݂��w��
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	MTFontTexture m_FontTexture;
	MTSTATICCAPTION_VERTEX* m_pVertex;
	D3DXCOLOR m_Color;

	int _CreateTexture(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pFontName,
			unsigned long fontSize,
			const TCHAR* pCaption
		);

	int _CreateVertex();

	void _SetVertexPosition(
			MTSTATICCAPTION_VERTEX* pVertex,
			float x,
			float y,
			float magRate
		);

	void _SetVertexColor(
			MTSTATICCAPTION_VERTEX* pVertex,
			D3DXCOLOR color
		);

};


