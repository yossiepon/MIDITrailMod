//******************************************************************************
//
// MIDITrail / MTGamePadCtrl
//
// Gamepad input controller.
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGamePadCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// Constructor
//******************************************************************************
MTGamePadCtrl::MTGamePadCtrl(void)
{
	m_isConnected = false;
	ZeroMemory(&m_PrevGamePadState, sizeof(XINPUT_GAMEPAD));
	ZeroMemory(&m_CurGamePadState, sizeof(XINPUT_GAMEPAD));
	m_ThumbLX = 0.0f;
	m_ThumbLY = 0.0f;
	m_ThumbRX = 0.0f;
	m_ThumbRY = 0.0f;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTGamePadCtrl::~MTGamePadCtrl(void)
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTGamePadCtrl::Initialize(int userIndex)
{
	int result = 0;
	DWORD dwResult = 0;
	XINPUT_STATE xInputState;
	
	if (userIndex >= XUSER_MAX_COUNT) {
		result = YN_SET_ERR("Program error.", userIndex, 0);
		goto EXIT;
	}
	
	//User index
	m_UserIndex = userIndex;
	
	//Get gamepad state
	m_isConnected = false;
	dwResult = XInputGetState(m_UserIndex, &xInputState);
	if (dwResult == ERROR_DEVICE_NOT_CONNECTED) {
		//Gamepad not connected
		//Do nothing
	}
	else if (dwResult == ERROR_SUCCESS) {
		//Gamepad state retrieved successfully
		m_isConnected = true;
		m_CurGamePadState = xInputState.Gamepad;
	}
	else if (dwResult != ERROR_SUCCESS) {
		result = YN_SET_ERR("XInput API error.", m_UserIndex, dwResult);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Update gamepad state
//******************************************************************************
int MTGamePadCtrl::UpdateState()
{
	int result = 0;
	DWORD dwResult = 0;
	XINPUT_STATE xInputState;
	
	m_PrevGamePadState = m_CurGamePadState;
	ZeroMemory(&m_CurGamePadState, sizeof(XINPUT_GAMEPAD));
	
	//Get gamepad state
	m_isConnected = false;
	dwResult = XInputGetState(m_UserIndex, &xInputState);
	if (dwResult == ERROR_DEVICE_NOT_CONNECTED) {
		//Gamepad not connected
		//Do nothing
	}
	else if (dwResult == ERROR_SUCCESS) {
		//Gamepad state retrieved successfully
		m_isConnected = true;
		m_CurGamePadState = xInputState.Gamepad;
	}
	else if (dwResult != ERROR_SUCCESS) {
		result = YN_SET_ERR("XInput API error.", m_UserIndex, dwResult);
		goto EXIT;
	}
	
	//Normalize stick state
	if (m_isConnected) {
		_NormalizeLStickState();
		_NormalizeRStickState();
	}
	else {
		m_ThumbLX = 0.0f;
		m_ThumbLY = 0.0f;
		m_ThumbRX = 0.0f;
		m_ThumbRY = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Normalize left stick state
//******************************************************************************
void MTGamePadCtrl::_NormalizeLStickState()
{
	float rawX = 0.0f;
	float rawY = 0.0f;
	float normalizedX = 0.0f;
	float normalizedY = 0.0f;
	float magnitude = 0.0f;

	rawX = m_CurGamePadState.sThumbLX;
	rawY = m_CurGamePadState.sThumbLY;
	
	magnitude = sqrt((rawX * rawX) + (rawY * rawY));
	normalizedX = rawX / magnitude;
	normalizedY = rawY / magnitude;

	if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
		if (magnitude > 0x7FFF) {
			magnitude = 0x7FFF;
		}
		magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	}
	else {
		magnitude = 0.0f;
		normalizedX = 0.0f;
		normalizedY = 0.0f;
	}

	m_ThumbLX = normalizedX;
	m_ThumbLY = normalizedY;
	
	return;
}

//******************************************************************************
// Normalize right stick state
//******************************************************************************
void MTGamePadCtrl::_NormalizeRStickState()
{
	float rawX = 0.0f;
	float rawY = 0.0f;
	float normalizedX = 0.0f;
	float normalizedY = 0.0f;
	float magnitude = 0.0f;

	rawX = m_CurGamePadState.sThumbRX;
	rawY = m_CurGamePadState.sThumbRY;

	magnitude = sqrt((rawX * rawX) + (rawY * rawY));
	normalizedX = rawX / magnitude;
	normalizedY = rawY / magnitude;

	if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
		if (magnitude > 0x7FFF) {
			magnitude = 0x7FFF;
		}
		magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}
	else {
		magnitude = 0.0f;
		normalizedX = 0.0f;
		normalizedY = 0.0f;
	}

	m_ThumbRX = normalizedX;
	m_ThumbRY = normalizedY;

	return;
}

//******************************************************************************
// Get button state: Up
//******************************************************************************
bool MTGamePadCtrl::GetState_DPadUp()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_DPAD_UP)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Down
//******************************************************************************
bool MTGamePadCtrl::GetState_DPadDown()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Left
//******************************************************************************
bool MTGamePadCtrl::GetState_DPadLeft()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Right
//******************************************************************************
bool MTGamePadCtrl::GetState_DPadRight()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: A
//******************************************************************************
bool MTGamePadCtrl::GetState_A()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_A)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: B
//******************************************************************************
bool MTGamePadCtrl::GetState_B()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_B)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: X
//******************************************************************************
bool MTGamePadCtrl::GetState_X()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_X)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Y
//******************************************************************************
bool MTGamePadCtrl::GetState_Y()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_Y)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Left shoulder
//******************************************************************************
bool MTGamePadCtrl::GetState_LShoulder()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Right shoulder
//******************************************************************************
bool MTGamePadCtrl::GetState_RShoulder()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Left trigger
//******************************************************************************
bool MTGamePadCtrl::GetState_LTrigger()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.bLeftTrigger > 250)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Right trigger
//******************************************************************************
bool MTGamePadCtrl::GetState_RTrigger()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.bRightTrigger > 250)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Start
//******************************************************************************
bool MTGamePadCtrl::GetState_Start()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_START)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get button state: Back
//******************************************************************************
bool MTGamePadCtrl::GetState_Back()
{
	bool state = false;
	
	if ((m_isConnected) && (m_CurGamePadState.wButtons & XINPUT_GAMEPAD_BACK)) {
		state = true;
	}
	
	return state;
}

//******************************************************************************
// Get stick state: Left stick X
//******************************************************************************
float MTGamePadCtrl::GetState_ThumbLX()
{
	float state = 0.0f;
	
	if (m_isConnected) {
		state = m_ThumbLX;
	}
	
	return state;
}

//******************************************************************************
// Get stick state: Left stick Y
//******************************************************************************
float MTGamePadCtrl::GetState_ThumbLY()
{
	float state = 0.0f;
	
	if (m_isConnected) {
		state = m_ThumbLY;
	}
	
	return state;
}

//******************************************************************************
// Get stick state: Right stick X
//******************************************************************************
float MTGamePadCtrl::GetState_ThumbRX()
{
	float state = 0.0f;
	
	if (m_isConnected) {
		state = m_ThumbRX;
	}
	
	return state;
}

//******************************************************************************
// Get stick state: Right stick Y
//******************************************************************************
float MTGamePadCtrl::GetState_ThumbRY()
{
	float state = 0.0f;
	
	if (m_isConnected) {
		state = m_ThumbRY;
	}
	
	return state;
}

//******************************************************************************
// Check button press: A
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_A()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_A) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_A) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: B
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_B()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_B) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_B) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: X
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_X()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_X) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_X) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: Y
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_Y()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_Y) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_Y) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: Left shoulder
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_LShoulder()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: Right shoulder
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_RShoulder()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: Left trigger
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_LTrigger()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if ((m_PrevGamePadState.bLeftTrigger < MT_GAME_PAD_TRRIGER_ON_THRESHOLD)
		 && (m_CurGamePadState.bLeftTrigger >= MT_GAME_PAD_TRRIGER_ON_THRESHOLD)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: Right trigger
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_RTrigger()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if ((m_PrevGamePadState.bRightTrigger < MT_GAME_PAD_TRRIGER_ON_THRESHOLD)
		 && (m_CurGamePadState.bRightTrigger >= MT_GAME_PAD_TRRIGER_ON_THRESHOLD)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: Start
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_Start()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_START) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_START) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

//******************************************************************************
// Check button press: Back
//******************************************************************************
bool MTGamePadCtrl::DidPressNow_Back()
{
	bool isNow = false;
	
	if (m_isConnected) {
		if (((m_PrevGamePadState.wButtons & XINPUT_GAMEPAD_BACK) == 0)
		 && ((m_CurGamePadState.wButtons & XINPUT_GAMEPAD_BACK) != 0)) {
			isNow = true;
		}
	}
	
	return isNow;
}

