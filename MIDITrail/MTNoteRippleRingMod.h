//******************************************************************************
//
// MIDITrail / MTNoteRippleRingMod
//
// �m�[�g�g�䃊���O�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRippleMod.h"


//******************************************************************************
// �����O�p�m�[�g�g��`��Mod�N���X
//******************************************************************************
class MTNoteRippleRingMod : public MTNoteRippleMod
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteRippleRingMod(void);
	virtual ~MTNoteRippleRingMod(void);

private:

	virtual int _CreateNoteDesign();

};


