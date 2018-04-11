//******************************************************************************
//
// MIDITrail / MTGridBoxMod
//
// グリッドボックス描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridBox.h"


//******************************************************************************
//  グリッドボックス描画Modクラス
//******************************************************************************
class MTGridBoxMod : public MTGridBox
{
public:

	//コンストラクタ／デストラクタ
	MTGridBoxMod(void);
	virtual ~MTGridBoxMod(void);

	//更新
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

};

