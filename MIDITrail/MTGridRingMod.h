//******************************************************************************
//
// MIDITrail / MTGridRingMod
//
// �O���b�h�����O�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridRing.h"


//******************************************************************************
//  �O���b�h�����O�`��Mod�N���X
//******************************************************************************
class MTGridRingMod : public MTGridRing
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTGridRingMod(void);
	virtual ~MTGridRingMod(void);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�\���ݒ�
	void SetEnable(bool isEnable);

private:

	//�\����
	bool m_isEnable;
};

