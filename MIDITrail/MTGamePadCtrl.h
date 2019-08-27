//******************************************************************************
//
// MIDITrail / MTGamePadCtrl
//
// ゲームパッド制御クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// XInputを用いてゲームパッドの状態を取得する。

#pragma once

#include <XInput.h>


//******************************************************************************
// パラメータ定義
//******************************************************************************

//トリガーON閾値(0-255)
#define MT_GAME_PAD_TRRIGER_ON_THRESHOLD		(250)


//******************************************************************************
// ゲームパッド制御クラス
//******************************************************************************
class MTGamePadCtrl
{
public:
	
	//コンストラクタ／デストラクタ
	MTGamePadCtrl(void);
	virtual ~MTGamePadCtrl();
	
	//初期化
	int Initialize(int userIndex);
	
	//状態更新
	int UpdateState();
	
	//ボタン状態取得
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
	
	//スティック状態取得(0.0 - 1.0)
	float GetState_ThumbLX();
	float GetState_ThumbLY();
	float GetState_ThumbRX();
	float GetState_ThumbRY();
	
	//ボタン押下確認
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
	
	//ユーザインデックス
	int m_UserIndex;
	
	//ゲームパッド接続状態
	bool m_isConnected;
	
	//ゲームパッド状態
	XINPUT_GAMEPAD m_PrevGamePadState;
	XINPUT_GAMEPAD m_CurGamePadState;

	//スティック状態
	float m_ThumbLX;
	float m_ThumbLY;
	float m_ThumbRX;
	float m_ThumbRY;

	void _NormalizeLStickState();
	void _NormalizeRStickState();

};






