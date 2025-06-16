//******************************************************************************
//
// MIDITrail / MTColorParamImportDlg
//
// カラーパラメータ入力ダイアログ
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// パラメータ定義
//******************************************************************************

//パラメータ文字列最大長
#define MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX		(2048)


//******************************************************************************
// カラーパラメータ入力ダイアログ
//******************************************************************************
class MTColorParamImportDlg
{
public:

	//コンストラクタ／デストラクタ
	MTColorParamImportDlg(void);
	virtual ~MTColorParamImportDlg(void);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

	//インポート実行フラグ取得
	bool IsExecImport();

	//パラメータ文字列取得
	TCHAR* GetParamString();

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTColorParamImportDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウハンドル
	HWND m_hWnd;

	//エディットボックス
	HWND m_hEditBox;

	//パラメータ文字列
	TCHAR m_ParamString[MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX];

	//インポート実行フラグ
	bool m_isExecImport;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//ペーストボタン押下
	int _OnBtnPaste();

};


