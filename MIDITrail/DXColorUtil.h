//******************************************************************************
//
// MIDITrail / DXColorUtil
//
// カラーユーティリティクラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// カラーユーティリティクラス
//******************************************************************************
class DXColorUtil
{
public:

	//RGBA（16進数文字列）からの数値変換
	static D3DXCOLOR MakeColorFromHexRGBA(const TCHAR* pHexRGBA);

	//RGB（16進数文字列）からの数値変換
	static D3DCOLOR MakeColorFromHexRGB(const TCHAR* pHexRGB);

private:

	//コンストラクタ／デストラクタ
	DXColorUtil(void);
	virtual ~DXColorUtil(void);

	//代入とコピーコンストラクタの禁止
	void operator=(const DXColorUtil&);
	DXColorUtil(const DXColorUtil&);

};

