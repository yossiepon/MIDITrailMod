//******************************************************************************
//
// MIDITrail / MTPictBoard
//
// �s�N�`���{�[�h�`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �s�N�`���{�[�h��`�悷��B
// �摜�t�@�C���� .bmp .dds .dib .jpg .png .tga ���w��\�B
// �iD3DXCreateTextureFromFile ���T�|�[�g���Ă���摜�j

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
//  �s�N�`���{�[�h�`��N���X
//******************************************************************************
class MTPictBoard
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPictBoard(void);
	virtual ~MTPictBoard(void);

	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	void Release();

	//���t�`�b�N�^�C���o�^
	void SetCurTickTime(unsigned long curTickTime);

	//���Z�b�g
	void Reset();

	//���t�J�n�I��
	void OnPlayStart();
	void OnPlayEnd();

	//�\���ݒ�
	void SetEnable(bool isEnable);

private:

	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DXIMAGE_INFO m_ImgInfo;
	unsigned long m_CurTickTime;
	bool m_isPlay;
	bool m_isEnable;
	MTNoteDesign m_NoteDesign;

	//���_�o�b�t�@�\����
	struct MTPICTBOARD_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
		D3DXVECTOR2 t;	//�e�N�X�`���摜�ʒu
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	int _CreateVertexOfBoard(
			MTPICTBOARD_VERTEX* pVertex,
			unsigned long* pIbIndex
		);

	int _LoadTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);

};

