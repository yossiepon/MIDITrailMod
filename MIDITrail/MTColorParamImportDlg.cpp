//******************************************************************************
//
// MIDITrail / MTColorParamImportDlg
//
// カラーパラメータ入力ダイアログ
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "YNBaseLib.h"
#include "MTColorParamImportDlg.h"

using namespace YNBaseLib;


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTColorParamImportDlg* MTColorParamImportDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTColorParamImportDlg::MTColorParamImportDlg(void)
{
	m_pThis = this;
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_hEditBox = NULL;
	m_ParamString[0] = _T('\0');
	m_isExecImport = false;

	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTColorParamImportDlg::~MTColorParamImportDlg(void)
{
	return;
}

//******************************************************************************
// 表示
//******************************************************************************
int MTColorParamImportDlg::Show(
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
					MAKEINTRESOURCE(IDD_COLOR_PARAM_IMPORT),	//ダイアログボックステンプレート
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
//インポート実行フラグ取得
//******************************************************************************
bool MTColorParamImportDlg::IsExecImport()
{
	return m_isExecImport;
}

//******************************************************************************
//パラメータ文字列取得
//******************************************************************************
TCHAR* MTColorParamImportDlg::GetParamString()
{
	return m_ParamString;
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTColorParamImportDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTColorParamImportDlg::_WndProcImpl(
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
			if (LOWORD(wParam) == IDC_BTN_IMPORT) {
				m_ParamString[0] = _T('\0');
				GetWindowText(m_hEditBox, m_ParamString, MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX);
				m_isExecImport = true;
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDC_BTN_PASTE) {
				result = _OnBtnPaste();
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
int MTColorParamImportDlg::_OnInitDlg(HWND hDlg)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isExecImport = false;
	m_hEditBox = GetDlgItem(m_hWnd, IDC_EDIT_TEXT_IMPORT);

	//エディットボックスの最大入力文字数を制限する
	SendMessage(m_hEditBox, EM_SETLIMITTEXT, (WPARAM)(MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX - 1), 0);

//EXIT:;
	return result;
}

//******************************************************************************
// ペーストボタン押下
//******************************************************************************
int MTColorParamImportDlg::_OnBtnPaste()
{
	int result = 0;
	BOOL bresult = FALSE;
	HGLOBAL hGlobalMemory = NULL;
	TCHAR* pGlobalMemory = NULL;
	size_t length = 0;

	///クリップボードのデータ存在確認
	bresult = IsClipboardFormatAvailable(CF_TEXT);
	if (!bresult) {
		//テキストデータがなければ何もしない
		goto EXIT;
	}

	//クリップボードを開く
	bresult = OpenClipboard(m_hWnd);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//クリップボードのデータを取得
	hGlobalMemory = (HGLOBAL)GetClipboardData(CF_TEXT);
	if (hGlobalMemory == NULL) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//メモリロック
	pGlobalMemory = (LPSTR)GlobalLock(hGlobalMemory);
	if (pGlobalMemory == NULL) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//文字列サイズを確認
	length = _tcslen(pGlobalMemory);
	if (length >= MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX) {
		//クリップボードのデータが大きすぎるため
		CloseClipboard();
		result = YN_SET_ERR("The clipboad data is too long.", length, 0);
		goto EXIT;
	}

	//メモリにパラメータ文字列を書き込む
	_tcscpy_s(m_ParamString, MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX, pGlobalMemory);

	//メモリロック解除
	bresult = GlobalUnlock(hGlobalMemory);
	if ((!bresult) && (GetLastError() != NO_ERROR)) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//クリップボードを閉じる
	bresult = CloseClipboard();
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//パラメータ文字列表示
	bresult = SetWindowText(m_hEditBox, m_ParamString);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


