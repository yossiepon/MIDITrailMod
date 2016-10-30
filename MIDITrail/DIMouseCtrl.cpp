//******************************************************************************
//
// MIDITrail / DIMouseCtrl
//
// DirectInput マウス制御クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DIMouseCtrl.h"

using namespace YNBaseLib;

//******************************************************************************
// マクロ定義
//******************************************************************************
#define IS_KEYDOWN(btn)  (btn & 0x80)

//******************************************************************************
// コンストラクタ
//******************************************************************************
DIMouseCtrl::DIMouseCtrl(void)
{
	m_pDI = NULL;
	m_pDIDevice = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DIMouseCtrl::~DIMouseCtrl(void)
{
	Terminate();
}

//******************************************************************************
// 初期化
//******************************************************************************
int DIMouseCtrl::Initialize(
		HWND hWnd
	)
{
	int result = 0;
	HRESULT hresult = DI_OK;
	HINSTANCE hInstance = NULL;

	Terminate();

	//アプリケーションインスタンスハンドルを取得
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)hWnd);
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
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD)hInstance);
		goto EXIT;
	}

	//デバイスオブジェクトの生成
	hresult = m_pDI->CreateDevice(
					GUID_SysMouse,		//入力デバイスのインスタンスGUID
					&m_pDIDevice,		//作成されたインターフェースポインタ
					NULL				//IUnknownインターフェイスポインタ
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//デバイスのデータフォーマットを設定：定義済みグローバル変数を指定
	hresult = m_pDIDevice->SetDataFormat(&c_dfDIMouse2);
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
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD)hWnd);
		goto EXIT;
	}

	//デバイスのプロパティを設定：バッファサイズ
	DIPROPDWORD diprop;
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//DIPH_DEVICEの場合はゼロ
	diprop.diph.dwHow        = DIPH_DEVICE;	//dwObjの解釈方法：デバイス全体
	diprop.dwData            = 16;			//設定するプロパティ：バッファサイズ

	hresult = m_pDIDevice->SetProperty(
					DIPROP_BUFFERSIZE,	//設定対象プロパティのGUID
					&diprop.diph		//設定するDIPROPHEADER構造体
				);
	if (FAILED(hresult) && (hresult != DI_PROPNOEFFECT)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//デバイスのプロパティを設定：軸モード
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//DIPH_DEVICEの場合はゼロ
	diprop.diph.dwHow        = DIPH_DEVICE;	//dwObjの解釈方法：デバイス全体
	diprop.dwData            = DIPROPAXISMODE_REL;	//設定するプロパティ：相対値モード

	hresult = m_pDIDevice->SetProperty(
					DIPROP_AXISMODE,	//設定対象プロパティのGUID
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
// デバイスアクセス権取得
//******************************************************************************
int DIMouseCtrl::Acquire()
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
int DIMouseCtrl::Unacquire()
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
// マウス状態取得
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
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//ウィンドウが非アクティブ状態であるとGetDeviceState()はエラーになる(0x8007000c)
	//どうしよう・・・

EXIT:;
	return result;
}

//******************************************************************************
// マウスボタン状態確認
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
// マウス相対移動量取得
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
// バッファデータ取得
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

	//バッファデータ取得（実行後バッファがひとつ減る）
	hresult = m_pDIDevice->GetDeviceData(
						sizeof(DIDEVICEOBJECTDATA),	//DIOBJECTDATAFORMAT構造体サイズ
						&devObjData,				//バッファデータ配列：1個だけ
						&inOut,						//入力：バッファ要素数／出力：データ取得数
						0							//フラグ
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//バッファがなければ終了
	if (inOut == 0) goto EXIT;

	//バッファデータ解析
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

