//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// Window size configuration dialog.
//
// Copyright (C) 2010-2016 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "Resource.h"
#include "YNBaseLib.h"
#include <list>

using namespace YNBaseLib;


//******************************************************************************
//Window size configuration dialog class parameter definitions
//******************************************************************************

//Minimum window size setting
#define MT_WINDOW_SIZE_MIN			200

//Maximum window size setting
#define MT_WINDOW_SIZE_MAX			99999

//Maximum window size character count: 99999 -> 5 digits
#define MT_WINDOW_SIZE_CHAR_MAX		5


//******************************************************************************
//Window size configuration dialog class
//******************************************************************************
class MTWindowSizeCfgDlg
{
public:

	//Constructor / Destructor
	MTWindowSizeCfgDlg(void);
	virtual ~MTWindowSizeCfgDlg(void);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

	//Check for changes
	bool IsChanged();

private:

	typedef struct {
		unsigned long width;
		unsigned long height;
	} MTWindowSizeItem;

	typedef std::list<MTWindowSizeItem> MTWindowSizeList;

private:

	//Pointer for window procedure control
	static MTWindowSizeCfgDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window size selection list box window handle
	HWND m_hSizeList;

	//Window size list
	MTWindowSizeList m_SizeList;

	//Window size edit box window handles
	HWND m_hEditWidth;
	HWND m_hEditHeight;

	//"Apply to view area" checkbox window handle
	HWND m_hCheckApplyToView;

	//Configuration file
	YNConfFile m_ConfFile;

	//Save-executed flag
	bool m_isSaved;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Initialize config file
	int _InitConfFile();

	//Initialize the window size selection list box
	int _InitSizeList();

	//Initialize the window size edit boxes
	int _InitSizeEditbox();

	//Get window size
	int _GetConfWindowSize(int* pWidth, int* pHeight);

	//Save processing
	int _Save();

	//Window size list box selection state changed
	int _OnSizeListChanged();

	//Update the window size edit boxes
	int _UpdateSizeEditBox(int width, int height);

};

