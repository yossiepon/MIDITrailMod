//******************************************************************************
//
// MIDITrail / MTGridBox
//
// �O���b�h�{�b�N�X�`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �O���b�h�{�b�N�X�Ə��ߐ���`�悷��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
//  �O���b�h�{�b�N�X�`��N���X
//******************************************************************************
class MTGridBox
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTGridBox(void);
	virtual ~MTGridBox(void);

	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

// >>> modify function to virtual 20180404 yossiepon begin
	//�X�V
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify function to virtual 20180404 yossiepon end

	//���
	void Release();

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	DXPrimitive m_Primitive;

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	unsigned long m_BarNum;
	SMPortList m_PortList;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	MTNoteDesign m_NoteDesign;

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	bool m_isVisible;

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

	int _CreateVertexOfBar(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pIbIndex,
			unsigned long vartexIndexOffset,
			SMBarList* pBarList
		);

	int _CreateVertexOfPortSplitLine(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pIndex,
			unsigned long vartexIndexOffset,
			unsigned long totalTickTime
		);

	void _MakeMaterial(D3DMATERIAL9* pMaterial);

};

