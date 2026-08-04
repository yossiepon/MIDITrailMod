//******************************************************************************
//
// MIDITrail / MTColorPaletteCfgDlg
//
// カラーパレット設定ダイアログ
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTColorConf.h"
#include <map>


//******************************************************************************
// カラー設定ダイアログクラス
//******************************************************************************
class MTColorPaletteCfgDlg
{
public:

	//コンストラクタ／デストラクタ
	MTColorPaletteCfgDlg(void);
	virtual ~MTColorPaletteCfgDlg(void);

	//カラーパレット設定
	void SetColorPalette(
				MTColorPalette* pColorPalette, 
				MTColorPalette* pDefaultColorPalette,
			 	unsigned long colorPaletteNo
			 );

	//カラーパレット取得
	void GetColorPalette(MTColorPalette* pColorPalette);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

	//パラメータ変更確認
	bool IsChanged();

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTColorPaletteCfgDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウハンドル
	HWND m_hWnd;

	//カラーボタンリスト：Ch.1-16, BG/GL/CT, Start/End
	HWND m_hColorButtonList[SM_MAX_CH_NUM + 3 + 2];

	//カラーテキストリスト：Ch.1-16, BG/GL/CT, Start/End
	HWND m_hColorTextList[SM_MAX_CH_NUM + 3 + 2];

	//カラーパレット
	MTColorPalette m_ColorPalette;
	MTColorPalette m_DefaultColorPalette;
	unsigned long m_ColorPaletteNo;
	D3DXCOLOR m_ColorStart;
	D3DXCOLOR m_ColorEnd;

	//色選択ダイアログ用パラメータ
	COLORREF m_CustColors[16];

	//変更フラグ
	bool m_isChanged;

	//カラーパラメータマップ
	typedef std::map<std::string, std::string> MTColorParamDictionary;
	typedef std::pair<std::string, std::string> MTColorParamDictionaryPair;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//カラーボタンリスト初期化
	void _InitColorButtonList();

	//カラーボタン初期化
	int _InitColorButtons();

	//カラーテキスト初期化
	int _InitColorText();

	//コンボボックス初期化
	int _InitCombobox(HWND hCombobox, int selectedIndex);

	//カラーボタン押下
	int _OnBtnColor(unsigned long targetNo);

	//グラデーションツール：Set Gradation Colors ボタン押下
	int _OnBtnSetGradationColors();

	//パラメータセットアップツール：Set Default Colors ボタン押下
	int _OnBtnSetDefaultColors();

	//パラメータセットアップツール：Export Color Parameters ボタン押下
	int _OonBtnExportColorParameters();

	//パラメータセットアップツール：Import Color Parameters ボタン押下
	int _OnBtnImportColorParameters();

	//カラーボタン更新
	int _UpdateColorButtons();

	//カラーボタン描画
	int _DrawColorButton(DRAWITEMSTRUCT* pDrawItem);

	//カラーテキスト更新
	int _UpdateColorText();

	//カラー取得
	int _GetCurColor(unsigned long targetNo, D3DXCOLOR* pColor);

	//カラー設定
	int _SetCurColor(unsigned long targetNo, D3DXCOLOR color);

	//カラー選択ダイアログ表示
	int _ShowChooseColorDlg(
				D3DXCOLOR color,
				D3DXCOLOR* pNewColor,
				bool* pIsChoosed
			);

	//グラデーションカラー設定
	int _SetGradationColor(
				unsigned long chNoStart,
				unsigned long chNoEnd,
				D3DXCOLOR colorStart,
				D3DXCOLOR colorEnd
			);

	//出力用パラメータ文字列生成
	int _MakeColorParamForExport(TCHAR* pTextBuf, unsigned long bufSize);

	//パラメータ入力処理
	int _ImportColorParam(TCHAR* pParamString);

	//パラメータマップ作成
	int _MakeImportKeyValueMap(TCHAR* pParamString, MTColorParamDictionary* pParamDictionary);

	//パラメータ読み込み
	int _LoadParam(MTColorParamDictionary* pParamDictionary);

};


