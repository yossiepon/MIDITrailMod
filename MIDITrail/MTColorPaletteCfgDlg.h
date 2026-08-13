//******************************************************************************
//
// MIDITrail / MTColorPaletteCfgDlg
//
// Color palette configuration dialog.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTColorConf.h"
#include <map>
#include <directxtk/SimpleMath.h>


//******************************************************************************
// Color configuration dialog class
//******************************************************************************
class MTColorPaletteCfgDlg
{
public:

	//Constructor / Destructor
	MTColorPaletteCfgDlg(void);
	virtual ~MTColorPaletteCfgDlg(void);

	//Set color palette
	void SetColorPalette(
				MTColorPalette* pColorPalette, 
				MTColorPalette* pDefaultColorPalette,
			 	unsigned long colorPaletteNo
			 );

	//Get color palette
	void GetColorPalette(MTColorPalette* pColorPalette);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

	//Check for parameter changes
	bool IsChanged();

private:

	//Pointer for window procedure control
	static MTColorPaletteCfgDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window handle
	HWND m_hWnd;

	//Color button list: Ch.1-16, BG/GL/CT, Start/End
	HWND m_hColorButtonList[SM_MAX_CH_NUM + 3 + 2];

	//Color text list: Ch.1-16, BG/GL/CT, Start/End
	HWND m_hColorTextList[SM_MAX_CH_NUM + 3 + 2];

	//Color palette
	MTColorPalette m_ColorPalette;
	MTColorPalette m_DefaultColorPalette;
	unsigned long m_ColorPaletteNo;
	DirectX::SimpleMath::Color m_ColorStart;
	DirectX::SimpleMath::Color m_ColorEnd;

	//Parameters for the color selection dialog
	COLORREF m_CustColors[16];

	//Change flag
	bool m_isChanged;

	//Color parameter map
	typedef std::map<std::string, std::string> MTColorParamDictionary;
	typedef std::pair<std::string, std::string> MTColorParamDictionaryPair;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Initialize color button list
	void _InitColorButtonList();

	//Initialize color buttons
	int _InitColorButtons();

	//Initialize color text
	int _InitColorText();

	//Initialize combo box
	int _InitCombobox(HWND hCombobox, int selectedIndex);

	//Color button pressed
	int _OnBtnColor(unsigned long targetNo);

	//Gradation tool: Set Gradation Colors button pressed
	int _OnBtnSetGradationColors();

	//Parameter setup tool: Set Default Colors button pressed
	int _OnBtnSetDefaultColors();

	//Parameter setup tool: Export Color Parameters button pressed
	int _OonBtnExportColorParameters();

	//Parameter setup tool: Import Color Parameters button pressed
	int _OnBtnImportColorParameters();

	//Update color buttons
	int _UpdateColorButtons();

	//Draw color button
	int _DrawColorButton(DRAWITEMSTRUCT* pDrawItem);

	//Update color text
	int _UpdateColorText();

	//Get color
	int _GetCurColor(unsigned long targetNo, DirectX::SimpleMath::Color* pColor);

	//Set color
	int _SetCurColor(unsigned long targetNo, DirectX::SimpleMath::Color color);

	//Show the color selection dialog
	int _ShowChooseColorDlg(
				DirectX::SimpleMath::Color color,
				DirectX::SimpleMath::Color* pNewColor,
				bool* pIsChoosed
			);

	//Set gradation colors
	int _SetGradationColor(
				unsigned long chNoStart,
				unsigned long chNoEnd,
				DirectX::SimpleMath::Color colorStart,
				DirectX::SimpleMath::Color colorEnd
			);

	//Generate the export parameter string
	int _MakeColorParamForExport(TCHAR* pTextBuf, unsigned long bufSize);

	//Process parameter import
	int _ImportColorParam(TCHAR* pParamString);

	//Build the parameter map
	int _MakeImportKeyValueMap(TCHAR* pParamString, MTColorParamDictionary* pParamDictionary);

	//Load parameters
	int _LoadParam(MTColorParamDictionary* pParamDictionary);

};


