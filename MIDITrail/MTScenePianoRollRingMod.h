//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingMod
//
// �s�A�m���[�������O�V�[���`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRippleRingMod.h"
#include "MTGridRingMod.h"
#include "MTTimeIndicatorRingMod.h"
#include "MTScenePianoRollRing.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m���[�������O�V�[���`��Mod�N���X
//******************************************************************************
class MTScenePianoRollRingMod : public MTScenePianoRollRing
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^l
	MTScenePianoRollRingMod();
	virtual ~MTScenePianoRollRingMod();

	//���̎擾
	const TCHAR* GetName();

	//����
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

	//�ϊ�
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�j��
	virtual void Release();

	//�V�[�P���T���b�Z�[�W��M
	virtual int OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	);

	//�G�t�F�N�g�ݒ�
	virtual void SetEffect(MTScene::EffectType type, bool isEnable);

protected:

	//�`��I�u�W�F�N�g
	MTNoteRippleRingMod m_NoteRippleMod;
	MTGridRingMod m_GridRingMod;
	MTTimeIndicatorRingMod m_TimeIndicatorMod;

	//�V�[�����Z�b�g
	virtual void _Reset();

private:

};

