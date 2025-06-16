//******************************************************************************
//
// MIDITrail / MTColorParamExportDlg
//
// カラーパラメータ出力ダイアログ
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// パラメータ定義
//******************************************************************************

//パラメータ文字列最大長
#define MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX		(2048)


//******************************************************************************
// カラーパラメータ出力ダイアログ
//******************************************************************************
class MTColorParamExportDlg
{
public:

	//コンストラクタ／デストラクタ
	MTColorParamExportDlg(void);
	virtual ~MTColorParamExportDlg(void);

	//パラメータ文字列登録
	void SetParamString(TCHAR* pString);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTColorParamExportDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウハンドル
	HWND m_hWnd;

	//パラメータ文字列
	TCHAR m_ParamString[2048];

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//コピーボタン押下
	int _OnBtnCopy();

};


