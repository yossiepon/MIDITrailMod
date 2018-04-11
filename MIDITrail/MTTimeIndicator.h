//******************************************************************************
//
// MIDITrail / MTTimeIndicator
//
// �^�C���C���W�P�[�^�`��N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �u���Đ����Ă���Ƃ���v���w�������Đ��ʂ�`�悷��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// �^�C���C���W�P�[�^�`��N���X
//******************************************************************************
class MTTimeIndicator
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTTimeIndicator(void);
	virtual ~MTTimeIndicator(void);

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

// >>> add 20180404 yossiepon begin
	//�\���ݒ�
	void SetEnable(bool isEnable);
// <<< add 20180404 yossiepon end

private:

	DXPrimitive m_Primitive;
	DXPrimitive m_PrimitiveLine;
	float m_CurPos;
	MTNoteDesign m_NoteDesign;
	bool m_isEnableLine;

// >>> add 20180404 yossiepon begin
	//�\����
	bool m_isEnable;
// <<< add 20180404 yossiepon end

	unsigned long m_CurTickTime;

	//���_�o�b�t�@�\����
	struct MTTIMEINDICATOR_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreatePrimitive(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreatePrimitiveLine(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateVertexOfIndicator(MTTIMEINDICATOR_VERTEX* pVertex, unsigned long* pIbIndex);
	int _CreateVertexOfIndicatorLine(MTTIMEINDICATOR_VERTEX* pVertex);

};


