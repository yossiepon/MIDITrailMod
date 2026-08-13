//******************************************************************************
//
// MIDITrail / MTHowToViewDlg
//
// How-to-view dialog.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// How-to-view dialog
//******************************************************************************
class MTHowToViewDlg
{
public:

	//Constructor / Destructor
	MTHowToViewDlg(void);
	virtual ~MTHowToViewDlg(void);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

private:

	BITMAPFILEHEADER m_BmpHead;
	BITMAPINFOHEADER m_BmpInfo;
	HANDLE m_hMemBmpPixel;
	BYTE* m_pBmpPixcel;
	unsigned long m_PageNo;
	HWND m_hWnd;

	//Pointer for window procedure control
	static MTHowToViewDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Load the How-To bitmap
	int _LoadHowToBmp();

	//Draw the How-To bitmap
	int _DrawHowToBmp();

	//Clear
	void _Clear();

	//Previous button
	int _OnPreviousButton();

	//Next button
	int _OnNextButton();

	//Display image
	int _DrawHowToImage();

	//Update button state
	void _UpdateButtonStatus();

};


