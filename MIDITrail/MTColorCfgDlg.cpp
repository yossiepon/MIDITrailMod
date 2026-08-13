//******************************************************************************
//
// MIDITrail / MTColorCfgDlg
//
// Color configuration dialog.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "Resource.h"
#include "MTColorCfgDlg.h"


//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MTColorCfgDlg* MTColorCfgDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MTColorCfgDlg::MTColorCfgDlg(void)
{
	m_pThis = this;
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_SelectedColorPaletteNo = 0;
	m_isChanged = false;

	return;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTColorCfgDlg::~MTColorCfgDlg(void)
{
	return;
}

//******************************************************************************
// Show
//******************************************************************************
int MTColorCfgDlg::Show(
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
					MAKEINTRESOURCE(IDD_COLOR_CFG),		//Dialog box template
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
bool MTColorCfgDlg::IsChanged()
{
	return m_isChanged;
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTColorCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
//******************************************************************************
INT_PTR MTColorCfgDlg::_WndProcImpl(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	LRESULT lResult = 0;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG:
			result = _OnInitDlg(hDlg);
			if (result != 0) goto EXIT;
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				result = _Save();
				if (result != 0) goto EXIT;
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDC_RADIO_DEFAULT) {
				result = _OnBtnRadio(0);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_1) {
				result = _OnBtnRadio(1);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_2) {
				result = _OnBtnRadio(2);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_3) {
				result = _OnBtnRadio(3);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_4) {
				result = _OnBtnRadio(4);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_5) {
				result = _OnBtnRadio(5);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_6) {
				result = _OnBtnRadio(6);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_1) {
				result = _OnBtnEdit(1);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_2) {
				result = _OnBtnEdit(2);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_3) {
				result = _OnBtnEdit(3);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_4) {
				result = _OnBtnEdit(4);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_5) {
				result = _OnBtnEdit(5);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_6) {
				result = _OnBtnEdit(6);
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
	return lResult;
}

//******************************************************************************
// Pre-display dialog initialization
//******************************************************************************
int MTColorCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isChanged = false;

	//Initialize radio buttonlist
	_InitRadioButtonList();

	//Initialize color button list
	_InitColorButtonList();

	//Initialize color settings: display the PianoRoll3D.ini color settings as the default settings
	result = m_ColorConf.Initialize(_T("PianoRoll3D"));
	if (result != 0) goto EXIT;

	//Initialize radio button
	result = _InitRadioButtons();
	if (result != 0) goto EXIT;

	//Initialize color buttons
	result = _InitColorButtons();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize radio buttonlist
//******************************************************************************
void MTColorCfgDlg::_InitRadioButtonList()
{
	m_hBtnRadioPaletteList[0] = GetDlgItem(m_hWnd, IDC_RADIO_DEFAULT);
	m_hBtnRadioPaletteList[1] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_1);
	m_hBtnRadioPaletteList[2] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_2);
	m_hBtnRadioPaletteList[3] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_3);
	m_hBtnRadioPaletteList[4] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_4);
	m_hBtnRadioPaletteList[5] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_5);
	m_hBtnRadioPaletteList[6] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_6);
	
	return;
}

//******************************************************************************
// Initialize color button list
//******************************************************************************
void MTColorCfgDlg::_InitColorButtonList()
{
	//Color palette: default
	m_hBtnColorList[0][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_1);
	m_hBtnColorList[0][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_2);
	m_hBtnColorList[0][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_3);
	m_hBtnColorList[0][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_4);
	m_hBtnColorList[0][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_5);
	m_hBtnColorList[0][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_6);
	m_hBtnColorList[0][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_7);
	m_hBtnColorList[0][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_8);
	m_hBtnColorList[0][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_9);
	m_hBtnColorList[0][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_10);
	m_hBtnColorList[0][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_11);
	m_hBtnColorList[0][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_12);
	m_hBtnColorList[0][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_13);
	m_hBtnColorList[0][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_14);
	m_hBtnColorList[0][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_15);
	m_hBtnColorList[0][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_16);
	m_hBtnColorList[0][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_BG);
	m_hBtnColorList[0][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_GL);
	m_hBtnColorList[0][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_CT);
	//Color palette 1
	m_hBtnColorList[1][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_1);
	m_hBtnColorList[1][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_2);
	m_hBtnColorList[1][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_3);
	m_hBtnColorList[1][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_4);
	m_hBtnColorList[1][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_5);
	m_hBtnColorList[1][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_6);
	m_hBtnColorList[1][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_7);
	m_hBtnColorList[1][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_8);
	m_hBtnColorList[1][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_9);
	m_hBtnColorList[1][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_10);
	m_hBtnColorList[1][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_11);
	m_hBtnColorList[1][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_12);
	m_hBtnColorList[1][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_13);
	m_hBtnColorList[1][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_14);
	m_hBtnColorList[1][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_15);
	m_hBtnColorList[1][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_16);
	m_hBtnColorList[1][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_BG);
	m_hBtnColorList[1][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_GL);
	m_hBtnColorList[1][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_CT);
	//Color palette 2
	m_hBtnColorList[2][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_1);
	m_hBtnColorList[2][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_2);
	m_hBtnColorList[2][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_3);
	m_hBtnColorList[2][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_4);
	m_hBtnColorList[2][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_5);
	m_hBtnColorList[2][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_6);
	m_hBtnColorList[2][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_7);
	m_hBtnColorList[2][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_8);
	m_hBtnColorList[2][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_9);
	m_hBtnColorList[2][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_10);
	m_hBtnColorList[2][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_11);
	m_hBtnColorList[2][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_12);
	m_hBtnColorList[2][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_13);
	m_hBtnColorList[2][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_14);
	m_hBtnColorList[2][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_15);
	m_hBtnColorList[2][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_16);
	m_hBtnColorList[2][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_BG);
	m_hBtnColorList[2][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_GL);
	m_hBtnColorList[2][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_CT);
	//Color palette 3
	m_hBtnColorList[3][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_1);
	m_hBtnColorList[3][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_2);
	m_hBtnColorList[3][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_3);
	m_hBtnColorList[3][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_4);
	m_hBtnColorList[3][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_5);
	m_hBtnColorList[3][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_6);
	m_hBtnColorList[3][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_7);
	m_hBtnColorList[3][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_8);
	m_hBtnColorList[3][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_9);
	m_hBtnColorList[3][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_10);
	m_hBtnColorList[3][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_11);
	m_hBtnColorList[3][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_12);
	m_hBtnColorList[3][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_13);
	m_hBtnColorList[3][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_14);
	m_hBtnColorList[3][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_15);
	m_hBtnColorList[3][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_16);
	m_hBtnColorList[3][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_BG);
	m_hBtnColorList[3][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_GL);
	m_hBtnColorList[3][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_CT);
	//Color palette 4
	m_hBtnColorList[4][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_1);
	m_hBtnColorList[4][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_2);
	m_hBtnColorList[4][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_3);
	m_hBtnColorList[4][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_4);
	m_hBtnColorList[4][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_5);
	m_hBtnColorList[4][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_6);
	m_hBtnColorList[4][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_7);
	m_hBtnColorList[4][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_8);
	m_hBtnColorList[4][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_9);
	m_hBtnColorList[4][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_10);
	m_hBtnColorList[4][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_11);
	m_hBtnColorList[4][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_12);
	m_hBtnColorList[4][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_13);
	m_hBtnColorList[4][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_14);
	m_hBtnColorList[4][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_15);
	m_hBtnColorList[4][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_16);
	m_hBtnColorList[4][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_BG);
	m_hBtnColorList[4][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_GL);
	m_hBtnColorList[4][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_CT);
	//Color palette 5
	m_hBtnColorList[5][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_1);
	m_hBtnColorList[5][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_2);
	m_hBtnColorList[5][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_3);
	m_hBtnColorList[5][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_4);
	m_hBtnColorList[5][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_5);
	m_hBtnColorList[5][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_6);
	m_hBtnColorList[5][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_7);
	m_hBtnColorList[5][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_8);
	m_hBtnColorList[5][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_9);
	m_hBtnColorList[5][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_10);
	m_hBtnColorList[5][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_11);
	m_hBtnColorList[5][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_12);
	m_hBtnColorList[5][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_13);
	m_hBtnColorList[5][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_14);
	m_hBtnColorList[5][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_15);
	m_hBtnColorList[5][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_16);
	m_hBtnColorList[5][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_BG);
	m_hBtnColorList[5][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_GL);
	m_hBtnColorList[5][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_CT);
	//Color palette 6
	m_hBtnColorList[6][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_1);
	m_hBtnColorList[6][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_2);
	m_hBtnColorList[6][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_3);
	m_hBtnColorList[6][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_4);
	m_hBtnColorList[6][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_5);
	m_hBtnColorList[6][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_6);
	m_hBtnColorList[6][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_7);
	m_hBtnColorList[6][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_8);
	m_hBtnColorList[6][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_9);
	m_hBtnColorList[6][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_10);
	m_hBtnColorList[6][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_11);
	m_hBtnColorList[6][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_12);
	m_hBtnColorList[6][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_13);
	m_hBtnColorList[6][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_14);
	m_hBtnColorList[6][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_15);
	m_hBtnColorList[6][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_16);
	m_hBtnColorList[6][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_BG);
	m_hBtnColorList[6][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_GL);
	m_hBtnColorList[6][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_CT);
	
	return;
}

//******************************************************************************
// Initialize radio button
//******************************************************************************
int MTColorCfgDlg::_InitRadioButtons()
{
	int result = 0;
	unsigned long colorPaletteNo = 0;

	//Get the selected color palette number
	colorPaletteNo = m_ColorConf.GetSelectedColorPaletteNo();
	if (colorPaletteNo >= MT_COLOR_PALETTE_NUM_MAX) {
		result = YN_SET_ERR("Program error.", colorPaletteNo, 0);
		goto EXIT;
	}

	//Set radio buttonselectionstate
	SendMessage(m_hBtnRadioPaletteList[colorPaletteNo], BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	m_SelectedColorPaletteNo = colorPaletteNo;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize color buttons
//******************************************************************************
int MTColorCfgDlg::_InitColorButtons()
{
	int result = 0;
	
	//Nothing to do here; the button is drawn upon receiving the WM_DRAWITEM message
	
	return result;
}

//******************************************************************************
// radio button pressed
//******************************************************************************
int MTColorCfgDlg::_OnBtnRadio(unsigned long buttonNo)
{
	int result = 0;

	m_SelectedColorPaletteNo = buttonNo;

	//Register with the color settings
	result = m_ColorConf.SetSelectedColorPaletteNo(m_SelectedColorPaletteNo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Edit button pressed
//******************************************************************************
int MTColorCfgDlg::_OnBtnEdit(unsigned long paletteNo)
{
	int result = 0;
	
	//Show the color palette configuration dialog
	result = _ShowColorPaletteCfgDlg(paletteNo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Show the color palette configuration dialog
//******************************************************************************
int MTColorCfgDlg::_ShowColorPaletteCfgDlg(unsigned long colorPaletteNo)
{
	int result = 0;
	MTColorPalette colorPalette;
	MTColorPalette defaultColorPalette;

	//Get color palette
	result = m_ColorConf.GetColorPalette(colorPaletteNo, &colorPalette);
	if (result != 0) goto EXIT;
	result = m_ColorConf.GetColorPalette(0, &defaultColorPalette);
	if (result != 0) goto EXIT;

	//Set color palette
	m_ColorPaletteCfgDlg.SetColorPalette(&colorPalette, &defaultColorPalette, colorPaletteNo);

	//Show the dialog
	result = m_ColorPaletteCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;	

	//If changed, recreate the renderer and scene object
	if (m_ColorPaletteCfgDlg.IsChanged()) {
		//Get the updated color palette
		m_ColorPaletteCfgDlg.GetColorPalette(&colorPalette);
		//Register the color palette
		result = m_ColorConf.SetColorPalette(colorPaletteNo, &colorPalette);
		if (result != 0) goto EXIT;
		//Update color buttons
		result = _UpdateColorButtons(colorPaletteNo);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update color buttons
//******************************************************************************
int MTColorCfgDlg::_UpdateColorButtons(unsigned long colorPaletteNo)
{
	int result = 0;
	BOOL bResult = FALSE;
	unsigned long i = 0;

	//Request a button redraw; updated via the WM_DRAWITEM message
	for (i = 0; i < (SM_MAX_CH_NUM + 3); i++) {
		bResult = InvalidateRect(m_hBtnColorList[colorPaletteNo][i], NULL, FALSE);
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
int MTColorCfgDlg::_DrawColorButton(DRAWITEMSTRUCT* pDrawItem)
{
	int result = 0;
	BOOL bResult = FALSE;
	bool isFound = false;
	unsigned long i = 0;
	unsigned long j = 0;
	unsigned long paletteNo = 0;
	unsigned long chNo = 0;
	MTColorPalette colorPalette;
	DirectX::SimpleMath::Color color;
	COLORREF bkColor1;
	COLORREF bkColor2;
	
	//---------------------------------
	// Identify the button
	//---------------------------------
	//Do nothing if the control type is not a button
	if (pDrawItem->CtlType != ODT_BUTTON) goto EXIT;
	
	//Control ID
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		for (j = 0; j < (SM_MAX_CH_NUM + 3); j++) {
			if (pDrawItem->CtlID == GetDlgCtrlID(m_hBtnColorList[i][j])) {
				//Identify the target button
				isFound = true;
				paletteNo = i;
				chNo = j;
			}
		}
	}
	//Do nothing if the target button was not found
	if (!isFound) goto EXIT;
	
	//---------------------------------
	// Identify the color
	//---------------------------------
	//Get color palette
	result = m_ColorConf.GetColorPalette(paletteNo, &colorPalette);
	if (result != 0) goto EXIT;
	
	//Get color
	if (chNo < SM_MAX_CH_NUM) {
		//Ch.1 - 16
		colorPalette.GetChColor(chNo, &color);
	}
	else if (chNo == SM_MAX_CH_NUM + 0) {
		//Background
		colorPalette.GetBackgroundColor(&color);
	}
	else if (chNo == SM_MAX_CH_NUM + 1) {
		//Grid line
		colorPalette.GetGridLineColor(&color);
	}
	else if (chNo == SM_MAX_CH_NUM + 2) {
		//Counter
		colorPalette.GetCounterColor(&color);
	}
	
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
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_SUNKEN, BF_TOPLEFT | BF_BOTTOMRIGHT | BF_FLAT);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	else {
		//Draw the raised state
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_RAISED, BF_TOPLEFT | BF_BOTTOMRIGHT | BF_FLAT);
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
// Save color information
//******************************************************************************
int MTColorCfgDlg::_Save()
{
	int result = 0;
	
	//Save the color settings
	result = m_ColorConf.Save();
	if (result != 0) goto EXIT;
	
	m_isChanged = true;
	
EXIT:;
	return result;
}


