//******************************************************************************
//
// MIDITrail / MTNoteBoxRing
//
// �m�[�g�{�b�N�X�����O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBox.h"


//******************************************************************************
// �m�[�g�{�b�N�X�����O�`��N���X
//******************************************************************************
class MTNoteBoxRing : public MTNoteBox
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteBoxRing(void);
	virtual ~MTNoteBoxRing(void);

private:

	virtual int _CreateNoteDesign();

};


