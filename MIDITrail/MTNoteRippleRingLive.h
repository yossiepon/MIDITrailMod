//******************************************************************************
//
// MIDITrail / MTNoteRippleRingLive
//
// ���C�u���j�^�p�m�[�g�g�䃊���O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRipple.h"


//******************************************************************************
// ���C�u���j�^�p�m�[�g�g�䃊���O�`��N���X
//******************************************************************************
class MTNoteRippleRingLive : public MTNoteRipple
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteRippleRingLive(void);
	virtual ~MTNoteRippleRingLive(void);

private:

	virtual int _CreateNoteDesign();

};


