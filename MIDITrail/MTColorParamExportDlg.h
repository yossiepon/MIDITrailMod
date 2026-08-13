//******************************************************************************
//
// MIDITrail / MTColorParamExportDlg
//
// Color parameter export dialog.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// Parameter definitions
//******************************************************************************

//Maximum parameter string length
#define MT_COLOR_PARAM_EXPORT_STRING_LENGTH_MAX		(2048)


//******************************************************************************
// Color parameter export dialog
//******************************************************************************
class MTColorParamExportDlg
{
public:

	//Constructor / Destructor
	MTColorParamExportDlg(void);
	virtual ~MTColorParamExportDlg(void);

	//Register the parameter string
	void SetParamString(TCHAR* pString);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

private:

	//Pointer for window procedure control
	static MTColorParamExportDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window handle
	HWND m_hWnd;

	//Parameter string
	TCHAR m_ParamString[2048];

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Copy button pressed
	int _OnBtnCopy();

};


