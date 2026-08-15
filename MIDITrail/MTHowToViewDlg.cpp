//******************************************************************************
//
// MIDITrail / MTHowToViewDlg
//
// How-to-view dialog.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "Resource.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTHowToViewDlg.h"

using namespace YNBaseLib;

//******************************************************************************
// Parameter definitions
//******************************************************************************
//Number of images to display
#define MT_HOWTOVIEW_IMAGE_NUM  (3)

//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MTHowToViewDlg* MTHowToViewDlg::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MTHowToViewDlg::MTHowToViewDlg(void)
{
	m_pThis = this;
	m_hMemBmpPixel = NULL;
	m_pBmpPixcel = NULL;
	m_PageNo = 0;
	m_hWnd = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTHowToViewDlg::~MTHowToViewDlg(void)
{
	_Clear();
}

//******************************************************************************
// Window procedure
//******************************************************************************
INT_PTR CALLBACK MTHowToViewDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Window procedure: implementation
//******************************************************************************
INT_PTR MTHowToViewDlg::_WndProcImpl(
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
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_PREVIOUS) {
				result = _OnPreviousButton();
				if (result != 0) goto EXIT;
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_NEXT) {
				result = _OnNextButton();
				if (result != 0) goto EXIT;
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_CLOSE) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			break;
		case WM_PAINT:
			result = _DrawHowToBmp();
			if (result != 0) goto EXIT;
			bresult = TRUE;
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
int MTHowToViewDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	m_PageNo = 0;

	//Get the application instance handle
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//Show the dialog
	dresult = DialogBox(
					hInstance,							//Instance handle
					MAKEINTRESOURCE(IDD_HOWTOVIEW),		//Dialog box template
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
int MTHowToViewDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_hWnd = hDlg;

	//Load the How-To bitmap
	result = _LoadHowToBmp();
	if (result != 0) goto EXIT;

	//Update button state
	_UpdateButtonStatus();

EXIT:;
	return result;
}

//******************************************************************************
// Load the How-To bitmap
//******************************************************************************
int MTHowToViewDlg::_LoadHowToBmp()
{
	int result = 0;
	HANDLE hFile = NULL;
	BOOL bresult = FALSE;
	DWORD numOfBytesRead = 0;
	DWORD fp = 0;
	HANDLE hMemBmpPixel = NULL;
	BYTE* pBmpPixcel = NULL;
	TCHAR bmpFilePath[_MAX_PATH] = {_T('\0')};
	const TCHAR* pBmpFileName[3] = { MT_IMGFILE_HOWTOVIEW1, MT_IMGFILE_HOWTOVIEW2, MT_IMGFILE_HOWTOVIEW3 };
	DWORD bmpPixelDataSize = 0;

	_Clear();

	//----------------------------------------------------------------
	//Open file
	//----------------------------------------------------------------
	//Get the process executable directory path
	result = YNPathUtil::GetModuleDirPath(bmpFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	if (m_PageNo >= MT_HOWTOVIEW_IMAGE_NUM) {
		result = YN_SET_ERR("Program error.", m_PageNo, 0);
		goto EXIT;
	}

	//Build the BMP file path
	_tcscat_s(bmpFilePath, _MAX_PATH, pBmpFileName[m_PageNo]);

	//Open the BMP file
	hFile = CreateFile(
				bmpFilePath,			//File path
				GENERIC_READ,			//Access type
				0,						//Share mode
				NULL,					//Security attributes
				OPEN_EXISTING,			//Creation disposition
				FILE_ATTRIBUTE_NORMAL,	//File attributes and flags
				NULL					//Template file handle
			);
	if (hFile == INVALID_HANDLE_VALUE) {
		result = YN_SET_ERR("File open error.", GetLastError(), 0);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMP file header
	//----------------------------------------------------------------
	//Read the BMP file header
	bresult = ReadFile(
					hFile,							//File handle
					&m_BmpHead,						//Buffer address
					sizeof(BITMAPFILEHEADER),		//Read size
					&numOfBytesRead,				//Number of bytes read
					NULL							//Overlapped structure buffer
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	//Verify the file type is "BM"
	if (m_BmpHead.bfType != 0x4D42) {
		result = YN_SET_ERR("Invalid data found.", m_BmpHead.bfType, 0);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMP info header
	//----------------------------------------------------------------
	//Read the BMP info header
	bresult = ReadFile(
					hFile,							//File handle
					&m_BmpInfo,						//Buffer address
					sizeof(BITMAPINFOHEADER),		//Read size
					&numOfBytesRead,				//Number of bytes read
					NULL							//Overlapped structure buffer
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	//Only 24-bit images are read
	//Assumes no color table is present
	if ((m_BmpInfo.biBitCount != 24) || (m_BmpInfo.biClrUsed != 0)) {
		result = YN_SET_ERR("Invalid BMP file.", m_BmpInfo.biBitCount, m_BmpInfo.biClrUsed);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMP pixel data
	//----------------------------------------------------------------
	//Set the file pointer to the start of the pixel data
	fp = SetFilePointer(
				hFile,					//File handle
				m_BmpHead.bfOffBits,	//File pointer move distance: low 32 bits
				0,						//File pointer move distance: high 32 bits
				FILE_BEGIN				//Starting point
			);
	if (fp == INVALID_SET_FILE_POINTER) {
		result = YN_SET_ERR("File access error.", GetLastError(), m_BmpHead.bfOffBits);
		goto EXIT;
	}

	//BMP pixel data size
	//Assumes no color table is present
	bmpPixelDataSize = m_BmpHead.bfSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);

	//Allocate memory for reading pixel data
	hMemBmpPixel = GlobalAlloc(GHND, bmpPixelDataSize);
	if (hMemBmpPixel == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	pBmpPixcel = (BYTE*)GlobalLock(hMemBmpPixel);
	if (pBmpPixcel == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//Read the BMP pixel data
	bresult = ReadFile(
					hFile,				//File handle
					pBmpPixcel,			//Buffer address
					bmpPixelDataSize,	//Read size
					&numOfBytesRead,	//Number of bytes read
					NULL				//Overlapped structure buffer
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	m_hMemBmpPixel = hMemBmpPixel;
	m_pBmpPixcel = pBmpPixcel;

EXIT:;
	if (hFile != NULL) {
		CloseHandle(hFile);
	}
	if (result != 0) {
		if (hMemBmpPixel != NULL) {
			GlobalUnlock(hMemBmpPixel);
			GlobalFree(hMemBmpPixel);
		}
	}
	return result;
}

//******************************************************************************
// Draw the How-To bitmap
//******************************************************************************
int MTHowToViewDlg::_DrawHowToBmp()
{
	int result = 0;
	int apiresult = 0;
	HDC hdc = NULL;
	HWND hWndPicture = NULL;
	PAINTSTRUCT ps;

	if (m_pBmpPixcel == NULL) goto EXIT;

	//Get the window handle to draw into
	hWndPicture = GetDlgItem(m_hWnd, IDC_HOWTO_PICTURE);

	//Prepare to draw
	hdc = BeginPaint(hWndPicture, &ps);

	apiresult = SetDIBitsToDevice(
					hdc,					//Device context handle
					0,						//Destination top-left coordinate: x
					0,						//Destination top-left coordinate: y
					m_BmpInfo.biWidth,		//Source size: width
					m_BmpInfo.biHeight,		//Source size: height
					0,						//Source bottom-left coordinate: x
					0,						//Source bottom-left coordinate: y
					0,						//Starting scan line
					m_BmpInfo.biHeight,		//Number of scan lines
					m_pBmpPixcel,			//Address of the start of the bitmap data
					(BITMAPINFO*)&m_BmpInfo,//BMP info header
					DIB_RGB_COLORS			//Color usage specification
				);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Finish drawing
	EndPaint(hWndPicture, &ps);

EXIT:;
	return result;
}

//******************************************************************************
// Clear
//******************************************************************************
void MTHowToViewDlg::_Clear()
{
	ZeroMemory(&m_BmpHead, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&m_BmpInfo, sizeof(BITMAPINFOHEADER));

	if (m_hMemBmpPixel != NULL) {
		GlobalUnlock(m_hMemBmpPixel);
		GlobalFree(m_hMemBmpPixel);
		m_hMemBmpPixel = NULL;
	}
	m_pBmpPixcel = NULL;
}

//******************************************************************************
// Previous button pressed
//******************************************************************************
int MTHowToViewDlg::_OnPreviousButton()
{
	int result = 0;

	//Move to the previous image
	m_PageNo--;

	//Guard just in case
	if (m_PageNo < 0) {
		m_PageNo = 0;
	}

	//Update button state
	_UpdateButtonStatus();

	//Display image
	result = _DrawHowToImage();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Next button pressed
//******************************************************************************
int MTHowToViewDlg::_OnNextButton()
{
	int result = 0;

	//Move to the next image
	m_PageNo++;

	//Guard just in case
	if (m_PageNo >= MT_HOWTOVIEW_IMAGE_NUM) {
		m_PageNo = MT_HOWTOVIEW_IMAGE_NUM - 1;
	}

	//Update button state
	_UpdateButtonStatus();

	//Display image
	result = _DrawHowToImage();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Display image
//******************************************************************************
int MTHowToViewDlg::_DrawHowToImage()
{
	int result = 0;

	//Load the How-To bitmap
	result = _LoadHowToBmp();
	if (result != 0) goto EXIT;

	//Redraw
	InvalidateRect(
			m_hWnd,	//Window handle
			NULL,	//Update region: entire client area
			FALSE	//Erase background: none
		);
	UpdateWindow(m_hWnd);

EXIT:;
	return result;
}

//******************************************************************************
// Update button state
//******************************************************************************
void MTHowToViewDlg::_UpdateButtonStatus()
{
	HWND hPreviousButton = NULL;
	HWND hNextButton = NULL;

	hPreviousButton = GetDlgItem(m_hWnd, IDC_BTN_PREVIOUS);
	hNextButton = GetDlgItem(m_hWnd, IDC_BTN_NEXT);

	EnableWindow(hPreviousButton, TRUE);
	EnableWindow(hNextButton, TRUE);

	//First image is displayed: disable the Previous button
	if (m_PageNo == 0) {
		EnableWindow(hPreviousButton, FALSE);
	}
	//Last image is displayed: disable the Next button
	if (m_PageNo == (MT_HOWTOVIEW_IMAGE_NUM - 1)) {
		EnableWindow(hNextButton, FALSE);
	}
}


