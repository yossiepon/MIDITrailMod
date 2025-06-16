//******************************************************************************
//
// MIDITrail / MTColorConf
//
// カラー設定クラス
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTColorPalette.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//カラーパレット数最大値（デフォルト設定を含む）
#define MT_COLOR_PALETTE_NUM_MAX	(7)


//******************************************************************************
// カラー設定クラス
//******************************************************************************
class MTColorConf
{
public:
	
	//コンストラクタ／デストラクタ
	MTColorConf(void);
	virtual ~MTColorConf(void);
	
	//初期化
	int Initialize(const TCHAR* pDefaultSceneName);
	
	//選択カラーパレット番号取得：0 デフォルト、1-6 パレット番号
	unsigned long GetSelectedColorPaletteNo();
	
	//選択カラーパレット番号登録：0 デフォルト、1-6 パレット番号
	int SetSelectedColorPaletteNo(unsigned long paletteNo);
	
	//カラーパレット取得：0 デフォルト、1-6 パレット番号
	int GetColorPalette(unsigned long paletteNo, MTColorPalette* pColorPalette);
	
	//選択カラーパレット取得
	void GetSelectedColorPalette(MTColorPalette* pColorPalette);
	
	//カラーパレット登録：1-6 パレット番号、0 デフォルトは登録不可
	int SetColorPalette(unsigned long paletteNo, MTColorPalette* pColorPalette);
	
	//設定保存
	int Save();
	
private:
	
	//代入とコピーコンストラクタの禁止
	void operator=(const MTColorConf&);
	MTColorConf(const MTColorConf&);
	
	YNConfFile m_ConfFile;
	int m_SelectedColorPaletteNo;
	MTColorPalette* m_pColorPalette[MT_COLOR_PALETTE_NUM_MAX];
	
	int _InitConfFile();
	int _LoadColorConf(const TCHAR* pDefaultSceneName);
	int _LoadColorPaletteDefault(const TCHAR* pDefaultSceneName, MTColorPalette* pColorPalette);
	int _LoadColorPalettes(unsigned long paletteNo, MTColorPalette* pColorPalette);
	int _SaveColorPalette(unsigned long paletteNo, MTColorPalette* pColorPalette);
	
};


