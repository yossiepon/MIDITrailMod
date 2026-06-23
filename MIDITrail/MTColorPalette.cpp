//******************************************************************************
//
// MIDITrail / MTColorPalette
//
// カラーパレットクラス
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTColorPalette.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTColorPalette::MTColorPalette(void)
{
	_Clear();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTColorPalette::~MTColorPalette(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTColorPalette::Initialize()
{
	int result = 0;
	
	_Clear();
	
	return result;
}

//******************************************************************************
// クリア
//******************************************************************************
void MTColorPalette::_Clear()
{
	unsigned int chNo = 0;
	
	//色初期化
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		m_ChColor[chNo] = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	}
	m_BgColor       = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_GridLineColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_CounterColor  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	return;
}

//******************************************************************************
// チャンネル色取得
//******************************************************************************
int MTColorPalette::GetChColor(unsigned int chNo, D3DXCOLOR* pColor)
{
	int result = 0;
	
	if (chNo >= SM_MAX_CH_NUM) {
		result = YN_SET_ERR("Program error.", chNo, 0);
		goto EXIT;
	}
	
	*pColor = m_ChColor[chNo];
	
EXIT:;
	return result;
}

//******************************************************************************
// チャンネル色登録
//******************************************************************************
int MTColorPalette::SetChColor(unsigned int chNo, D3DXCOLOR color)
{
	int result = 0;
	
	if (chNo >= SM_MAX_CH_NUM) {
		result = YN_SET_ERR("Program error.", chNo, 0);
		goto EXIT;
	}
	
	m_ChColor[chNo] = color;
	
EXIT:;
	return result;
}

//******************************************************************************
// 背景色取得
//******************************************************************************
void MTColorPalette::GetBackgroundColor(D3DXCOLOR* pColor)
{
	*pColor = m_BgColor;
}

//******************************************************************************
// 背景色登録
//******************************************************************************
void MTColorPalette::SetBackgroundColor(D3DXCOLOR color)
{
	m_BgColor = color;
}

//******************************************************************************
// グリッドライン色取得
//******************************************************************************
void MTColorPalette::GetGridLineColor(D3DXCOLOR* pColor)
{
	*pColor = m_GridLineColor;
}

//******************************************************************************
// グリッドライン色登録
//******************************************************************************
void MTColorPalette::SetGridLineColor(D3DXCOLOR color)
{
	m_GridLineColor = color;
}

//******************************************************************************
// カウンター色取得
//******************************************************************************
void MTColorPalette::GetCounterColor(D3DXCOLOR* pColor)
{
	*pColor = m_CounterColor;
}

//******************************************************************************
// カウンター色登録
//******************************************************************************
void MTColorPalette::SetCounterColor(D3DXCOLOR color)
{
	m_CounterColor = color;
}

//******************************************************************************
// コピー
//******************************************************************************
int MTColorPalette::CopyFrom(MTColorPalette* pColorSrc)
{
	int result = 0;
	unsigned int chNo = 0;
	
	//チャンネル色のコピー
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		result = pColorSrc->GetChColor(chNo, &m_ChColor[chNo]);
		if (result != 0) goto EXIT;
	}
	
	//背景色のコピー
	pColorSrc->GetBackgroundColor(&m_BgColor);
	
	//グリッドライン色のコピー
	pColorSrc->GetGridLineColor(&m_GridLineColor);
	
	//カウンター色のコピー
	pColorSrc->GetCounterColor(&m_CounterColor);
	
EXIT:;
	return result;
}


