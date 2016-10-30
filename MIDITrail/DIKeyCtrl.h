//******************************************************************************
//
// MIDITrail / DIKeyCtrl
//
// DirectInput キー入力制御クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DirectInputを用いてキーボードの状態を取得する。
// 現状はイベントバッファ参照機能を持たない。

#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


//******************************************************************************
// DirectInput キー入力制御クラス
//******************************************************************************
class DIKeyCtrl
{
public:

	//コンストラクタ／デストラクタ
	DIKeyCtrl(void);
	virtual ~DIKeyCtrl(void);

	//初期化／終了
	int Initialize(HWND hWnd);
	void Terminate();

	//アクセス権取得／解放
	int Acquire();
	int Unacquire();

	//現時点の状態を取得
	//  GetKeyStatusを一回呼び出してから
	//  状態を取得したいキーの数だけIsKeyDownを呼び出す
	//  BUG: ウィンドウが非アクティブ状態の場合にデバイスへアクセスできず
	//       GetKeyStatus()がエラーになる
	int GetKeyStatus();
	bool IsKeyDown(unsigned char key);

private:

	LPDIRECTINPUT8 m_pDI;
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	unsigned char m_KeyStatus[256];

};


