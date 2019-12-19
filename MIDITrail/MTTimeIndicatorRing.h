//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRing
//
// �^�C���C���W�P�[�^�����O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �u���Đ����Ă���Ƃ���v���w�������Đ��ʂ�`�悷��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


//******************************************************************************
// �^�C���C���W�P�[�^�����O�`��N���X
//******************************************************************************
class MTTimeIndicatorRing
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTTimeIndicatorRing(void);
	virtual ~MTTimeIndicatorRing(void);

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

	//���݈ʒu�擾
	float GetPos();

	//�ړ��x�N�g���擾
	D3DXVECTOR3 GetMoveVector();

private:

	DXPrimitive m_PrimitiveLine;
	float m_CurPos;
	MTNoteDesignRing m_NoteDesign;

	unsigned long m_CurTickTime;

	//���_�o�b�t�@�\����
	struct MTTIMEINDICATOR_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreatePrimitiveLine(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateVertexOfIndicatorLine(MTTIMEINDICATOR_VERTEX* pVertex, unsigned long* pIndex);

};


