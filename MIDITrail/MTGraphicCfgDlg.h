//******************************************************************************
//
// MIDITrail / MTGraphicCfgDlg
//
// Graphics configuration dialog.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "DXRenderer11.h"

using namespace YNBaseLib;


//******************************************************************************
// Graphics configuration definitions
//******************************************************************************
//Antialiasing: default multisample type
#define MT_GRAPHIC_MULTI_SAMPLE_TYPE_DEF  (0)  //OFF


//******************************************************************************
// Graphics configuration dialog class
//******************************************************************************
class MTGraphicCfgDlg
{
public:

	//Constructor / Destructor
	MTGraphicCfgDlg(void);
	virtual ~MTGraphicCfgDlg(void);

	//Set antialiasing support information
	void SetAntialiasSupport(unsigned long multiSampleType, bool isSupport);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

	//Check for parameter changes
	bool IsChanged();

private:

	//Pointer for window procedure control
	static MTGraphicCfgDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window handle
	HWND m_hWnd;

	//Configuration file
	YNConfFile m_ConfFile;

	//Combo box window handle
	HWND m_hComboMultiSampleType;
	bool m_MultSampleTypeSupport[DX_MULTI_SAMPLE_TYPE_MAX+1];

	//Background image file path edit box window handle
	HWND m_hEditImageFilePath;
	
	//Quarter-note length magnification edit box window handle
	HWND m_hEditQuarterNoteLengthMag;

	//Antialiasing setting
	unsigned long m_MultiSampleType;

	//Background image file path
	WCHAR m_ImageFilePath[_MAX_PATH];
	
	//Quarter-note length magnification
	int m_QuarterNoteLengthMag;

	//Update flag
	bool m_isChanged;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Initialize config file
	int _InitConfFile();

	//Load config file
	int _LoadConf();

	//Initialize the device selection combo box
	int _InitComboMultiSampleType(HWND hCombo, unsigned long selMultiSampleType);

	//Initialize the background image file path
	int _InitBackgroundImageFilePath();
	
	//Initialize quarter-note settings
	int _InitQuarterNote();

	//Save processing
	int _Save();

	//Background image file path browse button pressed
	int _OnBtnBrowse();

	//Select image file
	int _SelectImageFile(WCHAR* pFilePath, unsigned long bufSize, bool* pIsSelected);

};


