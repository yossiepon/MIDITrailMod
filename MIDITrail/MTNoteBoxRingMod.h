//******************************************************************************
//
// MIDITrail / MTNoteBoxRingMod
//
// �m�[�g�{�b�N�X�����O�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBoxMod.h"


//******************************************************************************
// �m�[�g�{�b�N�X�����O�`��N���X
//******************************************************************************
class MTNoteBoxRingMod : public MTNoteBoxMod
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteBoxRingMod(void);
	virtual ~MTNoteBoxRingMod(void);

private:

	virtual int _CreateNoteDesign();

};


