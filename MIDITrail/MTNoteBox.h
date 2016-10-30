//******************************************************************************
//
// MIDITrail / MTNoteBox
//
// �m�[�g�{�b�N�X�`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �m�[�g�{�b�N�X��`�悷��B

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
//�ő唭���m�[�g�`�搔
#define MTNOTEBOX_MAX_ACTIVENOTE_NUM  (100)

// TODO: �ő唭���m�[�g�`�搔���ςɂ���
//   ���O�ɃV�[�P���X�f�[�^�̍ő哯���������𒲍����Ă�����
//   �m�ۂ���o�b�t�@�T�C�Y��ύX�ł���
//   ����ł��o�b�t�@�T�C�Y�͏��������_�œ��I�ɕύX�\�ł���


//******************************************************************************
// �m�[�g�{�b�N�X�`��N���X
//******************************************************************************
class MTNoteBox
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteBox(void);
	virtual ~MTNoteBox(void);

	//����
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�X�V
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	virtual void Release();

	//���t�`�b�N�^�C���o�^
	void SetCurTickTime(unsigned long curTickTime);

	//���Z�b�g
	virtual void Reset();

	//�X�L�b�v���
	void SetSkipStatus(bool isSkipping);

protected:

	//���_�o�b�t�@�\����
	struct MTNOTEBOX_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
	};

protected:

	//�m�[�g�f�U�C��
	MTNoteDesign m_NoteDesign;

	//�m�[�g���X�g
	SMNoteList m_NoteList;

	//�S�m�[�g�{�b�N�X
	DXPrimitive m_PrimitiveAllNotes;

	//�������m�[�g�{�b�N�X
	DXPrimitive m_PrimitiveActiveNotes;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	unsigned long m_ActiveNoteNum;

	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;

	virtual int _CreateNoteStatus();

	int _CreateVertexOfNote(
			SMNote note,
			MTNOTEBOX_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIbIndex,
			unsigned long elapsedTime = 0xFFFFFFFF,
			bool isEnablePitchBend = false
		);
	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

	virtual int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

	int _HideNoteBox(unsigned long index);
	int _ShowNoteBox(unsigned long index);

private:

	//�����m�[�g���\����
	struct NoteStatus {
		bool isActive;
		bool isHide;
		unsigned long index;
		unsigned long startTime;
	};

private:

	//�������m�[�g�{�b�N�X
	NoteStatus* m_pNoteStatus;

	//�X�L�b�v���
	bool m_isSkipping;

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateAllNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateActiveNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);

	unsigned long _GetVertexIndexOfNote(unsigned long index);

	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	void _MakeMaterialForActiveNote(D3DMATERIAL9* pMaterial);

};


