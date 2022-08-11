//******************************************************************************
//
// MIDITrail / MTGridRing
//
// �O���b�h�����O�`��N���X
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


//******************************************************************************
//  �O���b�h�����O�`��N���X
//******************************************************************************
class MTGridRing
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTGridRing(void);
	virtual ~MTGridRing(void);

	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

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
	unsigned long m_BarNum;
	SMPortList m_PortList;
	MTNoteDesignRing m_NoteDesign;
	bool m_isVisible;
	bool m_isEnable;

	//���_�o�b�t�@�\����
	struct MTGRIDBOX_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateVertexOfGrid(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pIbIndex,
			unsigned long totalTickTime
		);

	void _MakeMaterial(D3DMATERIAL9* pMaterial);

};

