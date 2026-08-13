//******************************************************************************
//
// MIDITrail / MTGamePadCtrl
//
// Gamepad input controller.
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Uses XInput to retrieve the gamepad state.

#pragma once

#include <XInput.h>


//******************************************************************************
// Parameter definitions
//******************************************************************************

//Trigger ON threshold (0-255)
#define MT_GAME_PAD_TRRIGER_ON_THRESHOLD		(250)


//******************************************************************************
// Gamepad controller class
//******************************************************************************
class MTGamePadCtrl
{
public:
	
	//Constructor / Destructor
	MTGamePadCtrl(void);
	virtual ~MTGamePadCtrl();
	
	//Initialize
	int Initialize(int userIndex);
	
	//Update state
	int UpdateState();
	
	//Get button state
	bool GetState_DPadUp();
	bool GetState_DPadDown();
	bool GetState_DPadLeft();
	bool GetState_DPadRight();
	bool GetState_A();
	bool GetState_B();
	bool GetState_X();
	bool GetState_Y();
	bool GetState_LShoulder();
	bool GetState_RShoulder();
	bool GetState_LTrigger();
	bool GetState_RTrigger();
	bool GetState_Start();
	bool GetState_Back();
	
	//Get stick state (0.0 - 1.0)
	float GetState_ThumbLX();
	float GetState_ThumbLY();
	float GetState_ThumbRX();
	float GetState_ThumbRY();
	
	//Check button press
	bool DidPressNow_A();
	bool DidPressNow_B();
	bool DidPressNow_X();
	bool DidPressNow_Y();
	bool DidPressNow_LShoulder();
	bool DidPressNow_RShoulder();
	bool DidPressNow_LTrigger();
	bool DidPressNow_RTrigger();
	bool DidPressNow_Start();
	bool DidPressNow_Back();
	
private:
	
	//User index
	int m_UserIndex;
	
	//Gamepad connection state
	bool m_isConnected;
	
	//Gamepad state
	XINPUT_GAMEPAD m_PrevGamePadState;
	XINPUT_GAMEPAD m_CurGamePadState;

	//Stick state
	float m_ThumbLX;
	float m_ThumbLY;
	float m_ThumbRX;
	float m_ThumbRY;

	void _NormalizeLStickState();
	void _NormalizeRStickState();

};






