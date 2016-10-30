//******************************************************************************
//
// MIDITrail / MTDynamicCaption
//
// ���I�L���v�V�����`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ���I�ȕ�����̍����`�����������B
// ID3DXFont��GDI�𗘗p���Ă��邽�ߎg�p���Ȃ��B
// �\�����镶���̃e�N�X�`�����쐬���Ă����i��F"ABCD...0123..."�j�A
// �^�C����ɕ��Ԏl�p�`�|���S���ɓ\��t����B
// ������̕ύX�́A���_�f�[�^�̃e�N�X�`��UV���W���X�V���邾���B
// ���̂��߁A�ȉ��̐���������B
// (1) ���炩���ߎw�肵�����������g�p�ł��Ȃ��B  
//     ���e�N�X�`���摜�Œ�̂���
// (2) ���炩���ߎw�肵�������������`��ł��Ȃ��B
//     ���|���S�����Œ�̂���
// (3) �Œ�s�b�`�t�H���g�����g�p�ł��Ȃ��B
//     ���e�N�X�`���摜�̕����ʒu����肷��̂�����Ȃ̂�
//
//   +-----------+
//   |A B C ... Z| �e�N�X�`���摜
//   +-----------+
//   +-+-+-+-+
//   |N|E|K|O| �|���S�����1�������\��t����ꂽ�e�N�X�`��
//   +-+-+-+-+

// BUG:
// �V���O���o�C�g�������������Ȃ��B
// �e�N�X�`���r�b�g�}�b�v�̉�����4�̔{���ɂȂ�悤�ɕ␳�����B
// ���̂���1�����̐؂�o���Ō덷��������ꍇ������B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFontTexture.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�L���v�V�����ő啶����
#define MTDYNAMICCAPTION_MAX_CHARS  (256)

//******************************************************************************
// �t�H���g�^�C���`��N���X
//******************************************************************************
class MTDynamicCaption
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTDynamicCaption(void);
	virtual ~MTDynamicCaption(void);

	//����
	//  pFontName   �t�H���g����
	//  fontSize    �t�H���g�T�C�Y�i�|�C���g�j
	//  pCharacters �C�ӕ������w�肷��i�ő�255�����j��F"0123456789"
	//  captionSize �L���v�V����������
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pFontName,
			unsigned long fontSize,
			const TCHAR* pCharacters,
			unsigned long captionSize
		);

	//�e�N�X�`���T�C�Y�擾
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);

	//������ݒ�
	//  Create�Ŏw�肵�Ă��Ȃ������͕`�悳��Ȃ�
	//  Create�Ŏw�肵���L���v�V�����������𒴂��������͕`�悵�Ȃ�
	int SetString(TCHAR* pStr);

	//�F�ݒ�
	void SetColor(D3DXCOLOR color);

	//�`��
	//  �`��ʒu�͍��W�ϊ��ςݒ��_�Ƃ��Ĉ����F�E�B���h�E���オ(0,0)
	//  �e�N�X�`���T�C�Y���Q�Ƃ�����ŉ�ʕ\���{�����w�肷��
	//  magRate=1.0 �Ȃ�e�N�X�`���T�C�Y�̂܂ܕ`�悷��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice, float x, float y, float magRate);

	//���\�[�X�j��
	void Release();

private:

	//���_�o�b�t�@�\����
	struct MTDYNAMICCAPTION_VERTEX {
		D3DXVECTOR3 p;		//���_���W
		float		rhw;	//���Z��
		DWORD		c;		//�f�B�t���[�Y�F
		D3DXVECTOR2	t;		//�e�N�X�`���摜�ʒu
	};

	//���_�o�b�t�@�[�̃t�H�[�}�b�g�̒�`�F���W�ϊ��ς݂��w��
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	TCHAR m_Chars[MTDYNAMICCAPTION_MAX_CHARS];
	unsigned long m_CaptionSize;
	MTFontTexture m_FontTexture;
	MTDYNAMICCAPTION_VERTEX* m_pVertex;
	D3DXCOLOR m_Color;

	int _CreateTexture(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pFontName,
			unsigned long fontSize,
			const TCHAR* pCharacters
		);

	int _CreateVertex();

	int _GetTextureUV(
			TCHAR target,
			D3DXVECTOR2* pV0,
			D3DXVECTOR2* pV1,
			D3DXVECTOR2* pV2,
			D3DXVECTOR2* pV3
		);

	void _SetVertexPosition(
			MTDYNAMICCAPTION_VERTEX* pVertex,
			float x,
			float y,
			float magRate
		);

	void _SetVertexColor(
			MTDYNAMICCAPTION_VERTEX* pVertex,
			D3DXCOLOR color
		);

};


