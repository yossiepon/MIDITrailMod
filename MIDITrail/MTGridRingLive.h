//******************************************************************************
//
// MIDITrail / MTGridRingLive
//
// ���C�u���j�^�p�O���b�h�����O�`��N���X
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXPrimitive.h"
#include "MTNoteDesignRing.h"


//******************************************************************************
// ���C�u���j�^�p�O���b�h�����O�`��N���X
//******************************************************************************
class MTGridRingLive
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTGridRingLive(void);
	virtual ~MTGridRingLive(void);
	
	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	
	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
	
	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//���
	void Release();
	
	//�\���ݒ�
	void SetEnable(bool isEnable);
	
private:
	
	DXPrimitive m_Primitive;
	MTNoteDesignRing m_NoteDesign;
	bool m_isVisible;
	bool m_isEnable;
	
	//���_�o�b�t�@�\����
	struct MTGRIDBOXLIVE_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
	};
	
	//���_�o�b�t�@FVF�t�H�[�}�b�g
	unsigned long _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }
	
	int _CreateVertexOfGrid(
			MTGRIDBOXLIVE_VERTEX* pVertex,
			unsigned long* pIbIndex
		);
	
	void _MakeMaterial(D3DMATERIAL9* pMaterial);

};


