//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// Window size configuration dialog.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTParam.h"
#include "MTWindowSizeCfgDlg.h"


//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MTWindowSizeCfgDlg* MTWindowSizeCfgDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MTWindowSizeCfgDlg::MTWindowSizeCfgDlg(void)
{
	m_pThis = this;
	m_hSizeList = NULL;
	m_hEditWidth = NULL;
	m_hEditHeight = NULL;
	m_hCheckApplyToView = NULL;
	m_isSaved = false;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTWindowSizeCfgDlg::~MTWindowSizeCfgDlg(void)
{
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTWindowSizeCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
//******************************************************************************
INT_PTR MTWindowSizeCfgDlg::_WndProcImpl(
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
				m_isSaved = true;
				result = _Save();
				if (result != 0) goto EXIT;
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			//List box
			else if (LOWORD(wParam) == IDC_WINDOWSIZE_LIST) {
				//Selection state changed
				if  (HIWORD(wParam) == LBN_SELCHANGE){
					result = _OnSizeListChanged();
					if (result != 0) goto EXIT;
				}
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
int MTWindowSizeCfgDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	m_isSaved = false;

	//Get the application instance handle
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//Show the dialog
	dresult = DialogBox(
					hInstance,							//Instance handle
					MAKEINTRESOURCE(IDD_WINDOWSIZE_CFG),//Dialog box template
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
// Check for changes
//******************************************************************************
bool MTWindowSizeCfgDlg::IsChanged()
{
	//Ideally this should check for actual value changes
	return m_isSaved;
}

//******************************************************************************
// Pre-display dialog initialization
//******************************************************************************
int MTWindowSizeCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	//Initialize config file
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//Get window handles
	m_hSizeList = GetDlgItem(hDlg, IDC_WINDOWSIZE_LIST);
	m_hEditWidth = GetDlgItem(hDlg, IDC_EDIT_WIDTH);
	m_hEditHeight = GetDlgItem(hDlg, IDC_EDIT_HEIGHT);
	m_hCheckApplyToView = GetDlgItem(hDlg, IDC_CHECK_APPLY_TO_VIEW);

	//Initialize the window size selection combo box
	result = _InitSizeList();
	if (result != 0) goto EXIT;

	//Initialize the window size edit boxes
	result = _InitSizeEditbox();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize config file
//******************************************************************************
int MTWindowSizeCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_VIEW);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

	result = m_ConfFile.SetCurSection(_T("WindowSize"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize the window size selection list box
//******************************************************************************
int MTWindowSizeCfgDlg::_InitSizeList()
{
	int result = 0;
	LRESULT lresult = 0;
	BOOL bresult = FALSE;
	unsigned long index = 0;
	int selectedIndex = -1;
	int curWidth = 0;
	int curHeight = 0;
	DEVMODE devMode;
	TCHAR caption[64];
	MTWindowSizeItem item;
	MTWindowSizeList::iterator itr;
	bool isExist = false;

	//Get window size
	result = _GetConfWindowSize(&curWidth, &curHeight);
	if (result != 0) goto EXIT;

	m_SizeList.clear();

	for (index = 0; ; index++) {

		//Get graphics mode information
		bresult = EnumDisplaySettings(
						NULL,		//Target display device
						index,		//Graphics mode index
						&devMode	//Retrieved graphics mode
					);
		if (!bresult) {
			//Finished retrieving the mode list
			break;
		}

		//Ignore anything other than 32-bit color depth
		if (devMode.dmBitsPerPel != 32) {
			continue;
		}

		//Check whether it is already registered in the list
		isExist = false;
		for (itr = m_SizeList.begin(); itr != m_SizeList.end(); itr++) {
			if ((itr->width == devMode.dmPelsWidth)
			 && (itr->height == devMode.dmPelsHeight)) {
				isExist = true;
				break;
			}
		}

		//Register in the list
		if (!isExist) {
			item.width = devMode.dmPelsWidth;
			item.height = devMode.dmPelsHeight;
			m_SizeList.push_back(item);
			
			//Build the caption
			_stprintf_s(caption, 64, _T("%d x %d  32bit"), item.width, item.height);

			//Add the window size to the list box
			lresult = SendMessage(m_hSizeList, LB_ADDSTRING, 0, (LPARAM)caption);
			if ((lresult == LB_ERR) || (lresult == LB_ERRSPACE)) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hSizeList);
				goto EXIT;
			}

			if ((item.width == curWidth) && (item.height == curHeight)) {
				selectedIndex = (int)(m_SizeList.size() - 1);
			}
		}
	}

	//If a matching size is found in the list, select it
	if (selectedIndex >= 0) {
		lresult = SendMessage(m_hSizeList, LB_SETCURSEL, selectedIndex, 0);
		if (lresult == LB_ERR) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Initialize the window size edit boxes
//******************************************************************************
int MTWindowSizeCfgDlg::_InitSizeEditbox()
{
	int result = 0;
	LRESULT lresult = 0;
	int width = 0;
	int height = 0;
	int maxsize = 0;
	int applyToViewArea = 0;

	//Get window size
	result = _GetConfWindowSize(&width, &height);
	if (result != 0) goto EXIT;

	//Set the maximum input length: 5 digits, up to 99,999
	maxsize = sizeof(TCHAR) * MT_WINDOW_SIZE_CHAR_MAX;
	SendMessage(m_hEditWidth, EM_SETLIMITTEXT, (WPARAM)maxsize, 0);
	SendMessage(m_hEditHeight, EM_SETLIMITTEXT, (WPARAM)maxsize, 0);

	//Set the window size number string into the edit boxes
	result = _UpdateSizeEditBox(width, height);
	if (result != 0) goto EXIT;

	//Get the "apply to view area" flag
	result = m_ConfFile.GetInt(_T("ApplyToViewArea"), &applyToViewArea, 0);
	if (result != 0) goto EXIT;

	//Initialize the "apply to view area" checkbox
	if (applyToViewArea == 0) {
		SendMessage(m_hCheckApplyToView, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	else {
		SendMessage(m_hCheckApplyToView, BM_SETCHECK, BST_CHECKED, 0);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get the window size setting value
//******************************************************************************
int MTWindowSizeCfgDlg::_GetConfWindowSize(
		int* pWidth,
		int* pHeight
	)
{
	int result = 0;
	int width = 0;
	int height = 0;

	if ((pWidth == NULL) || (pHeight == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Get the window size setting value
	result = m_ConfFile.GetInt(_T("Width"), &width, 0);
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetInt(_T("Height"), &height, 0);
	if (result != 0) goto EXIT;

	//If the size is invalid, reset it to the initial-launch window size
	if ((width <= 0)
	  || (height <= 0)
	  || (width > MT_WINDOW_SIZE_MAX)
	  || (height > MT_WINDOW_SIZE_MAX)) {
		width = 800;
		height = 600;
	}

	*pWidth = width;
	*pHeight = height;

EXIT:;
	return result;
}

//******************************************************************************
// Save window size information
//******************************************************************************
int MTWindowSizeCfgDlg::_Save()
{
	int result = 0;
	int apiresult = 0;
	LRESULT lresult = 0;
	int width = 0;
	int height = 0;
	int applyToViewArea = 0;
	TCHAR str[32] = {_T('\0')};

	//Width
	apiresult = GetWindowText(m_hEditWidth, str, 32);
	if (apiresult == 0) {
		//If there is no text or the window handle is invalid
		width = 0;
	}
	else {
		width = _tstoi(str);
	}

	//Height
	apiresult = GetWindowText(m_hEditHeight, str, 32);
	if (apiresult == 0) {
		//If there is no text or the window handle is invalid
		height = 0;
	}
	else {
		height = _tstoi(str);
	}

	//Clipping
	if (width < MT_WINDOW_SIZE_MIN) {
		width = MT_WINDOW_SIZE_MIN;
	}
	if (height < MT_WINDOW_SIZE_MIN) {
		height = MT_WINDOW_SIZE_MIN;
	}

	//Save the setting
	result = m_ConfFile.SetInt(_T("Width"), width);
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("Height"), height);
	if (result != 0) goto EXIT;

	//Save the "apply to view area" checkbox setting
	lresult = SendMessage(m_hCheckApplyToView, BM_GETCHECK, 0, 0);
	if (lresult == BST_CHECKED) {
		applyToViewArea = 1;
	}
	result = m_ConfFile.SetInt(_T("ApplyToViewArea"), applyToViewArea);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Window size list box selection state changed
//******************************************************************************
int MTWindowSizeCfgDlg::_OnSizeListChanged()
{
	int result = 0;
	LRESULT lresult = 0;
	int selectedIndex = 0;
	MTWindowSizeItem item;
	MTWindowSizeList::iterator itr;

	//Get the index of the selected item
	lresult = SendMessage(m_hSizeList, LB_GETCURSEL, 0, 0);
	if (lresult == LB_ERR) {
		//Do nothing if nothing is selected
		goto EXIT;
	}
	else if (lresult < 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hSizeList);
		goto EXIT;
	}

	//Get the selected size
	selectedIndex = (unsigned long)lresult;
	itr = m_SizeList.begin();
	advance(itr, selectedIndex);
	item = *itr;

	//Set the window size number string into the edit boxes
	result = _UpdateSizeEditBox(item.width, item.height);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Update the window size edit boxes
//******************************************************************************
int MTWindowSizeCfgDlg::_UpdateSizeEditBox(
		int width,
		int height
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR str[32] = {_T('\0')};

	//Width
	_stprintf_s(str, 32, _T("%d"), width);
	bresult = SetWindowText(m_hEditWidth, str);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), width);
		goto EXIT;
	}

	//Height
	_stprintf_s(str, 32, _T("%d"), height);
	bresult = SetWindowText(m_hEditHeight, str);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), height);
		goto EXIT;
	}

EXIT:;
	return result;
}

