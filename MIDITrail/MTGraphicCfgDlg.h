//******************************************************************************
//
// MIDITrail / MTGraphicCfgDlg
//
// グラフィック設定ダイアログクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"

using namespace YNBaseLib;

//multisample (MSAA) type range for the graphics config dialog
//(was in the old DX9 DXRenderer.h; kept here, the dialog that owns it)
#define DX_MULTI_SAMPLE_TYPE_MIN    (2)
#define DX_MULTI_SAMPLE_TYPE_MAX    (16)


//******************************************************************************
// グラフィック設定定義
//******************************************************************************
//アンチエイリアシング：マルチサンプル種別デフォルト
#define MT_GRAPHIC_MULTI_SAMPLE_TYPE_DEF  (0)  //OFF

//ced 20260628: スーパーサンプリング(SSAA)倍率（1=OFF, 2..4）
#define DX_SUPER_SAMPLE_MIN          (2)
#define DX_SUPER_SAMPLE_MAX          (4)
#define MT_GRAPHIC_SUPER_SAMPLE_DEF  (1)  //OFF(=1倍)


//******************************************************************************
// グラフィック設定ダイアログクラス
//******************************************************************************
class MTGraphicCfgDlg
{
public:

	//コンストラクタ／デストラクタ
	MTGraphicCfgDlg(void);
	virtual ~MTGraphicCfgDlg(void);

	//アンチエイリアシングサポート情報設定
	void SetAntialiasSupport(unsigned long multiSampleType, bool isSupport);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

	//パラメータ変更確認
	bool IsChanged();

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTGraphicCfgDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウハンドル
	HWND m_hWnd;

	//設定ファイル
	YNConfFile m_ConfFile;

	//コンボボックスのウィンドウハンドル
	HWND m_hComboMultiSampleType;
	bool m_MultSampleTypeSupport[DX_MULTI_SAMPLE_TYPE_MAX+1];

	//ced 20260628: スーパーサンプリング(SSAA)コンボ
	HWND m_hComboSuperSample;

	//背景画像ファイルパスエディットボックスのウィンドウハンドル
	HWND m_hEditImageFilePath;
	
	//四分音符長拡大率エディットボックスのウィンドウハンドル
	HWND m_hEditQuarterNoteLengthMag;

	//アンチエイリアシング設定
	unsigned long m_MultiSampleType;

	//ced 20260628: スーパーサンプリング倍率（1=OFF）
	unsigned long m_SuperSample;

	//背景画像ファイルパス
	TCHAR m_ImageFilePath[_MAX_PATH];
	
	//四分音符長拡大率
	int m_QuarterNoteLengthMag;

	//更新フラグ
	bool m_isChanged;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//設定ファイル初期化
	int _InitConfFile();

	//設定ファイル読み込み
	int _LoadConf();

	//デバイス選択コンボボックス初期化
	int _InitComboMultiSampleType(HWND hCombo, unsigned long selMultiSampleType);

	//ced 20260628: スーパーサンプリングコンボ初期化
	int _InitComboSuperSample(HWND hCombo, unsigned long selSuperSample);

	//背景画像ファイルパス初期化
	int _InitBackgroundImageFilePath();
	
	//四分音符設定初期化
	int _InitQuarterNote();

	//保存処理
	int _Save();

	//背景画像ファイルパスブラウズボタン押下
	int _OnBtnBrowse();

	//画像ファイル選択
	int _SelectImageFile(TCHAR* pFilePath, unsigned long bufSize, bool* pIsSelected);

};


