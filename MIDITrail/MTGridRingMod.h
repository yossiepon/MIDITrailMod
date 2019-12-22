//******************************************************************************
//
// MIDITrail / MTGridRingMod
//
// グリッドリング描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridRing.h"


//******************************************************************************
//  グリッドリング描画Modクラス
//******************************************************************************
class MTGridRingMod : public MTGridRing
{
public:

	//コンストラクタ／デストラクタ
	MTGridRingMod(void);
	virtual ~MTGridRingMod(void);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//表示設定
	void SetEnable(bool isEnable);

private:

	//表示可否
	bool m_isEnable;
};

