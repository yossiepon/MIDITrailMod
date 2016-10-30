//******************************************************************************
//
// MIDITrail / MTNoteLyrics
//
// �m�[�g�̎��`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesignMod.h"
#include "MTNotePitchBend.h"
#include "MTFontTexture.h"

using namespace SMIDILib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő�|�[�g��
#define MTNOTELYRICS_MAX_PORT_NUM  (8)

//�ő�̎��`�搔
#define MTNOTELYRICS_MAX_LYRICS_NUM  (100)

// TODO: �ő�̎��`�搔���ςɂ���
//   ���O�ɃV�[�P���X�f�[�^�̍ő哯���������𒲍����Ă�����
//   �m�ۂ���o�b�t�@�T�C�Y��ύX�ł���
//   ����ł��o�b�t�@�T�C�Y�͏��������_�œ��I�ɕύX�\�ł���


//******************************************************************************
// �m�[�g�̎��`��N���X
//******************************************************************************
class MTNoteLyrics
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteLyrics(void);
	virtual ~MTNoteLyrics(void);

	//����
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	void Release();

	//���t�`�b�N�^�C���o�^
	void SetCurTickTime(unsigned long curTickTime);

	//���t���Ԑݒ�
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//���Z�b�g
	void Reset();

	//�\���ݒ�
	void SetEnable(bool isEnable);

	//�X�L�b�v���
	void SetSkipStatus(bool isSkipping);

private:

	//�L�[���
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//�����m�[�g���\����
	struct NoteStatus {
		bool isActive;
		KeyStatus keyStatus;
		unsigned long index;
		float keyDownRate;
		MTFontTexture fontTexture;
	};

	//���_�o�b�t�@�\����
	struct MTNOTELYRICS_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
		D3DXVECTOR2 t;	//�e�N�X�`���摜�ʒu
	};

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

private:

	//�`��n
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTextures[MTNOTELYRICS_MAX_LYRICS_NUM];
	D3DMATERIAL9 m_Material;

	//�J����
	D3DXVECTOR3 m_CamVector;

	//�m�[�g�f�U�C��
	MTNoteDesignMod m_NoteDesign;

	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;

	//�m�[�g���X�g
	SMNoteList m_NoteListRT;

	//�������m�[�g�Ǘ�
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	float m_KeyDownRate[MTNOTELYRICS_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//�m�[�g������ԏ��
	NoteStatus* m_pNoteStatus;
	unsigned long m_ActiveNoteNum;

	//�\����
	bool m_isEnable;

	//�X�L�b�v���
	bool m_isSkipping;

	int _CreateNoteStatus();
	int _CreateVertex(LPDIRECT3DDEVICE9 pD3DDevice);
	int _SetVertexPosition(
				MTNOTELYRICS_VERTEX* pVertex,
				SMNote note,
				NoteStatus* pNoteStatus,
				unsigned long rippleNo
			);
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	int _TransformLyrics(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfLyrics(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long decayDuration,
				unsigned long releaseDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	int _UpdateVertexOfLyrics(LPDIRECT3DDEVICE9 pD3DDevice);

};


