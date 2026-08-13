//******************************************************************************
//
// MIDITrail / DIKeyCtrl
//
// DirectInput keyboard input controller.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Uses DirectInput to retrieve the keyboard state.
// Currently does not support event buffer access.

#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


//******************************************************************************
// DirectInput keyboard input controller class
//******************************************************************************
class DIKeyCtrl
{
public:

	//Constructor / Destructor
	DIKeyCtrl(void);
	virtual ~DIKeyCtrl(void);

	//Initialize / Terminate
	int Initialize(HWND hWnd);
	void Terminate();

	//Acquire / release access
	int Acquire();
	int Unacquire();

	//Get the current state
	//  Call GetKeyStatus once, then
	//  call IsKeyDown as many times as needed for the keys whose state you want
	int GetKeyStatus();
	bool IsKeyDown(unsigned char key);

private:

	LPDIRECTINPUT8 m_pDI;
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	unsigned char m_KeyStatus[256];

};


