//******************************************************************************
//
// MIDITrail / MTGridBoxLive
//
// ���C�u���j�^�p�O���b�h�{�b�N�X�`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �O���b�h�{�b�N�X��`�悷��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXPrimitive.h"
#include "MTNoteDesign.h"


//******************************************************************************
// ���C�u���j�^�p�O���b�h�{�b�N�X�`��N���X
//******************************************************************************
class MTGridBoxLive
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTGridBoxLive(void);
	virtual ~MTGridBoxLive(void);
	
	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	
	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
	
	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//���
	void Release();
	
private:
	
	DXPrimitive m_Primitive;
	MTNoteDesign m_NoteDesign;
	bool m_isVisible;
	
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


