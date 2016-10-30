//******************************************************************************
//
// MIDITrail / MTLogo
//
// MIDITrail ���S�`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFontTexture.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�^�C�g��������
#define MTLOGO_TITLE  _T("MIDITrail")

//���S�`��ʒu���
#define MTLOGO_POS_X  (20.0f)   //�`��ʒux
#define MTLOGO_POS_Y  (-15.0f)  //�`��ʒuy
#define MTLOGO_MAG    (0.1f)    //�g�嗦

//�^�C��������
#define MTLOGO_TILE_NUM  (40)

//�O���f�[�V�������ԊԊu(msec)
#define MTLOGO_GRADATION_TIME  (1000)


//******************************************************************************
// MIDITrail ���S�`��N���X
//******************************************************************************
class MTLogo
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^l
	MTLogo(void);
	virtual ~MTLogo(void);

	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice);

	//�ϊ�
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�j��
	void Release();

private:

	//���_�o�b�t�@�\����
	struct MTLOGO_VERTEX {
		D3DXVECTOR3 p;		//���_���W
		D3DXVECTOR3 n;		//�@��
		DWORD		c;		//�f�B�t���[�Y�F
		D3DXVECTOR2	t;		//�e�N�X�`���摜�ʒu
	};

	//���_�o�b�t�@�[�̃t�H�[�}�b�g�̒�`�F���W�ϊ��ς݂��w��
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

private:

	//�t�H���g�e�N�X�`��
	MTFontTexture m_FontTexture;
	MTLOGO_VERTEX* m_pVertex;

	unsigned long m_StartTime;
	unsigned long m_GradationTime;

	int _CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice);

	int _CreateVertex();

	int _GetTextureUV(
			TCHAR target,
			D3DXVECTOR2* pV0,
			D3DXVECTOR2* pV1,
			D3DXVECTOR2* pV2,
			D3DXVECTOR2* pV3
		);

	void _SetVertexPosition(
			MTLOGO_VERTEX* pVertex,
			float x,
			float y,
			float magRate
		);

	void _SetGradationColor();

	void _SetTileColor(
			MTLOGO_VERTEX* pVertex,
			float color
		);

};

