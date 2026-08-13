//******************************************************************************
//
// MIDITrail / MTMIDIOUTCfgDlg
//
// MIDI OUT configuration dialog.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "Resource.h"
#include "MTParam.h"
#include "MTMIDIOUTCfgDlg.h"
#include <string>


//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MTMIDIOUTCfgDlg* MTMIDIOUTCfgDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MTMIDIOUTCfgDlg::MTMIDIOUTCfgDlg(void)
{
	m_pThis = this;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTMIDIOUTCfgDlg::~MTMIDIOUTCfgDlg(void)
{
	m_hComboDevA = NULL;
	m_hComboDevB = NULL;
	m_hComboDevC = NULL;
	m_hComboDevD = NULL;
	m_hComboDevE = NULL;
	m_hComboDevF = NULL;
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTMIDIOUTCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
//******************************************************************************
INT_PTR MTMIDIOUTCfgDlg::_WndProcImpl(
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
			if (LOWORD(wParam) == IDOK) {
				result = _Save();
				if (result != 0) goto EXIT;
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(hDlg);
	}
	return (INT_PTR)bresult;
}

//******************************************************************************
// Show
//******************************************************************************
int MTMIDIOUTCfgDlg::Show(
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
					MAKEINTRESOURCE(IDD_MIDIOUT_CFG),	//Dialog box template
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
// Pre-display dialog initialization
//******************************************************************************
int MTMIDIOUTCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	//Initialize config file
	result = _InitConfFile();

	//Initialize MIDI output device control
	result = m_MIDIOutDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	//Initialize the MIDI output device selection combo box
	m_hComboDevA = GetDlgItem(hDlg, IDC_COMBO_PORT_A);
	m_hComboDevB = GetDlgItem(hDlg, IDC_COMBO_PORT_B);
	m_hComboDevC = GetDlgItem(hDlg, IDC_COMBO_PORT_C);
	m_hComboDevD = GetDlgItem(hDlg, IDC_COMBO_PORT_D);
	m_hComboDevE = GetDlgItem(hDlg, IDC_COMBO_PORT_E);
	m_hComboDevF = GetDlgItem(hDlg, IDC_COMBO_PORT_F);

	result = _InitComboDev(m_hComboDevA, _T("PortA"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevB, _T("PortB"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevC, _T("PortC"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevD, _T("PortD"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevE, _T("PortE"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevF, _T("PortF"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize config file
//******************************************************************************
int MTMIDIOUTCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_MIDI);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

	result = m_ConfFile.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize the device selection combo box
//******************************************************************************
int MTMIDIOUTCfgDlg::_InitComboDev(
		HWND hComboDev,
		TCHAR* pPortName
	)
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long index = 0;
	unsigned long devNum = 0;
	int comboIndex = 0;
	int selectedIndex = -1;
	TCHAR devName[MAXPNAMELEN];
	std::string selectedProductName;
	std::string productName;

	//Get the user-selected device name
	result = m_ConfFile.GetStr(pPortName, devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;
	selectedProductName = devName;

	//If there is no user-selected device, select "(none)"
	if (selectedProductName == _T("")) {
		selectedIndex = 0;
	}

	//Add "(none)" to the combo box
	lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)_T("(none)"));
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
		goto EXIT;
	}
	comboIndex++;

	//Number of MIDI devices
	devNum = m_MIDIOutDevCtrl.GetDevNum();

	for (index = 0; index < devNum; index++) {
		//Get the MIDI OUT device name
		result = m_MIDIOutDevCtrl.GetDevProductName(index, productName);
		if (result != 0) goto EXIT;

		//Add the device name to the combo box
		lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)productName.c_str());
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
			goto EXIT;
		}
		if (selectedProductName == productName) {
			selectedIndex = comboIndex;
		}

		comboIndex++;
	}

	//Account for USB devices
	//If the user-selected device is not currently connected, append it to the end of the combo box
	if (selectedIndex < 0) {
		//Add the device name to the combo box
		lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)selectedProductName.c_str());
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
			goto EXIT;
		}
		selectedIndex = comboIndex;
		comboIndex++;
	}

	//Set the selection state
	lresult = SendMessage(hComboDev, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Save device selection information
//******************************************************************************
int MTMIDIOUTCfgDlg::_Save()
{
	int result = 0;

	result = _SavePortCfg(m_hComboDevA, _T("PortA"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevB, _T("PortB"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevC, _T("PortC"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevD, _T("PortD"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevE, _T("PortE"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevF, _T("PortF"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Save port settings
//******************************************************************************
int MTMIDIOUTCfgDlg::_SavePortCfg(
		HWND hComboDev,
		TCHAR* pPortName
	)
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long selectedIndex = 0;
	std::string selectedProductName;
	std::string productName;
	unsigned long devNum = 0;
	bool isUpdate = true;

	//Number of MIDI devices
	devNum = m_MIDIOutDevCtrl.GetDevNum();

	//Get the index of the selected item
	lresult = SendMessage(hComboDev, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
		goto EXIT;
	}
	selectedIndex = (unsigned long)lresult;

	//Get the device name of the selected item
	if (selectedIndex == 0) {
		selectedProductName = _T("");
	}
	else if (selectedIndex <= devNum) {
		result = m_MIDIOutDevCtrl.GetDevProductName((selectedIndex-1), selectedProductName);
		if (result != 0) goto EXIT;
	}
	else {
		//The currently-disconnected user-selected device appended at the end remains selected
		isUpdate = false;
	}

	//Save the setting
	if (isUpdate) {
		result = m_ConfFile.SetStr(pPortName, selectedProductName.c_str());
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

