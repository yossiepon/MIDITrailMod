//******************************************************************************
//
// MIDITrail / MTColorParamExportDlg
//
// Color parameter export dialog.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "Resource.h"
#include "YNBaseLib.h"
#include "MTColorParamExportDlg.h"

using namespace YNBaseLib;


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTColorParamExportDlg* MTColorParamExportDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTColorParamExportDlg::MTColorParamExportDlg(void)
{
	m_pThis = this;
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_ParamString[0] = _T('\0');

	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTColorParamExportDlg::~MTColorParamExportDlg(void)
{
	return;
}

//******************************************************************************
// パラメータ文字列登録
//******************************************************************************
void MTColorParamExportDlg::SetParamString(TCHAR* pString)
{
	_tcscat_s(m_ParamString, MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX, pString);
	return;
}

//******************************************************************************
// 表示
//******************************************************************************
int MTColorParamExportDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	//アプリケーションインスタンスハンドルを取得
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//ダイアログ表示
	dresult = DialogBox(
					hInstance,							//インスタンスハンドル
					MAKEINTRESOURCE(IDD_COLOR_PARAM_EXPORT),	//ダイアログボックステンプレート
					hParentWnd,							//親ウィンドウハンドル
					_WndProc							//ダイアログボックスプロシージャ
				);
	if ((dresult == 0) || (dresult == -1)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hInstance);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTColorParamExportDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTColorParamExportDlg::_WndProcImpl(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	LRESULT lresult = 0;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG:
			result = _OnInitDlg(hDlg);
			if (result != 0) goto EXIT;
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDC_BTN_COPY) {
				result = _OnBtnCopy();
				if (result != 0) goto EXIT;
			}
			break;
		default:
			//処理しないメッセージ
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(hDlg);
	}
	return lresult;
}

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTColorParamExportDlg::_OnInitDlg(HWND hDlg)
{
	int result = 0;
	BOOL bresult = FALSE;

	m_hWnd = hDlg;

	//パラメータ文字列表示
	bresult = SetWindowText(GetDlgItem(m_hWnd, IDC_EDIT_TEXT_EXPORT), m_ParamString);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// コピーボタン押下
//******************************************************************************
int MTColorParamExportDlg::_OnBtnCopy()
{
	int result = 0;
	BOOL bresult = FALSE;
	HANDLE hData = NULL;

	HGLOBAL hGlobalMemory;
	TCHAR* pGlobalMemory = NULL;

	//メモリ確保
	hGlobalMemory = GlobalAlloc(GHND, MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX);
	if (hGlobalMemory == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//メモリロック
	pGlobalMemory = (LPSTR)GlobalLock(hGlobalMemory);
	if (pGlobalMemory == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//メモリにパラメータ文字列を書き込む
	_tcscat_s(pGlobalMemory, MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX, m_ParamString);

	//メモリロック解除
	bresult = GlobalUnlock(hGlobalMemory);
	if ((!bresult) && (GetLastError() != NO_ERROR)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//クリップボードを開く
	bresult = OpenClipboard(m_hWnd);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//クリップボードをクリア
	bresult = EmptyClipboard();
	if (!bresult) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//クリップボードにデータを登録
	hData = SetClipboardData(CF_TEXT, hGlobalMemory);
	if (hData == NULL) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//クリップボードへのデータ登録が成功したためメモリ管理はOSに引き継がれる
	hGlobalMemory = NULL;

	//クリップボードを閉じる
	bresult = CloseClipboard();
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	if (hGlobalMemory != NULL) {
		GlobalFree(hGlobalMemory);
	}
	return result;
}


