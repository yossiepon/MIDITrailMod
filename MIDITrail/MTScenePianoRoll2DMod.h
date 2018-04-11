//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DMod
//
// �s�A�m���[��2D�V�[���`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once
#include "MTScenePianoRoll3DMod.h"


//******************************************************************************
// �s�A�m���[��2D�V�[���`��Mod�N���X
//******************************************************************************
class MTScenePianoRoll2DMod : public MTScenePianoRoll3DMod
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTScenePianoRoll2DMod(void);
	virtual ~MTScenePianoRoll2DMod(void);

	//���̎擾
	const TCHAR* GetName();

	//����
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

};

