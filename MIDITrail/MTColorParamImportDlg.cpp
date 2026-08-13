//******************************************************************************
//
// MIDITrail / MTColorParamImportDlg
//
// Color parameter import dialog.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "Resource.h"
#include "YNBaseLib.h"
#include "MTColorParamImportDlg.h"

using namespace YNBaseLib;


//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MTColorParamImportDlg* MTColorParamImportDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
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
// Destructor
//******************************************************************************
MTColorParamImportDlg::~MTColorParamImportDlg(void)
{
	return;
}

//******************************************************************************
// Show
//******************************************************************************
int MTColorParamImportDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	//Get the application instance handle
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//Show the dialog
	dresult = DialogBox(
					hInstance,							//Instance handle
					MAKEINTRESOURCE(IDD_COLOR_PARAM_IMPORT),	//Dialog box template
					hParentWnd,							//Parent window handle
					_WndProc							//Dialog box procedure
				);
	if ((dresult == 0) || (dresult == -1)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hInstance);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
//Get the import-execute flag
//******************************************************************************
bool MTColorParamImportDlg::IsExecImport()
{
	return m_isExecImport;
}

//******************************************************************************
//Get the parameter string
//******************************************************************************
TCHAR* MTColorParamImportDlg::GetParamString()
{
	return m_ParamString;
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTColorParamImportDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
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
			//Message not handled
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(hDlg);
	}
	return lresult;
}

//******************************************************************************
// Pre-display dialog initialization
//******************************************************************************
int MTColorParamImportDlg::_OnInitDlg(HWND hDlg)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isExecImport = false;
	m_hEditBox = GetDlgItem(m_hWnd, IDC_EDIT_TEXT_IMPORT);

	//Limit the maximum input length of the edit box
	SendMessage(m_hEditBox, EM_SETLIMITTEXT, (WPARAM)(MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX - 1), 0);

//EXIT:;
	return result;
}

//******************************************************************************
// Paste button pressed
//******************************************************************************
int MTColorParamImportDlg::_OnBtnPaste()
{
	int result = 0;
	BOOL bresult = FALSE;
	HGLOBAL hGlobalMemory = NULL;
	TCHAR* pGlobalMemory = NULL;
	size_t length = 0;

	///Check whether clipboard data exists
	bresult = IsClipboardFormatAvailable(CF_TEXT);
	if (!bresult) {
		//Do nothing if there is no text data
		goto EXIT;
	}

	//Open the clipboard
	bresult = OpenClipboard(m_hWnd);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Get the clipboard data
	hGlobalMemory = (HGLOBAL)GetClipboardData(CF_TEXT);
	if (hGlobalMemory == NULL) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Lock memory
	pGlobalMemory = (LPSTR)GlobalLock(hGlobalMemory);
	if (pGlobalMemory == NULL) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Check the string size
	length = _tcslen(pGlobalMemory);
	if (length >= MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX) {
		//Because the clipboard data is too large
		CloseClipboard();
		result = YN_SET_ERR("The clipboad data is too long.", length, 0);
		goto EXIT;
	}

	//Write the parameter string into memory
	_tcscpy_s(m_ParamString, MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX, pGlobalMemory);

	//Unlock memory
	bresult = GlobalUnlock(hGlobalMemory);
	if ((!bresult) && (GetLastError() != NO_ERROR)) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Close the clipboard
	bresult = CloseClipboard();
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Display the parameter string
	bresult = SetWindowText(m_hEditBox, m_ParamString);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


