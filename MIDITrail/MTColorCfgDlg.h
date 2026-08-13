//******************************************************************************
//
// MIDITrail / MTColorCfgDlg
//
// Color configuration dialog.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "MTColorConf.h"
#include "MTColorPaletteCfgDlg.h"

using namespace YNBaseLib;


//******************************************************************************
// Color configuration dialog class
//******************************************************************************
class MTColorCfgDlg
{
public:

	//Constructor / Destructor
	MTColorCfgDlg(void);
	virtual ~MTColorCfgDlg(void);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

	//Check for parameter changes
	bool IsChanged();

private:

	//Pointer for window procedure control
	static MTColorCfgDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window handle
	HWND m_hWnd;

	//Radio button list
	HWND m_hBtnRadioPaletteList[MT_COLOR_PALETTE_NUM_MAX];

	//Color button list
	HWND m_hBtnColorList[MT_COLOR_PALETTE_NUM_MAX][SM_MAX_CH_NUM + 3];

	//Color information
	MTColorConf m_ColorConf;

	//Selected color palette number
	int m_SelectedColorPaletteNo;

	//Change flag
	bool m_isChanged;

	//Color palette configuration dialog
	MTColorPaletteCfgDlg m_ColorPaletteCfgDlg;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Initialize radio buttonlist
	void _InitRadioButtonList();

	//Initialize color button list
	void _InitColorButtonList();

	//Initialize radio button
	int _InitRadioButtons();

	//Initialize color buttons
	int _InitColorButtons();

	//radio button pressed
	int _OnBtnRadio(unsigned long buttonNo);

	//Edit button pressed
	int _OnBtnEdit(unsigned long paletteNo);

	//Show the color palette configuration dialog
	int _ShowColorPaletteCfgDlg(unsigned long colorPaletteNo);

	//Update color buttons
	int _UpdateColorButtons(unsigned long colorPaletteNo);

	//Draw color button
	int _DrawColorButton(DRAWITEMSTRUCT* pDrawItem);

	//Save color settings
	int _Save();

};


