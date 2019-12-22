//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRingMod
//
// タイムインジケータリング描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTTimeIndicatorRingMod.h"

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTTimeIndicatorRingMod::MTTimeIndicatorRingMod(void)
{
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTTimeIndicatorRingMod::~MTTimeIndicatorRingMod(void)
{
}

//******************************************************************************
// 描画
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
// 表示設定
//******************************************************************************
void MTTimeIndicatorRingMod::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}
