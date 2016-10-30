//******************************************************************************
//
// MIDITrail / MTConfFile
//
// 設定ファイルクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"

using namespace YNBaseLib;


//******************************************************************************
// 設定ファイルクラス
//******************************************************************************
class MTConfFile : public YNConfFile
{
public:

	//コンストラクタ／デストラクタ
	MTConfFile(void);
	virtual ~MTConfFile(void);

	//初期化
	int Initialize(const TCHAR* pCategory);

};


