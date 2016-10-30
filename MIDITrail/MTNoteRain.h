//******************************************************************************
//
// MIDITrail / MTNoteRain
//
// �m�[�g���C���`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �m�[�g���C����`�悷��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "MTPianoKeyboardDesign.h"

using namespace SMIDILib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő唭���m�[�g�`�搔
#define MTNOTERAIN_MAX_ACTIVENOTE_NUM  (100)

// TODO: �ő唭���m�[�g�`�搔���ςɂ���
//   ���O�ɃV�[�P���X�f�[�^�̍ő哯���������𒲍����Ă�����
//   �m�ۂ���o�b�t�@�T�C�Y��ύX�ł���
//   ����ł��o�b�t�@�T�C�Y�͏��������_�œ��I�ɕύX�\�ł���


//******************************************************************************
// �m�[�g���C���`��N���X
//******************************************************************************
class MTNoteRain
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteRain(void);
	virtual ~MTNoteRain(void);

	//����
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

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

	//�X�L�b�v���
	void SetSkipStatus(bool isSkipping);

private:

	//�����m�[�g���\����
	struct NoteStatus {
		bool isActive;
		unsigned long index;
		unsigned long startTime;
	};

	//���_�o�b�t�@�\����
	struct MTNOTERAIN_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD       c;	//�f�B�t���[�Y�F
	};

private:

	//�m�[�g�f�U�C��
	MTNoteDesign m_NoteDesign;

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesign m_KeyboardDesign;

	//�m�[�g���X�g
	SMNoteList m_NoteList;

	//�S�m�[�g���C��
	DXPrimitive m_PrimitiveAllNotes;

	//�������m�[�g�{�b�N�X
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	NoteStatus* m_pNoteStatus;
	float m_CurPos;

	//�X�L�b�v���
	bool m_isSkipping;

	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateAllNoteRain(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateVertexOfNote(
				SMNote note,
				MTNOTERAIN_VERTEX* pVertex,
				unsigned long vertexOffset,
				unsigned long* pIndex
			);
	int _CreateNoteStatus();
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateVertexOfNote(
				unsigned long index,
				bool isEnablePitchBendShift = false
			);

};


