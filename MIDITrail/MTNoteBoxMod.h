//******************************************************************************
//
// MIDITrail / MTNoteBoxMod
//
// �m�[�g�{�b�N�X�`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBox.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő�|�[�g��
#define MT_NOTEBOX_MAX_PORT_NUM  (8)


//******************************************************************************
// �m�[�g�{�b�N�X�`��Mod�N���X
//******************************************************************************
class MTNoteBoxMod : public MTNoteBox
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteBoxMod(void);
	virtual ~MTNoteBoxMod(void);

	//����
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//�X�V
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//���t���Ԑݒ�
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//���
	virtual void Release();

	//���Z�b�g
	virtual void Reset();

private:

	//�L�[���
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//�����m�[�g���\����
	struct NoteStatusMod {
		bool isActive;
		bool isHide;
		unsigned long index;
		KeyStatus keyStatus;
		float keyDownRate;
	};

protected:

	virtual int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//�m�[�g�f�U�C��
	MTNoteDesignMod m_NoteDesignMod;

	//�m�[�g���X�g
	SMNoteList m_NoteListRT;

	//�������m�[�g�Ǘ�
	unsigned long m_PlayTimeMSec;
	float m_KeyDownRate[MT_NOTEBOX_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//�m�[�g������ԏ��
	NoteStatusMod* m_pNoteStatusMod;

	virtual int _CreateNoteStatus();

	int _CreateVertexOfNote(
			SMNote note,
			MTNOTEBOX_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIbIndex,
			float keyDownRate = 0.0f,
			bool isEnablePitchBend = false
		);

	int _UpdateNoteStatus(
			unsigned long playTimeMSec,
			unsigned long decayDuration,
			unsigned long releaseDuration,
			SMNote note,
			NoteStatusMod* pNoteStatus
		);
};


