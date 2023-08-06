//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRingMod
//
// �^�C���C���W�P�[�^�����O�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTTimeIndicatorRingMod.h"

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTTimeIndicatorRingMod::MTTimeIndicatorRingMod(void)
{
	m_isEnable = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTTimeIndicatorRingMod::~MTTimeIndicatorRingMod(void)
{
}

//******************************************************************************
// �`��
//******************************************************************************
int MTTimeIndicatorRingMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	result = MTTimeIndicatorRing::Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTTimeIndicatorRingMod::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}
