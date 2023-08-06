//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRingMod
//
// �^�C���C���W�P�[�^�����O�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTTimeIndicatorRing.h"


//******************************************************************************
//  �^�C���C���W�P�[�^�����O�`��Mod�N���X
//******************************************************************************
class MTTimeIndicatorRingMod : public MTTimeIndicatorRing
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTTimeIndicatorRingMod(void);
	virtual ~MTTimeIndicatorRingMod(void);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�\���ݒ�
	void SetEnable(bool isEnable);

private:

	//�\����
	bool m_isEnable;
};

