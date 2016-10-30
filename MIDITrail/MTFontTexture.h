//******************************************************************************
//
// MIDITrail / MTFontTexture
//
// �t�H���g�e�N�X�`���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �t�H���g���r�b�g�}�b�v�ϊ��N���X�𗘗p���āA
// ������r�b�g�}�b�v����e�N�X�`�����쐬����B
// 1�s�̕�����ɂ̂ݑΉ�����B�����s�͑Ή����Ă��Ȃ��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFont2Bmp.h"


//******************************************************************************
//  �t�H���g�e�N�X�`���N���X
//******************************************************************************
class MTFontTexture
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTFontTexture(void);
	virtual ~MTFontTexture(void);

	//�N���A
	void Clear();

	//�t�H���g�ݒ�
	int SetFont(
			const TCHAR* pFontName,
			unsigned long fontSize,
			unsigned long rgb,
			bool isForceFixedPitch = false
		);

	//�e�N�X�`������
	int CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pStr);

	//�e�N�X�`���C���^�[�t�F�[�X�|�C���^�Q��
	//  �e�N�X�`���I�u�W�F�N�g�͖{�N���X�ŊǗ����邽�߁A
	//  �e�N�X�`�����g�p���Ă�����Ԃ͖{�N���X�̃C���X�^���X��j�����Ă͂Ȃ�Ȃ��B
	LPDIRECT3DTEXTURE9 GetTexture();

	//�e�N�X�`���T�C�Y�擾
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);
	
private:

	LPDIRECT3DTEXTURE9 m_pTexture;
	MTFont2Bmp m_Font2Bmp;
	unsigned long m_RGB;

	unsigned long m_TexHeight;
	unsigned long m_TexWidth;

};

