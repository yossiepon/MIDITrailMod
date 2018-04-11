//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DMod
//
// �s�A�m���[��3D�V�[���`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRoll3D.h"
#include "MTGridBoxMod.h"
#include "MTNoteBoxMod.h"
#include "MTNoteRippleMod.h"
#include "MTNoteLyrics.h"
#include "MTPianoKeyboardCtrlMod.h"

//******************************************************************************
// �s�A�m���[��3D�V�[���`��Mod�N���X
//******************************************************************************
class MTScenePianoRoll3DMod : public MTScenePianoRoll3D
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^l
	MTScenePianoRoll3DMod();
	virtual ~MTScenePianoRoll3DMod();

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

	virtual void _Reset();

	//�V���O���L�[�{�[�h�t���O
	bool m_IsSingleKeyboard;

private:

	// ���C�g2
	DXDirLight m_DirLightBack;

	//�`��I�u�W�F�N�g
	MTGridBoxMod m_GridBoxMod;
	MTNoteBoxMod m_NoteBoxMod;
	MTNoteRippleMod m_NoteRippleMod;
	MTNoteLyrics m_NoteLyrics;
	MTPianoKeyboardCtrlMod m_PianoKeyboardCtrl;

};

