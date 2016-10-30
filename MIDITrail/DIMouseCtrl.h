//******************************************************************************
//
// MIDITrail / DIMouseCtrl
//
// DirectInput マウス制御クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DirectInputを用いてマウスの状態を取得する。
// 状態参照とイベントバッファ参照の機能を持つ。

// BUG:
// バッファサイズを指定するインターフェースがない。

#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


//******************************************************************************
// DirectInput マウス制御クラス
//******************************************************************************
class DIMouseCtrl
{
public:

	//マウスボタン種別
	enum MouseButton {
		LeftButton,
		RightButton,
		CenterButton
	};

	//マウス軸種別
	enum MouseAxis {
		AxisX,
		AxisY,
		AxisWheel
	};

	//マウスイベント種別
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

	//コンストラクタ／デストラクタ
	DIMouseCtrl(void);
	virtual ~DIMouseCtrl(void);

	//初期化／終了
	int Initialize(HWND hWnd);
	void Terminate();

	//アクセス権取得／解放
	int Acquire();
	int Unacquire();

	//現時点の状態を取得
	//  GetMouseStatusを一回呼び出してから
	//  状態を取得したいボタンと軸の数だけIsBtnDown,GetDeltaを呼び出す
	//  BUG: ウィンドウが非アクティブ状態の場合にデバイスへアクセスできず
	//       GetMouseStatus()がエラーになる
	int GetMouseStatus();
	bool IsBtnDown(MouseButton);
	int GetDelta(MouseAxis);

	//バッファデータを取得
	//  pIsExistがfalseになるまで繰り返し呼び出す
	//  呼び出すたびに取得したバッファが削除される
	int GetBuffer(bool* pIsExist, MouseEvent* pEvent, int* pDeltaAxis = NULL);

private:

	LPDIRECTINPUT8 m_pDI;
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	DIMOUSESTATE2 m_MouseState;

};


