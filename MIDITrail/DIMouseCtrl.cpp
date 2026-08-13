//******************************************************************************
//
// MIDITrail / DIMouseCtrl
//
// DirectInput mouse input controller.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DIMouseCtrl.h"

using namespace YNBaseLib;

//******************************************************************************
// Macro definitions
//******************************************************************************
#define IS_KEYDOWN(btn)  (btn & 0x80)

//******************************************************************************
// Constructor
//******************************************************************************
DIMouseCtrl::DIMouseCtrl(void)
{
	m_pDI = NULL;
	m_pDIDevice = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
DIMouseCtrl::~DIMouseCtrl(void)
{
	Terminate();
}

//******************************************************************************
// Initialize
//******************************************************************************
int DIMouseCtrl::Initialize(
		HWND hWnd
	)
{
	int result = 0;
	HRESULT hresult = DI_OK;
	HINSTANCE hInstance = NULL;

	Terminate();

	//Get the application instance handle
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hWnd);
		goto EXIT;
	}

	//Create the DirectInput object
	hresult = DirectInput8Create(
				hInstance,				//Application instance handle
				DIRECTINPUT_VERSION,	//DirectInput version number
				IID_IDirectInput8,		//Interface identifier
				(void**)&m_pDI,			//Pointer to the created interface
				NULL					//IUnknown interface pointer
			);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD64)hInstance);
		goto EXIT;
	}

	//Create the device object
	hresult = m_pDI->CreateDevice(
					GUID_SysMouse,		//Instance GUID of the input device
					&m_pDIDevice,		//Pointer to the created interface
					NULL				//IUnknown interface pointer
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//Set the device data format: specify the predefined global variable
	hresult = m_pDIDevice->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//Set the device cooperative level
	hresult = m_pDIDevice->SetCooperativeLevel(
					hWnd,					//Window handle associated with the device
					DISCL_FOREGROUND		//Cooperative level: foreground access
					| DISCL_NONEXCLUSIVE	//Cooperative level: non-exclusive access
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD64)hWnd);
		goto EXIT;
	}

	//Set the device property: buffer size
	DIPROPDWORD diprop;
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//Zero when using DIPH_DEVICE
	diprop.diph.dwHow        = DIPH_DEVICE;	//How dwObj is interpreted: entire device
	diprop.dwData            = 16;			//Property to set: buffer size

	hresult = m_pDIDevice->SetProperty(
					DIPROP_BUFFERSIZE,	//GUID of the property to set
					&diprop.diph		//DIPROPHEADER structure to set
				);
	if (FAILED(hresult) && (hresult != DI_PROPNOEFFECT)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//Set the device property: axis mode
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//Zero when using DIPH_DEVICE
	diprop.diph.dwHow        = DIPH_DEVICE;	//How dwObj is interpreted: entire device
	diprop.dwData            = DIPROPAXISMODE_REL;	//Property to set: relative mode

	hresult = m_pDIDevice->SetProperty(
					DIPROP_AXISMODE,	//GUID of the property to set
					&diprop.diph		//DIPROPHEADER structure to set
				);
	if (FAILED(hresult) && (hresult != DI_PROPNOEFFECT)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Terminate
//******************************************************************************
void DIMouseCtrl::Terminate()
{
	if (m_pDIDevice != NULL) {
		m_pDIDevice->Unacquire();
		m_pDIDevice->Release();
		m_pDIDevice = NULL;
	}

	if (m_pDI != NULL) {
		m_pDI->Release();
		m_pDI = NULL;
	}

	return;
}

//******************************************************************************
// Acquire device access
//******************************************************************************
int DIMouseCtrl::Acquire()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) goto EXIT;

	//Acquire access: device already acquired (S_FALSE) is treated as normal
	hresult = m_pDIDevice->Acquire();
	if (FAILED(hresult) && (hresult != S_FALSE)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release device access
//******************************************************************************
int DIMouseCtrl::Unacquire()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) goto EXIT;

	//Release access
	hresult = m_pDIDevice->Unacquire();
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get mouse state
//******************************************************************************
int DIMouseCtrl::GetMouseStatus()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	hresult = m_pDIDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &m_MouseState);
	if (FAILED(hresult)) {
		if (hresult == DIERR_INPUTLOST || hresult == DIERR_NOTACQUIRED) {
			ZeroMemory(&m_MouseState, sizeof(DIMOUSESTATE2));
		}
		else {
			result = YN_SET_ERR("DirectInput API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Check mouse button state
//******************************************************************************
bool DIMouseCtrl::IsBtnDown(
		MouseButton	target
	)
{
	bool isDown = false;
	BYTE btn = 0;

	if (m_pDIDevice == NULL) goto EXIT;

	if (target == LeftButton) {
		btn = m_MouseState.rgbButtons[0];
	}
	if (target == RightButton) {
		btn = m_MouseState.rgbButtons[1];
	}

	if (IS_KEYDOWN(btn)) {
		isDown = true;
	}

EXIT:;
	return isDown;
}

//******************************************************************************
// Get mouse relative movement
//******************************************************************************
int DIMouseCtrl::GetDelta(
		MouseAxis	target
	)
{
	int rel = 0;

	if (m_pDIDevice == NULL) goto EXIT;

	if (target == AxisX) {
		rel = m_MouseState.lX;
	}
	if (target == AxisY) {
		rel = m_MouseState.lY;
	}
	if (target == AxisWheel) {
		rel = m_MouseState.lZ;
	}

EXIT:;
	return rel;
}

//******************************************************************************
// Get buffer data
//******************************************************************************
int DIMouseCtrl::GetBuffer(
		bool* pIsExist,
		MouseEvent* pEvent,
		int* pDeltaAxis
	)
{
	int result = 0;
	HRESULT hresult = DI_OK;
	DIDEVICEOBJECTDATA devObjData;
	DWORD inOut = 1;

	if ((m_pDIDevice == NULL) || (pIsExist == NULL) || (pEvent == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	*pIsExist = false;

	//Get buffer data (the buffer count decreases by one after this call)
	hresult = m_pDIDevice->GetDeviceData(
						sizeof(DIDEVICEOBJECTDATA),	//Size of the DIOBJECTDATAFORMAT structure
						&devObjData,				//Buffer data array: one element only
						&inOut,						//In: buffer element count / Out: number of items retrieved
						0							//Flags
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//Exit if there is no buffer data
	if (inOut == 0) goto EXIT;

	//Parse buffer data
	switch (devObjData.dwOfs) {
		case DIMOFS_BUTTON0:
			if (IS_KEYDOWN(devObjData.dwData)) {
				*pEvent = LeftButtonDown;
			}
			else {
				*pEvent = LeftButtonUp;
			}
			break;
		case DIMOFS_BUTTON1:
			if (IS_KEYDOWN(devObjData.dwData)) {
				*pEvent = RightButtonDown;
			}
			else {
				*pEvent = RightButtonUp;
			}
			break;
		case DIMOFS_BUTTON2:
			if (IS_KEYDOWN(devObjData.dwData)) {
				*pEvent = CenterButtonDown;
			}
			else {
				*pEvent = CenterButtonUp;
			}
			break;
		case DIMOFS_X:
			*pEvent = AxisXMove;
			if (pDeltaAxis!= NULL) {
				*pDeltaAxis = (int)devObjData.dwData;
			}
			break;
		case DIMOFS_Y:
			*pEvent = AxisYMove;
			if (pDeltaAxis!= NULL) {
				*pDeltaAxis = (int)devObjData.dwData;
			}
			break;
		case DIMOFS_Z:
			*pEvent = AxisWheelMove;
			if (pDeltaAxis!= NULL) {
				*pDeltaAxis = (int)devObjData.dwData;
			}
			break;
		default:
			break;
	}

	*pIsExist = true;

EXIT:;
	return result;
}

