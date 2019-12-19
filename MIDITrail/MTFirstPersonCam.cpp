//******************************************************************************
//
// MIDITrail / MTFirstPersonCam
//
// 一人称カメラクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "mmsystem.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTFirstPersonCam.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTFirstPersonCam::MTFirstPersonCam(void)
{
	m_CamVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_CamDirPhi = 0.0f;
	m_CamDirTheta = 0.0f;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_hWnd = NULL;

	m_VelocityFB = 15.0f; // m/sec.
	m_VelocityLR = 15.0f; // m/sec.
	m_VelocityUD = 10.0f; // m/sec.
	m_VelocityPT =  6.0f; // degrees/sec.
	m_AcceleRate =  2.0f; // 加速倍率
	m_PrevTime = 0;
	m_DeltaTime = 0;

	m_RollAngle = 0.0f;
	m_VelocityAutoRoll = 6.0f;
	m_VelocityManualRoll = 1.0f;

	m_PrevTickTime = 0;
	m_CurTickTime = 0;
	m_ProgressDirection = DirX;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTFirstPersonCam::~MTFirstPersonCam(void)
{
	m_DIKeyCtrl.Terminate();
	m_DIMouseCtrl.Terminate();
	_ClipCursor(false);
}

//******************************************************************************
// 初期化処理
//******************************************************************************
int MTFirstPersonCam::Initialize(
		HWND hWnd,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	m_hWnd = hWnd;

	//パラメータ設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//キーボードデバイス制御初期化
	result = m_DIKeyCtrl.Initialize(hWnd);
	if (result != 0) goto EXIT;

	//マウスデバイス制御初期化
	result = m_DIMouseCtrl.Initialize(hWnd);
	if (result != 0) goto EXIT;

	//デバイスアクセス権取得
	m_DIKeyCtrl.Acquire();
	m_DIMouseCtrl.Acquire();

	//ゲームパッド初期化：ユーザインデックス0固定
	result = m_GamePadCtrl.Initialize(0);
	if (result != 0) goto EXIT;
	
	//カメラ初期化
	result = m_Camera.Initialize();
	if (result != 0) goto EXIT;

	//基本パラメータ設定
	m_Camera.SetBaseParam(
			45.0f,		//画角
			1.0f,		//Nearプレーン：0だとZ軸順制御がおかしくなる
			1000.0f		//Farプレーン
		);

	//カメラ位置設定
	m_Camera.SetPosition(
			D3DXVECTOR3(0.0f, 0.0f, 0.0f),	//カメラ位置
			D3DXVECTOR3(0.0f, 0.0f, 1.0f), 	//注目点
			D3DXVECTOR3(0.0f, 1.0f, 0.0f)	//カメラ上方向
		);

EXIT:;
	return result;
}

//******************************************************************************
// カメラ位置設定
//******************************************************************************
void MTFirstPersonCam::SetPosition(
		D3DXVECTOR3 camVector
	)
{
	m_CamVector = camVector;
}

//******************************************************************************
// カメラ方向設定
//******************************************************************************
void MTFirstPersonCam::SetDirection(
		float phi,
		float theta
	)
{
	m_CamDirPhi = phi;
	m_CamDirTheta = theta;
}
//******************************************************************************
// カメラ位置取得
//******************************************************************************
void MTFirstPersonCam::GetPosition(
		D3DXVECTOR3* pCamVector
	)
{
	*pCamVector = m_CamVector;
}

//******************************************************************************
// カメラ方向取得
//******************************************************************************
void MTFirstPersonCam::GetDirection(
		float* pPhi,
		float* pTheta
	)
{
	*pPhi = m_CamDirPhi;
	*pTheta = m_CamDirTheta;
}

//******************************************************************************
// マウス視線移動モード登録
//******************************************************************************
void MTFirstPersonCam::SetMouseCamMode(
		bool isEnable
	)
{
	m_IsMouseCamMode = isEnable;

	if (m_IsMouseCamMode) {
		ShowCursor(FALSE);
		_ClipCursor(true);
	}
	else {
		ShowCursor(TRUE);
		_ClipCursor(false);
	}
}

//******************************************************************************
// 自動回転モード登録
//******************************************************************************
void MTFirstPersonCam::SetAutoRollMode(
		bool isEnable
	)
{
	m_IsAutoRollMode = isEnable;
}

//******************************************************************************
// 自動回転方向切り替え
//******************************************************************************
void MTFirstPersonCam::SwitchAutoRllDirecton()
{
	//回転方向を逆にする
	m_VelocityAutoRoll *= -1.0f;
}

//******************************************************************************
// 変換処理
//******************************************************************************
int MTFirstPersonCam::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float dt = 0.0f;
	int dX = 0;
	int dY = 0;
	int dW = 0;
	
	//デルタタイム
	dt = (float)m_DeltaTime / 1000.0f;
	
	//TODO: ここじゃないどこかへ移す
	m_DIKeyCtrl.Acquire();
	m_DIMouseCtrl.Acquire();

	//ウィンドウが非アクティブ状態のとき状態取得がエラーになる
	//とりあえず無視するけど・・・

	//現在のキーボード状態を取得
	result = m_DIKeyCtrl.GetKeyStatus();
	//if (result != 0) goto EXIT;
	result = 0;

	//マウス状態取得
	result = m_DIMouseCtrl.GetMouseStatus();
	//if (result != 0) goto EXIT;
	result = 0;

	//ゲームパッド状態更新
	result = m_GamePadCtrl.UpdateState();
	if (result != 0) goto EXIT;

	//_RPTN(_CRT_WARN, "GamePad: %f %f\n", m_GamePadCtrl.GetState_ThumbRX(), m_GamePadCtrl.GetState_ThumbRY());
	
	//マウス／ホイール移動量
	dX = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisX);
	dY = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisY);
	dW = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisWheel);

	//マウス視線移動モードOFFなら移動量を無視する
	if (!m_IsMouseCamMode) {
		dX = 0;
		dY = 0;
	}

	//ゲームパッド操作：右スティック
	//スティック値は-1.0から1.0の範囲
	dX += (int)(m_VelocityPT * dt * m_GamePadCtrl.GetState_ThumbRX() * (100.0f));
	dY += (int)(m_VelocityPT * dt * m_GamePadCtrl.GetState_ThumbRY() * (-100.0f));
	
	//CTRL+移動キーで視線方向を変化させる
	if (m_DIKeyCtrl.IsKeyDown(DIK_LCONTROL) || m_DIKeyCtrl.IsKeyDown(DIK_RCONTROL)) {
		if (m_DIKeyCtrl.IsKeyDown(DIK_W) || m_DIKeyCtrl.IsKeyDown(DIK_UP)) {
			dY -= (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_S) || m_DIKeyCtrl.IsKeyDown(DIK_DOWN)) {
			dY += (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_A) || m_DIKeyCtrl.IsKeyDown(DIK_LEFT)) {
			dX -= (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_D) || m_DIKeyCtrl.IsKeyDown(DIK_RIGHT)) {
			dX += (int)m_VelocityPT;
		}
	}

	//デルタタイム算出
	_CalcDeltaTime();

	//視線方向の更新
	result = _TransformEyeDirection(dX, dY);
	if (result != 0) goto EXIT;

	//カメラ位置の更新
	result = _TransformCamPosition();
	if (result != 0) goto EXIT;

	//カメラ位置設定
	result = _SetCamPosition();
	if (result != 0) goto EXIT;

	//カメラ更新
	result = m_Camera.Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//回転対応
	result = _TransformRolling(dW);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 視線方向更新
//******************************************************************************
int MTFirstPersonCam::_TransformEyeDirection(
		int dX,
		int dY
	)
{
	int result = 0;
	float dt = 0.0f;
	float dPhi = 0.0f;
	float dTheta = 0.0f;

	//デルタタイム
	dt = (float)m_DeltaTime / 1000.0f;

	//マウス移動量から方位角と天頂角の増加量を算出
	dPhi   = (float)-dX * m_VelocityPT * dt;
	dTheta = (float) dY * m_VelocityPT * dt;

	//極端な角度の変化を抑止する
	//  画面描画が引っかかった場合にマウス移動量が蓄積され
	//  突然あらぬ方向を向いてしまうことを避けたい
	if (abs(dPhi) > 45.0f) {
		dPhi = 0.0f;
	}
	if (abs(dTheta) > 45.0f) {
		dTheta = 0.0f;
	}

	//マウス移動量を方位角と天頂角に反映する
	m_CamDirPhi += dPhi;
	m_CamDirTheta += dTheta;

	//クリッピング処理
	if (m_CamDirPhi >= 360.0f) {
		m_CamDirPhi -= 360.0f;
	}
	else if (m_CamDirPhi <= -360.0f) {
		m_CamDirPhi += 360.0f;
	}
	if (m_CamDirTheta <= 1.0f) {
		m_CamDirTheta = 1.0f;
	}
	else if (m_CamDirTheta >= 179.0f) {
		m_CamDirTheta = 179.0f;
	}
	//↑天頂角が0度または180度になると描画がおかしくなる・・・

//EXIT:;
	return result;
}

//******************************************************************************
// カメラ位置更新
//******************************************************************************
int MTFirstPersonCam::_TransformCamPosition()
{
	int result = 0;
	float dFB = 0.0f;
	float dLR = 0.0f;
	float phi = 0.0f;
	float phiRad = 0.0f;
	float distance = 0.0f;
	float dt = 0.0f;
	float rate = 0.0f;
	float progress = 0.0f;
	D3DXVECTOR3 moveVector;

	//デルタタイム
	dt = (float)m_DeltaTime / 1000.0f;

	//移動方向の方位角
	phi = m_CamDirPhi;

	if (m_DIKeyCtrl.IsKeyDown(DIK_LCONTROL) || m_DIKeyCtrl.IsKeyDown(DIK_RCONTROL)) {
		//左CTRLまたは右CTRLキーが押されている場合はキー入力を無視する
	}
	else {
		//移動速度の加速倍率
		rate = 1.0f;
		if (m_DIKeyCtrl.IsKeyDown(DIK_LSHIFT) || m_DIKeyCtrl.IsKeyDown(DIK_RSHIFT)) {
			rate = m_AcceleRate;
		}
		
		//前移動
		if (m_DIKeyCtrl.IsKeyDown(DIK_W) || m_DIKeyCtrl.IsKeyDown(DIK_UP)) {
			distance = m_VelocityFB * dt * rate;
			phi += 0.0f;
		}
		//後ろ移動：視線は前を向いたまま
		if (m_DIKeyCtrl.IsKeyDown(DIK_S) || m_DIKeyCtrl.IsKeyDown(DIK_DOWN)) {
			distance = m_VelocityFB * dt * rate;
			phi += 180.0f;
		}
		//左移動：視線は前を向いたまま
		if (m_DIKeyCtrl.IsKeyDown(DIK_A) || m_DIKeyCtrl.IsKeyDown(DIK_LEFT)) {
			distance = m_VelocityLR * dt * rate;
			phi += 90.0f;
		}
		//右移動：視線は前を向いたまま
		if (m_DIKeyCtrl.IsKeyDown(DIK_D) || m_DIKeyCtrl.IsKeyDown(DIK_RIGHT)) {
			distance = m_VelocityLR * dt * rate;
			phi += -90.0f;
		}
		//上昇：視線変更なし
		if (m_DIKeyCtrl.IsKeyDown(DIK_Q) || m_DIKeyCtrl.IsKeyDown(DIK_PRIOR)) {
			m_CamVector.y += +(m_VelocityUD * dt * rate);
		}
		//下降：視線変更なし
		if (m_DIKeyCtrl.IsKeyDown(DIK_E) ||  m_DIKeyCtrl.IsKeyDown(DIK_NEXT)) {
			m_CamVector.y += -(m_VelocityUD * dt * rate);
		}
		//-X軸方向（曲再生逆方向）に移動：視線変更なし
		if (m_DIKeyCtrl.IsKeyDown(DIK_Z) || m_DIKeyCtrl.IsKeyDown(DIK_COMMA)) {
			m_CamVector.x +=  -(m_VelocityFB * dt * rate);
		}
		//+X軸方向（曲再生方向）に移動：視線変更なし
		if (m_DIKeyCtrl.IsKeyDown(DIK_C) || m_DIKeyCtrl.IsKeyDown(DIK_PERIOD)) {
			m_CamVector.x +=  +(m_VelocityFB * dt * rate);
		}
	}
	
	//ゲームパッド操作：十字キー＞前後左右移動
	if (distance == 0.0f) {
		if (m_GamePadCtrl.GetState_DPadUp()) {
			dFB = m_VelocityFB * dt * (1.0f);
		}
		if (m_GamePadCtrl.GetState_DPadDown()) {
			dFB = m_VelocityFB * dt * (-1.0f);
		}
		if (m_GamePadCtrl.GetState_DPadRight()) {
			dLR = m_VelocityLR * dt * (-1.0f);
		}
		if (m_GamePadCtrl.GetState_DPadLeft()) {
			dLR = m_VelocityLR * dt * (1.0f);
		}
		distance = sqrt((dFB * dFB) + (dLR * dLR));
		phi += D3DXToDegree(atan2(dLR, dFB));
	}
	//ゲームパッド操作：左スティック＞前後左右移動
	if (distance == 0.0f) {
		//スティック値は-1.0から1.0の範囲
		dFB += m_VelocityFB * dt * m_GamePadCtrl.GetState_ThumbLX() * -1.0f;
		dLR += m_VelocityLR * dt * m_GamePadCtrl.GetState_ThumbLY();
		distance = sqrt((dFB * dFB) + (dLR * dLR));
		phi += D3DXToDegree(atan2(dFB, dLR));
	}
	//ゲームパッド操作：X,Yボタン＞下降,上昇移動
	if (m_GamePadCtrl.GetState_X()) {
		m_CamVector.y += -(m_VelocityUD * dt);
	}
	if (m_GamePadCtrl.GetState_Y()) {
		m_CamVector.y += +(m_VelocityUD * dt);
	}
	
	//クリッピング
	if (phi >= 360.0f) {
		phi -= 360.0f;
	}
	else if (phi <= -360.0f) {
		phi += 360.0f;
	}

	//移動ベクトル作成（極座標から直行座標へ変換）
	phiRad = D3DXToRadian(phi);
	moveVector.x = distance * cos(phiRad);  // r * sin(90) * cos(phi)
	moveVector.y = 0.0f;                    // r * cos(90)
	moveVector.z = distance * sin(phiRad);  // r * sin(90) * cos(phi)

	//カメラ位置を移動
	m_CamVector.x += moveVector.x;
	m_CamVector.y += moveVector.y;
	m_CamVector.z += moveVector.z;

	//演奏追跡
	progress = m_NoteDesign.GetPlayPosX(m_CurTickTime) - m_NoteDesign.GetPlayPosX(m_PrevTickTime);
	switch (m_ProgressDirection) {
		case DirX:
			m_CamVector.x += progress;
			break;
		case DirY:
			m_CamVector.y += progress;
			break;
		case DirZ:
			m_CamVector.z += progress;
			break;
	}

	//カメラ位置クリッピング
	_ClipCamVector(&m_CamVector);

	m_PrevTickTime = m_CurTickTime;

//EXIT:;
	return result;
}

//******************************************************************************
// 回転対応
//******************************************************************************
int MTFirstPersonCam::_TransformRolling(
		int dW
	)
{
	int result = 0;
	float dt = 0.0f;
	float domega = 0.0f;

	//デルタタイム
	dt = (float)m_DeltaTime / 1000.0f;

	//ホイール移動量から角度を算出
	domega = (float)dW * m_VelocityManualRoll * dt;

	//極端な角度の変化を抑止する
	//  画面描画が引っかかった場合にマウス移動量が蓄積され
	//  突然あらぬ方向を向いてしまうことを避けたい
	if (abs(domega) > 45.0f) {
		domega = 0.0f;
	}

	//自動回転
	if (m_IsAutoRollMode) {
		domega += m_VelocityAutoRoll * dt;
	}

	//回転角度更新
	m_RollAngle += domega;

	//回転角度のクリップ
	if (m_RollAngle >= 360.0f) {
		m_RollAngle -= 360.0f;
	}
	else if (m_RollAngle <= -360.0f) {
		m_RollAngle += 360.0f;
	}

//EXIT:;
	return result;
}

//******************************************************************************
// 手動回転角度取得
//******************************************************************************
float MTFirstPersonCam::GetManualRollAngle()
{
	return m_RollAngle;
}

//******************************************************************************
// 手動回転角度設定
//******************************************************************************
void MTFirstPersonCam::SetManualRollAngle(
		float rollAngle
	)
{
	m_RollAngle = rollAngle;
}

//******************************************************************************
// 自動回転速度取得
//******************************************************************************
float MTFirstPersonCam::GetAutoRollVelocity()
{
	return m_VelocityAutoRoll;
}

//******************************************************************************
// 自動回転速度設定
//******************************************************************************
void MTFirstPersonCam::SetAutoRollVelocity(
		float rollVelocity
	)
{
	m_VelocityAutoRoll = rollVelocity;
}

//******************************************************************************
// カメラ位置設定
//******************************************************************************
int MTFirstPersonCam::_SetCamPosition()
{
	int result = 0;
	float phiRad = 0.0f;
	float thetaRad = 0.0f;
	D3DXVECTOR3 lookVector;
	D3DXVECTOR3 camLookAtVector;
	D3DXVECTOR3 camUpVector;

	//視線ベクトル（極座標から直交座標へ変換）
	phiRad    = D3DXToRadian(m_CamDirPhi);
	thetaRad  = D3DXToRadian(m_CamDirTheta);
	lookVector.x = 10.0f * sin(thetaRad) * cos(phiRad);
	lookVector.y = 10.0f * cos(thetaRad);
	lookVector.z = 10.0f * sin(thetaRad) * sin(phiRad);

	//カメラ位置に視線ベクトルを足して注目点を算出
	camLookAtVector = m_CamVector;
	camLookAtVector.x += lookVector.x;
	camLookAtVector.y += lookVector.y;
	camLookAtVector.z += lookVector.z;

	//カメラ上方向
	camUpVector = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	//カメラ位置登録
	m_Camera.SetPosition(
			m_CamVector,		//カメラ位置
			camLookAtVector, 	//注目点
			camUpVector			//カメラ上方向
		);

	return result;
}

//******************************************************************************
// カーソル移動範囲制限
//******************************************************************************
int MTFirstPersonCam::_ClipCursor(
		bool isClip
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	RECT wrect;
	RECT crect;
	RECT clip;
	int wh = 0;
	int ww = 0;
	int ch = 0;
	int cw = 0;

	if (isClip) {
		bresult = GetWindowRect(m_hWnd, &wrect);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
		bresult = GetClientRect(m_hWnd, &crect);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
		wh = wrect.bottom - wrect.top;
		ww = wrect.right  - wrect.left;
		ch = crect.bottom - crect.top;
		cw = crect.right  - crect.left;
		clip = wrect;
		clip.left   += +(ww - cw);
		clip.right  += -(ww - cw);
		clip.top    += +(wh - ch);
		clip.bottom += -(wh - ch);
		bresult = ClipCursor(&clip);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	else {
		ClipCursor(NULL);
	}

EXIT:;
	return result;
}

//******************************************************************************
// デルタタイム取得
//******************************************************************************
void MTFirstPersonCam::_CalcDeltaTime()
{
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	if (m_PrevTime == 0) {
		//初回測定時は変化なしとする
		m_DeltaTime = 0;
	}
	else {
		//デルタタイム
		//49.71日をまたぐ場合もこの計算で問題ないはず
		m_DeltaTime = curTime - m_PrevTime;
	}
	
	m_PrevTime = curTime;
	
	return;
}

//******************************************************************************
// チックタイム設定
//******************************************************************************
void MTFirstPersonCam::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTFirstPersonCam::Reset()
{
	m_PrevTime = 0;
	m_DeltaTime = 0;
	m_PrevTickTime = 0;
	m_CurTickTime = 0;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTFirstPersonCam::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//カメラ移動速度情報取得
	result = confFile.SetCurSection(_T("FirstPersonCam"));
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityFB"), &m_VelocityFB, 15.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityLR"), &m_VelocityLR, 15.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityUD"), &m_VelocityUD, 10.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityPT"), &m_VelocityPT, 6.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("AcceleRate"), &m_AcceleRate, 2.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityAutoRoll"), &m_VelocityAutoRoll, 6.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityManualRoll"), &m_VelocityManualRoll, 1.0f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// カメラ位置クリッピング
//******************************************************************************
void MTFirstPersonCam::_ClipCamVector(
		D3DXVECTOR3* pVector
	)
{
	if (pVector->x < -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT)) {
		pVector->x = -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT);
	}
	if (pVector->x > MTFIRSTPERSONCAM_CAMVECTOR_LIMIT) {
		pVector->x = MTFIRSTPERSONCAM_CAMVECTOR_LIMIT;
	}
	if (pVector->y < -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT)) {
		pVector->y = -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT);
	}
	if (pVector->y > MTFIRSTPERSONCAM_CAMVECTOR_LIMIT) {
		pVector->y = MTFIRSTPERSONCAM_CAMVECTOR_LIMIT;
	}
	if (pVector->z < -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT)) {
		pVector->z = -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT);
	}
	if (pVector->z > MTFIRSTPERSONCAM_CAMVECTOR_LIMIT) {
		pVector->z = MTFIRSTPERSONCAM_CAMVECTOR_LIMIT;
	}
}

//******************************************************************************
// 進行方向設定
//******************************************************************************
void MTFirstPersonCam::SetProgressDirection(
		MTProgressDirection dir
	)
{
	m_ProgressDirection = dir;
}


