//******************************************************************************
//
// MIDITrail / MTColorCfgDlg
//
// カラー設定ダイアログ
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "MTColorConf.h"
#include "MTColorPaletteCfgDlg.h"

using namespace YNBaseLib;


//******************************************************************************
// カラー設定ダイアログクラス
//******************************************************************************
class MTColorCfgDlg
{
public:

	//コンストラクタ／デストラクタ
	MTColorCfgDlg(void);
	virtual ~MTColorCfgDlg(void);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

	//パラメータ変更確認
	bool IsChanged();

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTColorCfgDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウハンドル
	HWND m_hWnd;

	//ラジオボタンリスト
	HWND m_hBtnRadioPaletteList[MT_COLOR_PALETTE_NUM_MAX];

	//カラーボタンリスト
	HWND m_hBtnColorList[MT_COLOR_PALETTE_NUM_MAX][SM_MAX_CH_NUM + 3];

	//カラー情報
	MTColorConf m_ColorConf;

	//選択カラーパレット番号
	int m_SelectedColorPaletteNo;

	//変更フラグ
	bool m_isChanged;

	//カラーパレット設定ダイアログ
	MTColorPaletteCfgDlg m_ColorPaletteCfgDlg;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//ラジオボタンリスト初期化
	void _InitRadioButtonList();

	//カラーボタンリスト初期化
	void _InitColorButtonList();

	//ラジオボタン初期化
	int _InitRadioButtons();

	//カラーボタン初期化
	int _InitColorButtons();

	//ラジオボタン押下
	int _OnBtnRadio(unsigned long buttonNo);

	//編集ボタン押下
	int _OnBtnEdit(unsigned long paletteNo);

	//カラーパレット設定ダイアログ表示
	int _ShowColorPaletteCfgDlg(unsigned long colorPaletteNo);

	//カラーボタン更新
	int _UpdateColorButtons(unsigned long colorPaletteNo);

	//カラーボタン描画
	int _DrawColorButton(DRAWITEMSTRUCT* pDrawItem);

	//カラー設定保存
	int _Save();

};


