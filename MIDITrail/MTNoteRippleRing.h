//******************************************************************************
//
// MIDITrail / MTNoteRippleRing
//
// �����O�p�m�[�g�g��`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRipple.h"

using namespace SMIDILib;


//******************************************************************************
// �����O�p�m�[�g�g��`��N���X
//******************************************************************************
class MTNoteRippleRing : public MTNoteRipple
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteRippleRing(void);
	virtual ~MTNoteRippleRing(void);

private:

	virtual int _CreateNoteDesign();

};


