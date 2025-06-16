//******************************************************************************
//
// MIDITrail / MTColorPalette
//
// カラーパレットクラス
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "SMIDILib.h"
#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// カラーパレットクラス
//******************************************************************************
class MTColorPalette
{
public:
	
	//コンストラクタ／デストラクタ
	MTColorPalette(void);
	virtual ~MTColorPalette(void);
	
	//初期化
	int Initialize();
	
	//チャンネル色取得
	int GetChColor(unsigned int chNo, D3DXCOLOR* pColor);
	
	//チャンネル色登録
	int SetChColor(unsigned int chNo, D3DXCOLOR color);
	
	//背景色取得
	void GetBackgroundColor(D3DXCOLOR* pColor);

	//背景色登録
	void SetBackgroundColor(D3DXCOLOR color);
	
	//グリッドライン色取得
	void GetGridLineColor(D3DXCOLOR* pColor);

	//グリッドライン色登録
	void SetGridLineColor(D3DXCOLOR color);
	
	//カウンター色取得
	void GetCounterColor(D3DXCOLOR* pColor);

	//カウンター色登録
	void SetCounterColor(D3DXCOLOR color);

	//コピー
	int CopyFrom(MTColorPalette* pColorSrc);
	
private:
	
	//代入とコピーコンストラクタの禁止
	void operator=(const MTColorPalette&);
	MTColorPalette(const MTColorPalette&);
	
	D3DXCOLOR m_ChColor[SM_MAX_CH_NUM];
	D3DXCOLOR m_BgColor;
	D3DXCOLOR m_GridLineColor;
	D3DXCOLOR m_CounterColor;
	
	void _Clear();
	
};


