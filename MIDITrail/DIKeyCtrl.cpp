//******************************************************************************
//
// MIDITrail / DIKeyCtrl
//
// DirectInput keyboard input controller.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DIKeyCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// Constructor
//******************************************************************************
DIKeyCtrl::DIKeyCtrl(void)
{
	m_pDI = NULL;
	m_pDIDevice = NULL;
	ZeroMemory(m_KeyStatus, sizeof(unsigned char) * 256);
}

//******************************************************************************
// Destructor
//******************************************************************************
DIKeyCtrl::~DIKeyCtrl(void)
{
	Terminate();
}

//******************************************************************************
// Initialize
//******************************************************************************
int DIKeyCtrl::Initialize(
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
					GUID_SysKeyboard,	//Instance GUID of the input device
					&m_pDIDevice,		//Pointer to the created interface
					NULL				//IUnknown interface pointer
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//Set the device data format: specify the predefined global variable
	hresult = m_pDIDevice->SetDataFormat(&c_dfDIKeyboard);
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

	//Set the device property
	DIPROPDWORD diprop;
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//Zero when using DIPH_DEVICE
	diprop.diph.dwHow        = DIPH_DEVICE;	//How dwObj is interpreted: entire device
	diprop.dwData            = 8;			//Property to set: buffer size

	hresult = m_pDIDevice->SetProperty(
					DIPROP_BUFFERSIZE,	//GUID of the property to set
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
void DIKeyCtrl::Terminate()
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
int DIKeyCtrl::Acquire()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) goto EXIT;

	//Acquire access: S_FALSE (already acquired) and DIERR_OTHERAPPHASPRIO
	//(another app has priority, e.g. window inactive) are normal conditions
	hresult = m_pDIDevice->Acquire();
	if (FAILED(hresult) && (hresult != S_FALSE) && (hresult != DIERR_OTHERAPPHASPRIO)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release device access
//******************************************************************************
int DIKeyCtrl::Unacquire()
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
// Get key state
//******************************************************************************
int DIKeyCtrl::GetKeyStatus()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	hresult = m_pDIDevice->GetDeviceState(256, m_KeyStatus);
	if (FAILED(hresult)) {
		if (hresult == DIERR_INPUTLOST || hresult == DIERR_NOTACQUIRED) {
			ZeroMemory(m_KeyStatus, 256);
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
// Check key state
//******************************************************************************
bool DIKeyCtrl::IsKeyDown(
		unsigned char key
	)
{
	bool isDown = false;

	if ((m_KeyStatus[key] & 0x80) != 0) {
		isDown = true;
	}

	return isDown;
}

