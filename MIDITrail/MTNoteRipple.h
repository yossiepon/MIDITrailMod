//******************************************************************************
//
// MIDITrail / MTNoteRipple
//
// �m�[�g�g��`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő�g��`�搔
#define MTNOTERIPPLE_MAX_RIPPLE_NUM  (100)

// TODO: �ő�g��`�搔���ςɂ���
//   ���O�ɃV�[�P���X�f�[�^�̍ő哯���������𒲍����Ă�����
//   �m�ۂ���o�b�t�@�T�C�Y��ύX�ł���
//   ����ł��o�b�t�@�T�C�Y�͏��������_�œ��I�ɕύX�\�ł���


//******************************************************************************
// �m�[�g�g��`��N���X
//******************************************************************************
class MTNoteRipple
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteRipple(void);
	virtual ~MTNoteRipple(void);

	//����
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�X�V
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	virtual void Release();

	//�m�[�gOFF�o�^
	void SetNoteOff(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo
		);

	//�m�[�gON�o�^
	void SetNoteOn(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo,
			unsigned char velocity
		);

	//���t�`�b�N�^�C���o�^
	void SetCurTickTime(unsigned long curTickTime);

	//���Z�b�g
	virtual void Reset();

	//�\���ݒ�
	void SetEnable(bool isEnable);

	//�X�L�b�v���
	void SetSkipStatus(bool isSkipping);

protected:

	//�m�[�g������ԍ\����
	struct NoteStatus {
		bool isActive;
		unsigned char portNo;
		unsigned char chNo;
		unsigned char noteNo;
		unsigned char velocity;
		unsigned long regTime;
	};

	//���_�o�b�t�@�\����
	struct MTNOTERIPPLE_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
		D3DXVECTOR2 t;	//�e�N�X�`���摜�ʒu
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

protected:

	//�`��n
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;

	//�Đ�����
	unsigned long m_CurTickTime;

	//�J����
	D3DXVECTOR3 m_CamVector;

	//�m�[�g�f�U�C��
	MTNoteDesign m_NoteDesign;

	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;

	//�m�[�g������ԏ��
	unsigned long m_ActiveNoteNum;

	//�\����
	bool m_isEnable;

	//�X�L�b�v���
	bool m_isSkipping;

	virtual int _CreateNoteStatus();
	virtual int _CreateVertex(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void _MakeMaterial(D3DMATERIAL9* pMaterial);
	virtual int _TransformRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//�`��n
	D3DMATERIAL9 m_Material;

	//�m�[�g������ԏ��
	NoteStatus* m_pNoteStatus;

	int _CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	int _SetVertexPosition(
				MTNOTERIPPLE_VERTEX* pVertex,
				NoteStatus* pNoteStatus,
				unsigned long rippleNo,
				unsigned long curTime,
				bool* pIsTimeout
			);
};


