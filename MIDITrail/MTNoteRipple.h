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
// >>> modify 20120728 yossiepon begin
	virtual int Create(
// <<< modify 20120728 yossiepon end
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

	//�`��
// >>> modify 20120728 yossiepon begin
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify 20120728 yossiepon end

	//���
// >>> modify 20120728 yossiepon begin
	virtual void Release();
// <<< modify 20120728 yossiepon end

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
// >>> modify 20120728 yossiepon begin
	virtual void Reset();
// <<< modify 20120728 yossiepon end

	//�\���ݒ�
	void SetEnable(bool isEnable);

	//�X�L�b�v���
	void SetSkipStatus(bool isSkipping);

// >>> modify access level to protected 20120728 yossiepon begin
protected:
// >>> modify 20120728 yossiepon end

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

// >>> modify access level to protected 20161224 yossiepon begin
// >>> modify 20161224 yossiepon end

	//�`��n
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;
// >>> modify access level 20161224 yossiepon begin
private:
// >>> modify 20161224 yossiepon end

	D3DMATERIAL9 m_Material;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// >>> modify 20161224 yossiepon end

	//�Đ�����
	unsigned long m_CurTickTime;

	//�J����
	D3DXVECTOR3 m_CamVector;

	//�m�[�g�f�U�C��
	MTNoteDesign m_NoteDesign;

	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;

	//�m�[�g������ԏ��

// >>> modify access level 20161224 yossiepon begin
private:
// >>> modify 20161224 yossiepon end

	NoteStatus* m_pNoteStatus;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// >>> modify 20161224 yossiepon end

	unsigned long m_ActiveNoteNum;

	//�\����
	bool m_isEnable;

	//�X�L�b�v���
	bool m_isSkipping;

// >>> modify access level 20161224 yossiepon begin
private:
// >>> modify 20161224 yossiepon end

	int _CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// >>> modify 20161224 yossiepon end

	virtual int _CreateNoteStatus();
	virtual int _CreateVertex(LPDIRECT3DDEVICE9 pD3DDevice);

// >>> modify access level 20161224 yossiepon begin
private:
// >>> modify 20161224 yossiepon end

	int _SetVertexPosition(
				MTNOTERIPPLE_VERTEX* pVertex,
				NoteStatus* pNoteStatus,
				unsigned long rippleNo,
				unsigned long curTime,
				bool* pIsTimeout
			);

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// >>> modify 20161224 yossiepon end

	virtual void _MakeMaterial(D3DMATERIAL9* pMaterial);
	virtual int _TransformRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);

};


