//******************************************************************************
//
// MIDITrail / MTTimeIndicatorMod
//
// タイムインジケータ描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTTimeIndicatorMod.h"

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTTimeIndicatorMod::MTTimeIndicatorMod(void)
{
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTTimeIndicatorMod::~MTTimeIndicatorMod(void)
{
}

//******************************************************************************
// 描画
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
// 表示設定
//******************************************************************************
void MTTimeIndicatorMod::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}
