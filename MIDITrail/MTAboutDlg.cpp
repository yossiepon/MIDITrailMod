//******************************************************************************
//
// MIDITrail / MTAboutDlg
//
// About dialog.
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
// Window procedure control parameter setup
//******************************************************************************
MTAboutDlg* MTAboutDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MTAboutDlg::MTAboutDlg(void)
{
	m_pThis = this;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTAboutDlg::~MTAboutDlg(void)
{
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTAboutDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
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
// Show
//******************************************************************************
int MTAboutDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	//Get the application instance handle
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Show the dialog
	dresult = DialogBox(
					hInstance,						//Instance handle
					MAKEINTRESOURCE(IDD_ABOUTBOX),	//Dialog box template
					hParentWnd,						//Parent window handle
					_WndProc						//Dialog box procedure
				);
	if ((dresult == 0) || (dresult == -1)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Pre-display dialog initialization
//******************************************************************************
int MTAboutDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	const WCHAR* pVersion = NULL;
	const WCHAR* pCopyright = NULL;

	//Version string
#ifdef _WIN64
	//64bit
	pVersion = MIDITRAIL_VERSION_STRING_X64;
#else
	//32bit
	pVersion = MIDITRAIL_VERSION_STRING_X86;
#endif

	//Copyright string
	pCopyright = MIDITRAIL_COPYRIGHT;

	//Set the version string
	bresult = SetWindowTextW(GetDlgItem(hDlg, IDC_TEXT_VERSION), pVersion);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Set the copyright string
	bresult = SetWindowTextW(GetDlgItem(hDlg, IDC_TEXT_COPYRIGHT), pCopyright);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


