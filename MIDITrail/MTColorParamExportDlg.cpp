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
// Window procedure control parameter setup
//******************************************************************************
MTColorParamExportDlg* MTColorParamExportDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
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
// Destructor
//******************************************************************************
MTColorParamExportDlg::~MTColorParamExportDlg(void)
{
	return;
}

//******************************************************************************
// Register the parameter string
//******************************************************************************
void MTColorParamExportDlg::SetParamString(TCHAR* pString)
{
	_tcscat_s(m_ParamString, MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX, pString);
	return;
}

//******************************************************************************
// Show
//******************************************************************************
int MTColorParamExportDlg::Show(
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
					MAKEINTRESOURCE(IDD_COLOR_PARAM_EXPORT),	//Dialog box template
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
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTColorParamExportDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
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
int MTColorParamExportDlg::_OnInitDlg(HWND hDlg)
{
	int result = 0;
	BOOL bresult = FALSE;

	m_hWnd = hDlg;

	//Display the parameter string
	bresult = SetWindowText(GetDlgItem(m_hWnd, IDC_EDIT_TEXT_EXPORT), m_ParamString);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Copy button pressed
//******************************************************************************
int MTColorParamExportDlg::_OnBtnCopy()
{
	int result = 0;
	BOOL bresult = FALSE;
	HANDLE hData = NULL;

	HGLOBAL hGlobalMemory;
	TCHAR* pGlobalMemory = NULL;

	//Allocate memory
	hGlobalMemory = GlobalAlloc(GHND, MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX);
	if (hGlobalMemory == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Lock memory
	pGlobalMemory = (LPSTR)GlobalLock(hGlobalMemory);
	if (pGlobalMemory == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Write the parameter string into memory
	_tcscat_s(pGlobalMemory, MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX, m_ParamString);

	//Unlock memory
	bresult = GlobalUnlock(hGlobalMemory);
	if ((!bresult) && (GetLastError() != NO_ERROR)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Open the clipboard
	bresult = OpenClipboard(m_hWnd);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Clear the clipboard
	bresult = EmptyClipboard();
	if (!bresult) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Register the data with the clipboard
	hData = SetClipboardData(CF_TEXT, hGlobalMemory);
	if (hData == NULL) {
		CloseClipboard();
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Since the clipboard registration succeeded, memory management is handed off to the OS
	hGlobalMemory = NULL;

	//Close the clipboard
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


