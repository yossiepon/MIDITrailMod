//******************************************************************************
//
// MIDITrail / MTAboutDlg
//
// バージョン情報ダイアログクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTAboutDlg.h"

using namespace YNBaseLib;


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTAboutDlg* MTAboutDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTAboutDlg::MTAboutDlg(void)
{
	m_pThis = this;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTAboutDlg::~MTAboutDlg(void)
{
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTAboutDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTAboutDlg::_WndProcImpl(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	BOOL bresult = FALSE;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG:
			bresult = TRUE;
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			break;
	}

	return (INT_PTR)bresult;
}

//******************************************************************************
// 表示
//******************************************************************************
int MTAboutDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	//アプリケーションインスタンスハンドルを取得
	hInstance = (HINSTANCE)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//ダイアログ表示
	dresult = DialogBox(
					hInstance,						//インスタンスハンドル
					MAKEINTRESOURCE(IDD_ABOUTBOX),	//ダイアログボックステンプレート
					hParentWnd,						//親ウィンドウハンドル
					_WndProc						//ダイアログボックスプロシージャ
				);
	if ((dresult == 0) || (dresult == -1)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

