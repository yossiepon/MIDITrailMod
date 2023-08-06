//******************************************************************************
//
// MIDITrail / MTTimeIndicatorMod
//
// �^�C���C���W�P�[�^�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTTimeIndicatorMod.h"

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTTimeIndicatorMod::MTTimeIndicatorMod(void)
{
	m_isEnable = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTTimeIndicatorMod::~MTTimeIndicatorMod(void)
{
}

//******************************************************************************
// �`��
//******************************************************************************
int MTTimeIndicatorMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	result = MTTimeIndicator::Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTTimeIndicatorMod::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}
