//******************************************************************************
//
// MIDITrail / MTNoteBoxRingLive
//
// ���C�u���j�^�p�m�[�g�{�b�N�X�����O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBoxLive.h"


//******************************************************************************
// ���C�u���j�^�p�m�[�g�{�b�N�X�����O�`��N���X
//******************************************************************************
class MTNoteBoxRingLive : public MTNoteBoxLive
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteBoxRingLive(void);
	virtual ~MTNoteBoxRingLive(void);

private:

	virtual int _CreateNoteDesign();

};


