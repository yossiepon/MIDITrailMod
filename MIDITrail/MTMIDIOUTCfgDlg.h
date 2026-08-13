//******************************************************************************
//
// MIDITrail / MTMIDIOUTCfgDlg
//
// MIDI OUT configuration dialog.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "SMIDILib.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// MIDI OUT configuration dialog class
//******************************************************************************
class MTMIDIOUTCfgDlg
{
public:

	//Constructor / Destructor
	MTMIDIOUTCfgDlg(void);
	virtual ~MTMIDIOUTCfgDlg(void);

	//Show: does not return control until the dialog is closed
	int Show(HWND hParentWnd);

private:

	//Pointer for window procedure control
	static MTMIDIOUTCfgDlg* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Configuration file
	YNConfFile m_ConfFile;

	//MIDI output device control object
	SMOutDevCtrl m_MIDIOutDevCtrl;

	//Combo box window handle
	HWND m_hComboDevA;
	HWND m_hComboDevB;
	HWND m_hComboDevC;
	HWND m_hComboDevD;
	HWND m_hComboDevE;
	HWND m_hComboDevF;

	//Window procedure
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Pre-display dialog initialization
	int _OnInitDlg(HWND hDlg);

	//Initialize config file
	int _InitConfFile();

	//Initialize the device selection combo box
	int _InitComboDev(HWND hComboDev, TCHAR* pPortName);

	//Save processing
	int _Save();
	int _SavePortCfg(HWND hComboDev, TCHAR* pPortName);

};


