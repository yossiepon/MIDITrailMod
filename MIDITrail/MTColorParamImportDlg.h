//******************************************************************************
//
// MIDITrail / MTColorParamImportDlg
//
// Color parameter import dialog.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// Parameter definitions
//******************************************************************************

//Maximum parameter string length
#define MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX		(2048)


//******************************************************************************
// Color parameter import dialog
//******************************************************************************
class MTColorParamImportDlg
{
public:

	//Constructor / Destructor
	MTColorParamImportDlg(void);
	virtual ~MTColorParamImportDlg(void);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

	//Get the import-execute flag
	bool IsExecImport();

	//Get the parameter string
	TCHAR* GetParamString();

private:

	//Pointer for window procedure control
	static MTColorParamImportDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window handle
	HWND m_hWnd;

	//Edit box
	HWND m_hEditBox;

	//Parameter string
	TCHAR m_ParamString[MT_COLOR_PARAM_IMPORT_STRING_LENGTH_MAX];

	//Import-execute flag
	bool m_isExecImport;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Paste button pressed
	int _OnBtnPaste();

};


