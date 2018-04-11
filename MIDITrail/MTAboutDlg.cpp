//******************************************************************************
//
// MIDITrail / MTAboutDlg
//
// バージョン情報ダイアログクラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MIDITrailVersion.h"
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
	int result = 0;
	BOOL bresult = FALSE;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG:
			result = _OnInitDlg(hDlg);
			if (result != 0) goto EXIT;
			bresult = TRUE;
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			break;
	}

EXIT:;
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
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
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

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTAboutDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR* pVersion = NULL;
	TCHAR* pCopyright = NULL;

	//バージョン文字列
#ifdef _WIN64
	//64bit
	pVersion = MIDITRAIL_VERSION_STRING_X64;
#else
	//32bit
	pVersion = MIDITRAIL_VERSION_STRING_X86;
#endif

	//コピーライト文字列
	pCopyright = MIDITRAIL_COPYRIGHT;

	//バージョン文字列設定
	bresult = SetWindowText(GetDlgItem(hDlg, IDC_TEXT_VERSION), pVersion);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//コピーライト文字列設定
	bresult = SetWindowText(GetDlgItem(hDlg, IDC_TEXT_COPYRIGHT), pCopyright);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


