//******************************************************************************
//
// MIDITrail / MTGraphicCfgDlg
//
// Graphics configuration dialog.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "Resource.h"
#include "Commdlg.h"
#include "MTParam.h"
#include "MTGraphicCfgDlg.h"
#include <mbctype.h>


//******************************************************************************
// Graphics configuration dialog class parameter definitions
//******************************************************************************
//Note-length magnification: min/max
#define MT_QNOTE_LENGTH_MAG_MIN		(0)
#define MT_QNOTE_LENGTH_MAG_MAX		(1000)

//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MTGraphicCfgDlg* MTGraphicCfgDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MTGraphicCfgDlg::MTGraphicCfgDlg(void)
{
	unsigned long type = 0;

	m_pThis = this;
	m_hWnd = NULL;
	m_MultiSampleType = 0;
	m_hComboMultiSampleType = NULL;
	m_hEditImageFilePath = NULL;
	m_hEditQuarterNoteLengthMag = NULL;
	m_ImageFilePath[0] = L'\0';
	m_QuarterNoteLengthMag = 100;
	m_isChanged = false;

	for (type = 0; type < DX_MULTI_SAMPLE_TYPE_MAX; type++) {
		m_MultSampleTypeSupport[type] = false;
	}

	return;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTGraphicCfgDlg::~MTGraphicCfgDlg(void)
{
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTGraphicCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
//******************************************************************************
INT_PTR MTGraphicCfgDlg::_WndProcImpl(
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
				result = _Save();
				if (result != 0) goto EXIT;
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDC_BTN_BROWSE) {
				result = _OnBtnBrowse();
				if (result != 0) goto EXIT;
			}
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
// Set antialiasing support information
//******************************************************************************
void MTGraphicCfgDlg::SetAntialiasSupport(
		unsigned long multiSampleType,	//2-16
		bool isSupport
	)
{
	if ((DX_MULTI_SAMPLE_TYPE_MIN <= multiSampleType)
	 && (multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX)) {
		m_MultSampleTypeSupport[multiSampleType] = isSupport;
	}
	return;
}

//******************************************************************************
// Show
//******************************************************************************
int MTGraphicCfgDlg::Show(
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
	//  Display using the wide-string API so that UNICODE characters
	//  can be shown in the file path edit box
	dresult = DialogBoxW(
					hInstance,							//Instance handle
					MAKEINTRESOURCEW(IDD_GRAPHIC_CFG),	//Dialog box template
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
int MTGraphicCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isChanged = false;

	//Initialize config file
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//Load config file
	result = _LoadConf();
	if (result != 0) goto EXIT;

	//Initialize the multisample type selection combo box
	m_hComboMultiSampleType = GetDlgItem(hDlg, IDC_COMBO_MULTISAMPLETYPE);
	result = _InitComboMultiSampleType(m_hComboMultiSampleType, m_MultiSampleType);
	if (result != 0) goto EXIT;

	//Initialize the background image file path
	m_hEditImageFilePath = GetDlgItem(hDlg, IDC_EDIT_IMAGE_FILE_PATH);
	result = _InitBackgroundImageFilePath();
	if (result != 0) goto EXIT;

	//Initialize the quarter-note length magnification
	m_hEditQuarterNoteLengthMag = GetDlgItem(hDlg, IDC_EDIT_QUARTER_NOTE_LENGTH_MAG);
	result = _InitQuarterNote();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize config file
//******************************************************************************
int MTGraphicCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Load config file
//******************************************************************************
int MTGraphicCfgDlg::_LoadConf()
{
	int result = 0;
	int apiresult = 0;
	int multiSampleType = 0;
	TCHAR imageFilePathA[_MAX_PATH] = { _T('\0') };

	//Get the antialiasing setting value
	result = m_ConfFile.SetCurSection(_T("Anti-aliasing"));
	if (result != 0) goto EXIT;

	result = m_ConfFile.GetInt(
					_T("MultiSampleType"),
					&multiSampleType,
					MT_GRAPHIC_MULTI_SAMPLE_TYPE_DEF
				);
	if (result != 0) goto EXIT;

	//An invalid value turns antialiasing OFF
	if ((DX_MULTI_SAMPLE_TYPE_MIN <= multiSampleType)
	 && (multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX)) {
		m_MultiSampleType = multiSampleType;
	}
	else {
		m_MultiSampleType = 0;
	}

	//Get the background image file path setting value
	result = m_ConfFile.SetCurSection(_T("Background-image"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetWStr(_T("ImageFilePath_W"), m_ImageFilePath, _MAX_PATH, L"*** NO DATA ***");
	if (result != 0) goto EXIT;
	
	//If the wide-string file path is not set
	if (wcscmp(m_ImageFilePath, L"*** NO DATA ***") == 0) {
		//Since Ver.1.4.0, the wide-string file path is saved instead,
		//so try to get the multibyte-string file path
		memset(m_ImageFilePath, 0, sizeof(WCHAR) * _MAX_PATH);
		result = m_ConfFile.GetStr(_T("ImageFilePath"), imageFilePathA, _MAX_PATH, _T(""));
		if (result != 0) goto EXIT;
		if (_tcslen(imageFilePathA) > 0) {
			apiresult = MultiByteToWideChar(
								_getmbcp(),			//Code page
								MB_PRECOMPOSED,		//Flags:
								imageFilePathA,	//Source multibyte string
								(int)_tcslen(imageFilePathA),	//Source multibyte string byte count
								m_ImageFilePath,	//Destination wide-string buffer
								_MAX_PATH - 1		//Buffer size (in wide characters)
							);
			if (apiresult == 0) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
				goto EXIT;
			}
		}
	}

	//Get the quarter-note length magnification setting value
	result = m_ConfFile.SetCurSection(_T("QuarterNote"));
	if (result != 0) goto EXIT;
	
	result = m_ConfFile.GetInt(_T("LengthMagnification"), &m_QuarterNoteLengthMag, 100);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// Initialize the device selection combo box
//******************************************************************************
int MTGraphicCfgDlg::_InitComboMultiSampleType(
		HWND hCombo,
		unsigned long selMultiSampleType
	)
{
	int result = 0;
	LRESULT lresult = 0;
	int comboIndex = 0;
	int selectedIndex = -1;
	unsigned long type = 0;
	bool isSupportAA = false;
	TCHAR itemStr[256];

	//Check antialiasing support
	for (type = DX_MULTI_SAMPLE_TYPE_MIN; type <= DX_MULTI_SAMPLE_TYPE_MAX; type++) {
		if (m_MultSampleTypeSupport[type]) {
			isSupportAA = true;
		}
	}

	//Register the first item
	if (isSupportAA) {
		_stprintf_s(itemStr, 256, _T("OFF"));
	}
	else {
		_stprintf_s(itemStr, 256, _T("Not supported"));
	}
	lresult = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)itemStr);
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(hCombo, CB_SETITEMDATA, comboIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}
	selectedIndex = comboIndex;
	comboIndex++;

	//Register the multisample types
	for (type = DX_MULTI_SAMPLE_TYPE_MIN; type <= DX_MULTI_SAMPLE_TYPE_MAX; type++) {
		if (m_MultSampleTypeSupport[type]) {
			//Add the multisample type to the combo box
			_stprintf_s(itemStr, 256, _T("%dx"), type);
			lresult = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)itemStr);
			if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
				goto EXIT;
			}
			lresult = SendMessage(hCombo, CB_SETITEMDATA, comboIndex, type);
			if (lresult == CB_ERR) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
				goto EXIT;
			}
			if (type == selMultiSampleType) {
				selectedIndex = comboIndex;
			}
			comboIndex++;
		}
	}

	//Set the selection state
	lresult = SendMessage(hCombo, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

	//Disable if antialiasing is not supported
	if (!isSupportAA) {
		EnableWindow(hCombo, FALSE);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Initialize the background image file path
//******************************************************************************
int MTGraphicCfgDlg::_InitBackgroundImageFilePath()
{
	int result = 0;
	BOOL bresult = FALSE;

	//Set the maximum input length for the edit box
	SendMessage(m_hEditImageFilePath, EM_SETLIMITTEXT, (WPARAM)_MAX_PATH, 0);

	//Set the file path into the edit box
	bresult = SetWindowTextW(m_hEditImageFilePath, m_ImageFilePath);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Initialize quarter-note settings
//******************************************************************************
int MTGraphicCfgDlg::_InitQuarterNote()
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR str[32] = { _T('\0') };

	//Set the maximum number of input characters for the edit box: max 4 characters ("1000")
	SendMessage(m_hEditQuarterNoteLengthMag, EM_SETLIMITTEXT, (WPARAM)4, 0);

	//Set the quarter-note length magnification numeric string into the edit box
	_stprintf_s(str, 32, _T("%d"), m_QuarterNoteLengthMag);
	bresult = SetWindowText(m_hEditQuarterNoteLengthMag, str);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), m_QuarterNoteLengthMag);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Save configuration information
//******************************************************************************
int MTGraphicCfgDlg::_Save()
{
	int result = 0;
	int apiresult = 0;
	LRESULT lresult = 0;
	unsigned long selectedIndex = 0;
	unsigned long selectedMultiSampleType = 0;
	WCHAR filePath[_MAX_PATH] = { L'\0' };
	TCHAR strMag[32] = { _T('\0') };
	int mag = 0;

	//------------------------------
	//Antialiasing
	//------------------------------
	//Get the index of the selected item
	lresult = SendMessage(m_hComboMultiSampleType, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	selectedIndex = (unsigned long)lresult;

	//Get the user data of the selected item: multisample type
	lresult = SendMessage(m_hComboMultiSampleType, CB_GETITEMDATA, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}
	selectedMultiSampleType = (unsigned long)lresult;

	//Save the antialiasing setting
	result = m_ConfFile.SetCurSection(_T("Anti-aliasing"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("MultiSampleType"), selectedMultiSampleType);
	if (result != 0) goto EXIT;

	//Check for changes
	if (m_MultiSampleType != selectedMultiSampleType) {
		m_isChanged = true;
	}
	m_MultiSampleType = selectedMultiSampleType;

	//------------------------------
	//Background image file path
	//------------------------------
	//Get the background image file path from the edit box
	apiresult = GetWindowTextW(m_hEditImageFilePath, filePath, _MAX_PATH);
	if (apiresult == 0) {
		//If there is no text or the window handle is invalid
		filePath[0] = L'\0';
	}

	//Save the background image file path setting
	result = m_ConfFile.SetCurSection(_T("Background-image"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetWStr(_T("ImageFilePath_W"), filePath);
	if (result != 0) goto EXIT;

	//Check for changes
	if (wcscmp(m_ImageFilePath, filePath) != 0) {
		m_isChanged = true;
	}
	wcscpy_s(m_ImageFilePath, _MAX_PATH, filePath);
	
	//------------------------------
	//Quarter-note length magnification
	//------------------------------
	//Get the quarter-note length magnification
	apiresult = GetWindowText(m_hEditQuarterNoteLengthMag, strMag, 32);
	if (apiresult == 0) {
		//If there is no text or the window handle is invalid
		mag = 100;
	}
	else {
		mag = _tstoi(strMag);
	}

	//Clipping
	if (mag < MT_QNOTE_LENGTH_MAG_MIN) {
		mag = MT_QNOTE_LENGTH_MAG_MIN;
	}
	if (mag > MT_QNOTE_LENGTH_MAG_MAX) {
		mag = MT_QNOTE_LENGTH_MAG_MAX;
	}
	
	//Save the quarter-note length magnification
	result = m_ConfFile.SetCurSection(_T("QuarterNote"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("LengthMagnification"), mag);
	if (result != 0) goto EXIT;

	//Check for changes
	if (m_QuarterNoteLengthMag != mag) {
		m_isChanged = true;
	}
	m_QuarterNoteLengthMag = mag;

EXIT:;
	return result;
}

//******************************************************************************
// Check for parameter changes
//******************************************************************************
bool MTGraphicCfgDlg::IsChanged()
{
	return m_isChanged;
}

//******************************************************************************
// Background image file path browse button pressed
//******************************************************************************
int MTGraphicCfgDlg::_OnBtnBrowse()
{
	int result = 0;
	BOOL bresult = FALSE;
	WCHAR filePath[_MAX_PATH] = { L'\0' };
	bool isSelected = false;

	//Show the file selection dialog
	result = _SelectImageFile(filePath, _MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//Do nothing if no file was selected
	if (!isSelected) goto EXIT;

	//Set the file path into the edit box
	bresult = SetWindowTextW(m_hEditImageFilePath, filePath);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Select image file
//******************************************************************************
int MTGraphicCfgDlg::_SelectImageFile(
		WCHAR* pFilePath,
		unsigned long bufSize,
		bool* pIsSelected
	)
{
	int result = 0;
	BOOL apiresult = FALSE;
	OPENFILENAMEW ofn;

	if ((pFilePath == NULL) || (bufSize == 0) || (pIsSelected ==NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pFilePath[0] = L'\0';
	ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner   = m_hWnd;
	ofn.lpstrFilter = L"Image file (*.jpg *.png *.bmp)\0*.jpg;*.png;*.bmp\0";
	ofn.lpstrFile   = pFilePath;
	ofn.nMaxFile    = bufSize;
	ofn.lpstrTitle  = L"Select image file.";
	ofn.Flags       = OFN_FILEMUSTEXIST;  //OFN_HIDEREADONLY

	//Show the file selection dialog
	apiresult = GetOpenFileNameW(&ofn);
	if (!apiresult) {
		//Canceled or an error occurred: the error is not checked
		*pIsSelected = false;
		goto EXIT;
	}

	*pIsSelected = true;

EXIT:;
	return result;
}

