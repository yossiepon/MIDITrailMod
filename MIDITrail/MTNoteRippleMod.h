//******************************************************************************
//
// MIDITrail / MTNoteRippleMod
//
// �m�[�g�g��`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRipple.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő�|�[�g��
#define MTNOTERIPPLE_MAX_PORT_NUM  (8)


//******************************************************************************
// �m�[�g�g��`��Mod�N���X
//******************************************************************************
class MTNoteRippleMod : public MTNoteRipple
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteRippleMod(void);
	virtual ~MTNoteRippleMod(void);

	//����
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	virtual void Release();

	//���t���Ԑݒ�
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//���Z�b�g
	virtual void Reset();

protected:

	virtual int _CreateNoteStatus();
	virtual int _CreateVertex(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void _MakeMaterial(D3DMATERIAL9* pMaterial);
	virtual int _TransformRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//�m�[�g������ԍ\����
	//�L�[���
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//�����m�[�g���\����
	struct NoteStatusMod {
		bool isActive;
		KeyStatus keyStatus;
		unsigned long index;
		float keyDownRate;
	};

private:

	//�m�[�g�f�U�C��
	MTNoteDesignMod m_NoteDesignMod;

	//�m�[�g���X�g
	SMNoteList m_NoteListRT;

	//�������m�[�g�Ǘ�
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurNoteIndex;
	float m_KeyDownRate[MTNOTERIPPLE_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//�m�[�g������ԏ��
	NoteStatusMod* m_pNoteStatusMod;

	int _SetVertexPosition(
				MTNOTERIPPLE_VERTEX* pVertex,
				SMNote note,
				NoteStatusMod* pNoteStatus,
				unsigned long rippleNo
			);
	int _UpdateStatusOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long decayDuration,
				unsigned long releaseDuration,
				SMNote note,
				NoteStatusMod* pNoteStatus
			);
};


