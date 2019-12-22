//******************************************************************************
//
// MIDITrail / MTTimeIndicatorMod
//
// タイムインジケータ描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTTimeIndicator.h"


//******************************************************************************
//  タイムインジケータ描画Modクラス
//******************************************************************************
class MTTimeIndicatorMod : public MTTimeIndicator
{
public:

	//コンストラクタ／デストラクタ
	MTTimeIndicatorMod(void);
	virtual ~MTTimeIndicatorMod(void);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//表示設定
	void SetEnable(bool isEnable);

private:

	//表示可否
	bool m_isEnable;
};

