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

#include "StdAfx.h"
#include "Resource.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTColorPaletteCfgDlg.h"

using DirectX::SimpleMath::Color;
#include "MTColorParamExportDlg.h"
#include "MTColorParamImportDlg.h"
#include <Commdlg.h>
#include <sstream> //for std::stringstream
#include <istream> //for std::getline

using namespace YNBaseLib;


//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MTColorPaletteCfgDlg* MTColorPaletteCfgDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MTColorPaletteCfgDlg::MTColorPaletteCfgDlg(void)
{
	unsigned long i = 0;
	
	m_pThis = this;
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_ColorPaletteNo = 0;
	m_isChanged = false;

	//Color settings: Start/End
	m_ColorStart = Color(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
	m_ColorEnd = Color(1.0f, 1.0f, 1.0f, 1.0f); //RGBA

	//Parameters for the color selection dialog
	for (i = 0; i < 16; i++) {
		m_CustColors[i] = RGB(255, 255, 255);
	}

	return;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTColorPaletteCfgDlg::~MTColorPaletteCfgDlg(void)
{
	return;
}

//******************************************************************************
// Set color palette
//******************************************************************************
void MTColorPaletteCfgDlg::SetColorPalette(
			MTColorPalette* pColorPalette, 
			MTColorPalette* pDefaultColorPalette,
		 	unsigned long colorPaletteNo
		 )
{
	m_ColorPalette.CopyFrom(pColorPalette);
	m_DefaultColorPalette.CopyFrom(pDefaultColorPalette);
	m_ColorPaletteNo = colorPaletteNo;
}

//******************************************************************************
// Get color palette
//******************************************************************************
void MTColorPaletteCfgDlg::GetColorPalette(MTColorPalette* pColorPalette)
{
	pColorPalette->CopyFrom(&m_ColorPalette);
}

//******************************************************************************
// Show
//******************************************************************************
int MTColorPaletteCfgDlg::Show(
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
					MAKEINTRESOURCE(IDD_COLOR_PALETTE_CFG),	//Dialog box template
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
// Check for parameter changes
//******************************************************************************
bool MTColorPaletteCfgDlg::IsChanged()
{
	return m_isChanged;
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTColorPaletteCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
//******************************************************************************
INT_PTR MTColorPaletteCfgDlg::_WndProcImpl(
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
				m_isChanged = true;
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_1) {
				result = _OnBtnColor(0);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_2) {
				result = _OnBtnColor(1);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_3) {
				result = _OnBtnColor(2);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_4) {
				result = _OnBtnColor(3);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_5) {
				result = _OnBtnColor(4);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_6) {
				result = _OnBtnColor(5);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_7) {
				result = _OnBtnColor(6);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_8) {
				result = _OnBtnColor(7);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_9) {
				result = _OnBtnColor(8);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_10) {
				result = _OnBtnColor(9);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_11) {
				result = _OnBtnColor(10);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_12) {
				result = _OnBtnColor(11);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_13) {
				result = _OnBtnColor(12);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_14) {
				result = _OnBtnColor(13);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_15) {
				result = _OnBtnColor(14);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_16) {
				result = _OnBtnColor(15);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_BG) {
				result = _OnBtnColor(16);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_GL) {
				result = _OnBtnColor(17);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_CT) {
				result = _OnBtnColor(18);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_START) {
				result = _OnBtnColor(19);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_END) {
				result = _OnBtnColor(20);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_SET_GRADATION_COLORS) {
				result = _OnBtnSetGradationColors();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_SET_DEFAULT_COLORS) {
				result = _OnBtnSetDefaultColors();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EXPORT_COLOR_PARAMETERS) {
				result = _OonBtnExportColorParameters();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_IMPORT_COLOR_PARAMETERS) {
				result = _OnBtnImportColorParameters();
				if (result != 0) goto EXIT;
			}
			break;
		case WM_DRAWITEM:
			//Draw color button
			result = _DrawColorButton((DRAWITEMSTRUCT*)lParam);
			if (result != 0) goto EXIT;
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
int MTColorPaletteCfgDlg::_OnInitDlg(HWND hDlg)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isChanged = false;
	TCHAR title[256] = {_T('\0')};

	//Initialize color button list
	_InitColorButtonList();

	//Initialize color buttons
	result = _InitColorButtons();
	if (result != 0) goto EXIT;

	//Initialize color text
	result = _InitColorText();
	if (result != 0) goto EXIT;

	//Window title
	_stprintf_s(title, 256, _T("Color Palette %u"), m_ColorPaletteNo);
	SetWindowText(m_hWnd, title);

	//Initialize the Start/End combo boxes
	result = _InitCombobox(GetDlgItem(m_hWnd, IDC_COMBO_START), 0);
	if (result != 0) goto EXIT;
	result = _InitCombobox(GetDlgItem(m_hWnd, IDC_COMBO_END), SM_MAX_CH_NUM - 1);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize color button list
//******************************************************************************
void MTColorPaletteCfgDlg::_InitColorButtonList()
{
	//Color button list
	m_hColorButtonList[0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1);
	m_hColorButtonList[1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2);
	m_hColorButtonList[2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3);
	m_hColorButtonList[3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4);
	m_hColorButtonList[4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5);
	m_hColorButtonList[5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6);
	m_hColorButtonList[6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_7);
	m_hColorButtonList[7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_8);
	m_hColorButtonList[8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_9);
	m_hColorButtonList[9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_10);
	m_hColorButtonList[10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_11);
	m_hColorButtonList[11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_12);
	m_hColorButtonList[12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_13);
	m_hColorButtonList[13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_14);
	m_hColorButtonList[14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_15);
	m_hColorButtonList[15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_16);
	m_hColorButtonList[16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_BG);
	m_hColorButtonList[17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_GL);
	m_hColorButtonList[18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_CT);
	m_hColorButtonList[19] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_START);
	m_hColorButtonList[20] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_END);
	//Color text list
	m_hColorTextList[0]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_1);
	m_hColorTextList[1]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_2);
	m_hColorTextList[2]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_3);
	m_hColorTextList[3]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_4);
	m_hColorTextList[4]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_5);
	m_hColorTextList[5]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_6);
	m_hColorTextList[6]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_7);
	m_hColorTextList[7]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_8);
	m_hColorTextList[8]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_9);
	m_hColorTextList[9]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_10);
	m_hColorTextList[10] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_11);
	m_hColorTextList[11] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_12);
	m_hColorTextList[12] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_13);
	m_hColorTextList[13] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_14);
	m_hColorTextList[14] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_15);
	m_hColorTextList[15] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_16);
	m_hColorTextList[16] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_BG);
	m_hColorTextList[17] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_GL);
	m_hColorTextList[18] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_CT);
	m_hColorTextList[19] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_START);
	m_hColorTextList[20] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_END);

	return;
}

//******************************************************************************
// Initialize color buttons
//******************************************************************************
int MTColorPaletteCfgDlg::_InitColorButtons()
{
	int result = 0;
	
	//Nothing to do here; the button is drawn upon receiving the WM_DRAWITEM message
	
	return result;
}

//******************************************************************************
// Initialize color text
//******************************************************************************
int MTColorPaletteCfgDlg::_InitColorText()
{
	int result = 0;
	unsigned long targetNo = 0;
	Color color;
	TCHAR hexColor[16] = {_T('\0')};
	BOOL bResult = FALSE;

	for (targetNo = 0; targetNo < SM_MAX_CH_NUM + 3 + 2; targetNo++) {
		//Get the current color
		result = _GetCurColor(targetNo, &color);
		if (result != 0) goto EXIT;

		//Set color text
		DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
		bResult = SetWindowText(m_hColorTextList[targetNo], hexColor);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Initialize combo box
//******************************************************************************
int MTColorPaletteCfgDlg::_InitCombobox(HWND hCombobox, int selectedIndex)
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long chNo = 0;
	TCHAR itemStr[16] = {_T('\0')};
	
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(itemStr, 16, _T("Ch.%u"), chNo + 1);
		lresult = SendMessage(hCombobox, CB_ADDSTRING, 0, (LPARAM)itemStr);
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		lresult = SendMessage(hCombobox, CB_SETITEMDATA, chNo, 0);
		if (lresult == CB_ERR) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), chNo);
			goto EXIT;
		}
	}
	
	//Set the selection state
	lresult = SendMessage(hCombobox, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Color button pressed
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnColor(unsigned long targetNo)
{
	int result = 0;
	Color color;
	Color newColor;
	bool isChoosed = false;
	TCHAR hexColor[16] = {_T('\0')};
	BOOL bResult = FALSE;

	//Get the current color
	result = _GetCurColor(targetNo, &color);
	if (result != 0) goto EXIT;

	//Show the color selection dialog
	newColor = color;
	result = _ShowChooseColorDlg(color, &newColor, &isChoosed);
	if (result != 0) goto EXIT;

	//If a new color was selected
	if (isChoosed) {
		//Set the current color
		result = _SetCurColor(targetNo, newColor);
		if (result != 0) goto EXIT;

		//Request a button redraw; updated via the WM_DRAWITEM message
		InvalidateRect(m_hColorButtonList[targetNo], NULL, FALSE);

		//Update color text
		DXColorUtil::MakeHexRGBAFromColor(newColor, hexColor, 16);
		bResult = SetWindowText(m_hColorTextList[targetNo], hexColor);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Gradation tool: Set Gradation Colors button pressed
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnSetGradationColors()
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long chNoStart = 0;
	unsigned long chNoEnd = 0;

	//Selected channel number: Start
	lresult = SendMessage(GetDlgItem(m_hWnd, IDC_COMBO_START), CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	chNoStart = (unsigned long)lresult;

	//Selected channel number: End
	lresult = SendMessage(GetDlgItem(m_hWnd, IDC_COMBO_END), CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	chNoEnd = (unsigned long)lresult;

	//Set gradation colors
	result = _SetGradationColor(chNoStart, chNoEnd, m_ColorStart, m_ColorEnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Parameter setup tool: Set Default Colors button pressed
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnSetDefaultColors()
{
	int result = 0;

	//Apply the default color palette
	m_ColorPalette.CopyFrom(&m_DefaultColorPalette);

	//Update color buttons
	result = _UpdateColorButtons();
	if (result != 0) goto EXIT;
	
	//Update color text
	result = _UpdateColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Parameter setup tool: Export Color Parameters button pressed
//******************************************************************************
int MTColorPaletteCfgDlg::_OonBtnExportColorParameters()
{
	int result = 0;
	TCHAR paramString[2048] = {_T('\0')};
	MTColorParamExportDlg colorParamExportDlg;
	
	//Generate the export parameter string
	result = _MakeColorParamForExport(paramString, 2048);
	if (result != 0) goto EXIT;

	//Set the export parameter string
	colorParamExportDlg.SetParamString(paramString);

	//Show the color parameter export dialog
	result = colorParamExportDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Parameter setup tool: Import Color Parameters button pressed
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnImportColorParameters()
{
	int result = 0;
	MTColorParamImportDlg colorParamImportDlg;

	//Show the color parameter import dialog
	result = colorParamImportDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//Process color parameter import
	if (colorParamImportDlg.IsExecImport()) {
		result = _ImportColorParam(colorParamImportDlg.GetParamString());
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update color buttons
//******************************************************************************
int MTColorPaletteCfgDlg::_UpdateColorButtons()
{
	int result = 0;
	BOOL bResult = FALSE;
	unsigned long i = 0;

	//Request a button redraw; updated via the WM_DRAWITEM message
	for (i = 0; i < (SM_MAX_CH_NUM + 3 + 2); i++) {
		bResult = InvalidateRect(m_hColorButtonList[i], NULL, FALSE);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), i);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Draw color button
//******************************************************************************
int MTColorPaletteCfgDlg::_DrawColorButton(DRAWITEMSTRUCT* pDrawItem)
{
	int result = 0;
	BOOL bResult = FALSE;
	bool isFound = false;
	unsigned long i = 0;
	unsigned long targetNo = 0;
	Color color;
	COLORREF bkColor1;
	COLORREF bkColor2;
	
	//Do nothing if the control type is not a button
	if (pDrawItem->CtlType != ODT_BUTTON) goto EXIT;

	//---------------------------------
	// Identify the button and color
	//---------------------------------	
	//Control ID
	for (i = 0; i < (SM_MAX_CH_NUM + 3 + 2); i++) {
		if (pDrawItem->CtlID == GetDlgCtrlID(m_hColorButtonList[i])) {
			//Identify the target button
			isFound = true;
			targetNo = i;
		}
	}
	//Do nothing if the target button was not found
	if (!isFound) goto EXIT;

	//Get color
	result = _GetCurColor(targetNo, &color);
	if (result != 0) goto EXIT;
	
	//---------------------------------
	// Draw the button
	//---------------------------------
	//Set the device context background color
	bkColor1 = SetBkColor(pDrawItem->hDC, RGB(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f));
	if (bkColor1 == CLR_INVALID) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//Draw the button rectangle
	bResult = ExtTextOut(
					pDrawItem->hDC,	//Device context
					0,				//X coordinate of the text reference point (logical coordinates)
					0,				//Y coordinate of the text reference point (logical coordinates)
					ETO_OPAQUE,		//Rectangle fill mode: fill using the current background color
					&(pDrawItem->rcItem),	//Logical coordinates of the rectangle
					NULL,			//Text to draw
					0,				//String length
					NULL			//Distance between origins of adjacent character cells
				);
	if (!bResult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//Draw the border reflecting the button's pressed state
	if (pDrawItem->itemState & ODS_SELECTED) {
		//Draw the sunken state
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_SUNKEN, BF_TOPLEFT | BF_BOTTOMRIGHT);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	else {
		//Draw the raised state
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_RAISED, BF_TOPLEFT | BF_BOTTOMRIGHT);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	
	//Restore the device context background color
	bkColor2 = SetBkColor(pDrawItem->hDC, bkColor1);
	if (bkColor2 == CLR_INVALID) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Update color text
//******************************************************************************
int MTColorPaletteCfgDlg::_UpdateColorText()
{
	int result = 0;

	result =_InitColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get color
//******************************************************************************
int MTColorPaletteCfgDlg::_GetCurColor(
		unsigned long targetNo,
		Color* pColor
	)
{
	int result = 0;

	if (targetNo < SM_MAX_CH_NUM) {
		//Ch.1 - 16
		m_ColorPalette.GetChColor(targetNo, pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 0) {
		//Background
		m_ColorPalette.GetBackgroundColor(pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 1) {
		//Grid line
		m_ColorPalette.GetGridLineColor(pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 2) {
		//Counter
		m_ColorPalette.GetCounterColor(pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 3) {
		//Start
		*pColor = m_ColorStart;
	}
	else if (targetNo == SM_MAX_CH_NUM + 4) {
		//End
		*pColor = m_ColorEnd;
	}
	else {
		result = YN_SET_ERR("Program error.", targetNo, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Set color
//******************************************************************************
int MTColorPaletteCfgDlg::_SetCurColor(
	unsigned long targetNo,
	Color color
)
{
	int result = 0;

	if (targetNo < SM_MAX_CH_NUM) {
		//Ch.1 - 16
		m_ColorPalette.SetChColor(targetNo, color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 0) {
		//Background
		m_ColorPalette.SetBackgroundColor(color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 1) {
		//Grid line
		m_ColorPalette.SetGridLineColor(color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 2) {
		//Counter
		m_ColorPalette.SetCounterColor(color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 3) {
		//Start
		m_ColorStart = color;
	}
	else if (targetNo == SM_MAX_CH_NUM + 4) {
		//End
		m_ColorEnd = color;
	}
	else {
		result = YN_SET_ERR("Program error.", targetNo, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Show the color selection dialog
//******************************************************************************
int MTColorPaletteCfgDlg::_ShowChooseColorDlg(
		Color color,
		Color* pNewColor,
		bool* pIsChoosed
	)
{
	int result = 0;
	CHOOSECOLOR cc;
	BOOL bResult = FALSE;
	DWORD dlgErrorCode;

	//Set up the color selection dialog
	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = m_hWnd;
	cc.hInstance = NULL;
	cc.rgbResult = RGB(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f);
	cc.lpCustColors = m_CustColors;
	cc.Flags = CC_FULLOPEN		//Display the whole dialog box
				| CC_RGBINIT;	//Use the color specified in the rgbResult member as the initial color
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;

	//Show the color selection dialog
	bResult = ChooseColor(&cc);
	if (!bResult) {
		dlgErrorCode = CommDlgExtendedError();
		if (dlgErrorCode == 0) {
			//Canceled
			*pIsChoosed = false;
		}
		else {
			//Error occurred
			result = YN_SET_ERR("Windows API error.", dlgErrorCode, 0);
			goto EXIT;
		}
	}
	else {
		//If a new color was selected
		*pNewColor = Color(
							GetRValue(cc.rgbResult) / 255.0f,
							GetGValue(cc.rgbResult) / 255.0f,
							GetBValue(cc.rgbResult) / 255.0f,
							1.0f
						);
		*pIsChoosed = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Set gradation colors
//******************************************************************************
int MTColorPaletteCfgDlg::_SetGradationColor(
		unsigned long chNoStart,
		unsigned long chNoEnd,
		Color colorStart,
		Color colorEnd
	)
{
	int result = 0;
	unsigned int chNo = 0;
	Color color;
	float ratio = 0.0f;

	if ((chNoStart >= SM_MAX_CH_NUM) || (chNoEnd >= SM_MAX_CH_NUM)) {
		//Do nothing
	}

	if (chNoStart == chNoEnd) {
		//Do nothing
	}
	else if (chNoStart < chNoEnd) {
		for (chNo = chNoStart; chNo <= chNoEnd; chNo++) {
			ratio = (float)(chNo - chNoStart) / (float)(chNoEnd - chNoStart);
			color = Color((colorEnd.x - colorStart.x) * ratio + colorStart.x,
				(colorEnd.y - colorStart.y) * ratio + colorStart.y,
				(colorEnd.z - colorStart.z) * ratio + colorStart.z,
				1.0f);
			//Set color
			m_ColorPalette.SetChColor(chNo, color);
		}
	}
	else {
		for (chNo = chNoEnd; chNo <= chNoStart; chNo++) {
			ratio = (float)(chNo - chNoEnd) / (float)(chNoStart - chNoEnd);
			color = Color((colorStart.x - colorEnd.x) * ratio + colorEnd.x,
				(colorStart.y - colorEnd.y) * ratio + colorEnd.y,
				(colorStart.z - colorEnd.z) * ratio + colorEnd.z,
				1.0f);
			//Set color
			m_ColorPalette.SetChColor(chNo, color);
		}
	}

	//Update color buttons
	result = _UpdateColorButtons();
	if (result != 0) goto EXIT;

	//Update color text
	result = _UpdateColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Generate the export parameter string
//******************************************************************************
int MTColorPaletteCfgDlg::_MakeColorParamForExport(TCHAR* pTextBuf, unsigned long bufSize)
{
	int result = 0;
	unsigned long chNo = 0;
	Color color;
	TCHAR hexColor[16] = { _T('\0') };
	TCHAR line[64] = {_T('\0')};

	if ((pTextBuf == NULL) || (bufSize < 2048)) {
		result = YN_SET_ERR("Program error.", bufSize, 0);
		goto EXIT;
	}

	pTextBuf[0] = _T('\0');

	//Generate parameter string: Ch.1-16
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		m_ColorPalette.GetChColor(chNo, &color);
		DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
		_stprintf_s(line, 64, _T("Ch-%02u-NoteRGBA=%s\r\n"), chNo + 1, hexColor);
		_tcscat_s(pTextBuf, bufSize, line);
	}

	//Generate parameter string: Background
	m_ColorPalette.GetBackgroundColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	_stprintf_s(line, 64, _T("BackGroundRGBA=%s\r\n"), hexColor);
	_tcscat_s(pTextBuf, bufSize, line);

	//Generate parameter string: Grid line
	m_ColorPalette.GetGridLineColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	_stprintf_s(line, 64, _T("GridLineRGBA=%s\r\n"), hexColor);
	_tcscat_s(pTextBuf, bufSize, line);

	//Generate parameter string: Counter
	m_ColorPalette.GetCounterColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	_stprintf_s(line, 64, _T("CounterRGBA=%s\r\n"), hexColor);
	_tcscat_s(pTextBuf, bufSize, line);

EXIT:;
	return result;
}

//******************************************************************************
// Process parameter import
//******************************************************************************
int MTColorPaletteCfgDlg::_ImportColorParam(TCHAR* pParamString)
{
	int result = 0;
	MTColorParamDictionary paramDictionary;

	//Build the parameter map
	result = _MakeImportKeyValueMap(pParamString, &paramDictionary);
	if (result != 0) goto EXIT;

	//Load parameters
	result = _LoadParam(&paramDictionary);
	if (result != 0) goto EXIT;

	//Update color buttons
	result = _UpdateColorButtons();
	if (result != 0) goto EXIT;

	//Update color text
	result = _UpdateColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Build the parameter map
//******************************************************************************
int MTColorPaletteCfgDlg::_MakeImportKeyValueMap(
		TCHAR* pParamString,
		MTColorParamDictionary* pParamDictionary
	)
{
	int result = 0;
	std::string paramString = pParamString;
	std::stringstream stream{ paramString };
	std::string line;
	std::string trimLine;
	std::string key;
	std::string value;
	std::string::size_type trimLeft;
	std::string::size_type trimRight;
	TCHAR* trimCharList = _T(" \t\r\n");
	MTColorParamDictionary::iterator itr;

	//Parse each line
	while (std::getline(stream, line, '\n')) {
		//Trim leading/trailing whitespace and line breaks
		trimLeft = line.find_first_not_of(trimCharList);
		trimRight = line.find_last_not_of(trimCharList);

		//If nothing remains after trimming, move to the next line
		if ((trimLeft == std::string::npos) || (trimRight == std::string::npos)) {
			continue;
		}

		//Detected a logic error
		if (trimLeft > trimRight) {
			result = YN_SET_ERR("Program error.", trimLeft, trimRight);
			goto EXIT;
		}

		//Ignore comment lines
		trimLine = line.substr(trimLeft, trimRight - trimLeft + 1);
		if ((trimLine.front() == _T('#')) || (trimLine.front() == _T(';'))) {
			//Treat as a comment line, ignore it, and move to the next line
			continue;
		}

		//Identify the key
		trimLeft = trimLine.find_first_of(_T("="));
		if ((trimLeft == std::string::npos) || (trimLeft == 0)) {
			//No delimiter found, or the delimiter is at the start; ignore and move to the next line
			continue;
		}
		key = trimLine.substr(0, trimLeft);

		//Identify the value
		if ((trimLeft + 1) == trimLine.length()) {
			//If the delimiter is at the end, the value is an empty string
			value = _T("");
		}
		else {
			//Treat everything after the delimiter as the value
			value = trimLine.substr(trimLeft + 1, trimLine.length() - trimLeft + 1);
			//If the value is enclosed in matching quotes, strip the quotes
			//If whitespace separates the delimiter from the quote, treat that whitespace as part of the value
			if (value.length() >= 2) {
				if ((value.front() == _T('\'')) && (value.back() == _T('\''))) {
					value = value.substr(1, value.length() - 2);
				}
				else if ((value.front() == _T('\"')) && (value.back() == _T('\"'))) {
					value = value.substr(1, value.length() - 2);
				}
			}
		}

		//Register in the map
		//If the key already exists, remove the existing entry
		itr = pParamDictionary->find(key);
		if (itr != pParamDictionary->end()) {
			pParamDictionary->erase(itr);
		}
		//Register the data
		pParamDictionary->insert(MTColorParamDictionaryPair(key, value));

		//OutputDebugString(_T("key:"));
		//OutputDebugString(key.c_str());
		//OutputDebugString(_T("\n"));
		//OutputDebugString(_T("value:"));
		//OutputDebugString(value.c_str());
		//OutputDebugString(_T("\n"));
	}

EXIT:;
	return result;
}

//******************************************************************************
// Load parameters
//******************************************************************************
int MTColorPaletteCfgDlg::_LoadParam(MTColorParamDictionary* pParamDictionary)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR hexColor[16] = { _T('\0') };
	TCHAR key[32] = { _T('\0') };
	const TCHAR* pValue = NULL;
	MTColorParamDictionary::iterator itr;
	Color color;
	unsigned long colorRGB;

	//Ch.1-16
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo + 1);
		itr = pParamDictionary->find(key);
		if (itr != pParamDictionary->end()) {
			pValue = (itr->second).c_str();
			if (_tcslen(pValue) == 8) {
				color = DXColorUtil::MakeColorFromHexRGBA(pValue);
				m_ColorPalette.SetChColor(chNo, color);
			}
		}
	}

	//Background
	itr = pParamDictionary->find(_T("BackGroundRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetBackgroundColor(color);
		}
	}
	//For backward compatibility, allow importing "BackGroundRGB" (without alpha) as defined in the ini file
	itr = pParamDictionary->find(_T("BackGroundRGB"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 6) {
			colorRGB = DXColorUtil::MakeColorFromHexRGB(pValue);
			color = Color(colorRGB);
			m_ColorPalette.SetBackgroundColor(color);
		}
	}

	//Grid line
	itr = pParamDictionary->find(_T("GridLineRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetGridLineColor(color);
		}
	}

	//Counter
	itr = pParamDictionary->find(_T("CounterRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetCounterColor(color);
		}
	}
	//For backward compatibility, allow importing "CaptionRGBA" as defined in the ini file
	itr = pParamDictionary->find(_T("CaptionRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetCounterColor(color);
		}
	}

//EXIT:;
	return result;
}


