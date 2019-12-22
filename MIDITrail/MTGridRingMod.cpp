//******************************************************************************
//
// MIDITrail / MTGridRingMod
//
// グリッドリング描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTGridRingMod.h"

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTGridRingMod::MTGridRingMod(void)
{
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTGridRingMod::~MTGridRingMod(void)
{
}

//******************************************************************************
// 描画
//******************************************************************************
int MTGridRingMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	result = MTGridRing::Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTGridRingMod::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}
