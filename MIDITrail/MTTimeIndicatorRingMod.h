//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRingMod
//
// タイムインジケータリング描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTTimeIndicatorRing.h"


//******************************************************************************
//  タイムインジケータリング描画Modクラス
//******************************************************************************
class MTTimeIndicatorRingMod : public MTTimeIndicatorRing
{
public:

	//コンストラクタ／デストラクタ
	MTTimeIndicatorRingMod(void);
	virtual ~MTTimeIndicatorRingMod(void);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//表示設定
	void SetEnable(bool isEnable);

private:

	//表示可否
	bool m_isEnable;
};

