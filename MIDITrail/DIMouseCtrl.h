//******************************************************************************
//
// MIDITrail / DIMouseCtrl
//
// DirectInput mouse input controller.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Uses DirectInput to retrieve the mouse state.
// Provides both state polling and event buffer access.

// NOTE:
// No interface to specify the buffer size.

#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


//******************************************************************************
// DirectInput mouse controller class
//******************************************************************************
class DIMouseCtrl
{
public:

	//Mouse button type
	enum MouseButton {
		LeftButton,
		RightButton,
		CenterButton
	};

	//Mouse axis type
	enum MouseAxis {
		AxisX,
		AxisY,
		AxisWheel
	};

	//Mouse event type
	enum MouseEvent {
		LeftButtonDown,
		LeftButtonUp,
		RightButtonDown,
		RightButtonUp,
		CenterButtonDown,
		CenterButtonUp,
		AxisXMove,
		AxisYMove,
		AxisWheelMove
	};

public:

	//Constructor / Destructor
	DIMouseCtrl(void);
	virtual ~DIMouseCtrl(void);

	//Initialize / Terminate
	int Initialize(HWND hWnd);
	void Terminate();

	//Acquire / release access
	int Acquire();
	int Unacquire();

	//Get the current state
	//  Call GetMouseStatus once, then
	//  call IsBtnDown/GetDelta as many times as needed for the buttons and axes whose state you want
	int GetMouseStatus();
	bool IsBtnDown(MouseButton);
	int GetDelta(MouseAxis);

	//Get buffer data
	//  Call repeatedly until pIsExist becomes false
	//  Each call removes the retrieved buffer entry
	int GetBuffer(bool* pIsExist, MouseEvent* pEvent, int* pDeltaAxis = NULL);

private:

	LPDIRECTINPUT8 m_pDI;
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	DIMOUSESTATE2 m_MouseState;

};


