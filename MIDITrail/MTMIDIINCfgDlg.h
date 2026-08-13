//******************************************************************************
//
// MIDITrail / MTMIDIINCfgDlg
//
// MIDI IN configuration dialog.
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "SMIDILib.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// MIDI IN configuration dialog class
//******************************************************************************
class MTMIDIINCfgDlg
{
public:

	//Constructor / Destructor
	MTMIDIINCfgDlg(void);
	virtual ~MTMIDIINCfgDlg(void);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

private:

	//Pointer for window procedure control
	static MTMIDIINCfgDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Configuration file
	YNConfFile m_ConfFile;

	//MIDI input device control object
	SMInDevCtrl m_MIDIInDevCtrl;

	//Combo box window handle
	HWND m_hComboDevA;

	//MIDITHRU checkbox window handle
	HWND m_hMIDITHRU;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Initialize config file
	int _InitConfFile();

	//Initialize the device selection combo box
	int _InitComboDev(HWND hComboDev, TCHAR* pPortName);

	//Initialize the MIDITHRU checkbox
	int _InitCheckBtnMIDITHRU();

	//Save processing
	int _Save();
	int _SavePortCfg(HWND hComboDev, TCHAR* pPortName);
	int _SaveMIDITHRU();

};


