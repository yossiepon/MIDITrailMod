//******************************************************************************
//
// MIDITrail / MTAboutDlg
//
// About dialog.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "Resource.h"


//******************************************************************************
// Version information dialog class
//******************************************************************************
class MTAboutDlg
{
public:

	//Constructor / Destructor
	MTAboutDlg(void);
	virtual ~MTAboutDlg(void);

	//Show
	int Show(HWND hParentWnd);

private:

	//Pointer for window procedure control
	static MTAboutDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

};


