//******************************************************************************
//
// MIDITrail / MTGridBoxMod
//
// �O���b�h�{�b�N�X�`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridBox.h"


//******************************************************************************
//  �O���b�h�{�b�N�X�`��Mod�N���X
//******************************************************************************
class MTGridBoxMod : public MTGridBox
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTGridBoxMod(void);
	virtual ~MTGridBoxMod(void);

	//�X�V
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

};

