//******************************************************************************
//
// MIDITrail / DIKeyCtrl
//
// DirectInput キー入力制御クラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DIKeyCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
DIKeyCtrl::DIKeyCtrl(void)
{
	m_pDI = NULL;
	m_pDIDevice = NULL;
	ZeroMemory(m_KeyStatus, sizeof(unsigned char) * 256);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DIKeyCtrl::~DIKeyCtrl(void)
{
	Terminate();
}

//******************************************************************************
// 初期化
//******************************************************************************
int DIKeyCtrl::Initialize(
		HWND hWnd
	)
{
	int result = 0;
	HRESULT hresult = DI_OK;
	HINSTANCE hInstance = NULL;

	Terminate();

	//アプリケーションインスタンスハンドルを取得
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hWnd);
		goto EXIT;
	}

	//DirectInputオブジェクトの生成
	hresult = DirectInput8Create(
				hInstance,				//アプリケーションインスタンスハンドル
				DIRECTINPUT_VERSION,	//DirectInputバージョン番号
				IID_IDirectInput8,		//インターフェース識別子
				(void**)&m_pDI,			//作成されたインターフェースポインタ
				NULL					//IUnknownインターフェイスポインタ
			);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD64)hInstance);
		goto EXIT;
	}

	//デバイスオブジェクトの生成
	hresult = m_pDI->CreateDevice(
					GUID_SysKeyboard,	//入力デバイスのインスタンスGUID
					&m_pDIDevice,		//作成されたインターフェースポインタ
					NULL				//IUnknownインターフェイスポインタ
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//デバイスのデータフォーマットを設定：定義済みグローバル変数を指定
	hresult = m_pDIDevice->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//デバイスの協調レベルを設定
	hresult = m_pDIDevice->SetCooperativeLevel(
					hWnd,					//デバイスに関連付けられているウィンドウハンドル
					DISCL_FOREGROUND		//協調レベル：フォアグランドアクセス権
					| DISCL_NONEXCLUSIVE	//協調レベル：非排他的アクセス権
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD64)hWnd);
		goto EXIT;
	}

	//デバイスのプロパティを設定
	DIPROPDWORD diprop;
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//DIPH_DEVICEの場合はゼロ
	diprop.diph.dwHow        = DIPH_DEVICE;	//dwObjの解釈方法：デバイス全体
	diprop.dwData            = 8;			//設定するプロパティ：バッファサイズ

	hresult = m_pDIDevice->SetProperty(
					DIPROP_BUFFERSIZE,	//設定対象プロパティのGUID
					&diprop.diph		//設定するDIPROPHEADER構造体
				);
	if (FAILED(hresult) && (hresult != DI_PROPNOEFFECT)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 終了処理
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
// デバイスアクセス権取得
//******************************************************************************
int DIKeyCtrl::Acquire()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) goto EXIT;

	//アクセス権取得：//デバイス取得済み(S_FALSE)は正常とみなす
	hresult = m_pDIDevice->Acquire();
	if (FAILED(hresult) && (hresult != S_FALSE)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// デバイスアクセス権解放
//******************************************************************************
int DIKeyCtrl::Unacquire()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) goto EXIT;

	//アクセス権解放
	hresult = m_pDIDevice->Unacquire();
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// キー状態取得
//******************************************************************************
int DIKeyCtrl::GetKeyStatus()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	hresult = m_pDIDevice->GetDeviceState(256, m_KeyStatus);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//ウィンドウが非アクティブ状態であるとGetDeviceState()はエラーになる(0x8007000c)
	//どうしよう・・・

EXIT:;
	return result;
}

//******************************************************************************
// キー状態確認
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

