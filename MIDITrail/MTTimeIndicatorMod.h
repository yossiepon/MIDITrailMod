//******************************************************************************
//
// MIDITrail / MTTimeIndicatorMod
//
// �^�C���C���W�P�[�^�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTTimeIndicator.h"


//******************************************************************************
//  �^�C���C���W�P�[�^�`��Mod�N���X
//******************************************************************************
class MTTimeIndicatorMod : public MTTimeIndicator
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTTimeIndicatorMod(void);
	virtual ~MTTimeIndicatorMod(void);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�\���ݒ�
	void SetEnable(bool isEnable);

private:

	//�\����
	bool m_isEnable;
};

