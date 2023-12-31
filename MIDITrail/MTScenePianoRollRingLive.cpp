//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingLiveLive
//
// ライブモニタ用ピアノロールリングシーン描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTScenePianoRollRingLive.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRollRingLive::MTScenePianoRollRingLive()
{
	m_IsEnableLight = false;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_IsSkipping = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRollRingLive::~MTScenePianoRollRingLive()
{
	Release();
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTScenePianoRollRingLive::GetName()
{
	return _T("PianoRollRingLive");
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTScenePianoRollRingLive::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	
	//ライブモニタのため pSeqData には NULL が指定される
	
	Release();
	
	if (pD3DDevice == NULL) {
		result =  YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//設定ファイル読み込み
	result = _LoadConf();
	if (result != 0) goto EXIT;
	
	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//----------------------------------
	// カメラ
	//----------------------------------
	//カメラ初期化
	result = m_FirstPersonCam.Initialize(hWnd, GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//デフォルト視点を取得
	GetDefaultViewParam(&m_ViewParamMap);
	
	//視点を設定
	SetViewParam(&m_ViewParamMap);
	
	//----------------------------------
	// ライト
	//----------------------------------
	//ライト無効
	m_IsEnableLight = false;

	//ライト初期化
	result = m_DirLight.Initialize();
	if (result != 0) goto EXIT;
	
	//ライト方向
	//  原点を光源としてその方向をベクトルで表現する
	m_DirLight.SetDirection(D3DXVECTOR3(-1.0f, 0.0f, 0.0f));
	
	//ライトのデバイス登録
	result = m_DirLight.SetDevice(pD3DDevice, m_IsEnableLight);
	if (result != 0) goto EXIT;
	
	//----------------------------------
	// 描画オブジェクト
	//----------------------------------
	//ピッチベンド情報初期化
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;
	
	//ノートボックス生成
	result = m_NoteBoxLive.Create(pD3DDevice, GetName(), &m_NotePitchBend);
	if (result != 0) goto EXIT;
	
	//ノート波紋生成
	result = m_NoteRipple.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;
	
	//グリッドボックス生成
	result = m_GridBoxLive.Create(pD3DDevice, GetName());
	if (result != 0) goto EXIT;
	
	//ピクチャボード生成
	result = m_PictBoard.Create(pD3DDevice, GetName(), pSeqData, true);
	if (result != 0) goto EXIT;
	
	//ダッシュボード生成
	result = m_DashboardLive.Create(pD3DDevice, GetName(), hWnd);
	if (result != 0) goto EXIT;
	
	//星生成
	result = m_Stars.Create(pD3DDevice, GetName(), &m_DirLight);
	if (result != 0) goto EXIT;
	
	//タイムインジケータ生成
	result = m_TimeIndicator.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//メッシュ制御生成
	result = m_MeshCtrl.Create(pD3DDevice, GetName());
	if (result != 0) goto EXIT;
	
	//背景画像生成
	result = m_BackgroundImage.Create(pD3DDevice, hWnd);
	if (result != 0) goto EXIT;
	
	//----------------------------------
	// レンダリングステート
	//----------------------------------
	//画面描画モード
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	
	//Z深度比較：ON
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	
	//ディザリング:ON 高品質描画
	pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	
	//マルチサンプリングアンチエイリアス：有効
	pD3DDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	
	//レンダリングステート設定：通常のアルファ合成
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	
EXIT:;
	return result;
}

//******************************************************************************
// 変換処理
//******************************************************************************
int MTScenePianoRollRingLive::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;
	
	if (pD3DDevice == NULL) {
		result =  YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//カメラ更新
	result = m_FirstPersonCam.Transform(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//カメラ座標取得
	m_FirstPersonCam.GetPosition(&camVector);
	
	//回転角度取得
	rollAngle = m_FirstPersonCam.GetManualRollAngle();
	
	//ノートボックス更新
	result = m_NoteBoxLive.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;
	
	//グリッドボックス更新
	result = m_GridBoxLive.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;
	
	//ピクチャボード更新
	result = m_PictBoard.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;
	
	//ダッシュボード更新
	result = m_DashboardLive.Transform(pD3DDevice, camVector);
	if (result != 0) goto EXIT;
	
	//星更新
	result = m_Stars.Transform(pD3DDevice, camVector);
	if (result != 0) goto EXIT;
	
	//メッシュ更新
	result = m_MeshCtrl.Transform(pD3DDevice, m_TimeIndicator.GetMoveVector());
	if (result != 0) goto EXIT;

	//タイムインジケータ更新
	result = m_TimeIndicator.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;
	
	//ノート波紋更新
	result = m_NoteRipple.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTScenePianoRollRingLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	if (pD3DDevice == NULL) {
		result =  YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//更新
	result = Transform(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//背景画像描画
	result = m_BackgroundImage.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//グリッドボックス描画
	result = m_GridBoxLive.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//ノートボックス描画
	result = m_NoteBoxLive.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//ピクチャボード描画
	result = m_PictBoard.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//星描画
	result = m_Stars.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//メッシュ描画
	result = m_MeshCtrl.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//タイムインジケータ描画
	result = m_TimeIndicator.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//ノート波紋描画
	result = m_NoteRipple.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//ダッシュボード描画：正射影のため一番最後に描画する
	result = m_DashboardLive.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 破棄
//******************************************************************************
void MTScenePianoRollRingLive::Release()
{
	m_NoteBoxLive.Release();
	m_GridBoxLive.Release();
	m_PictBoard.Release();
	m_DashboardLive.Release();
	m_Stars.Release();
	m_TimeIndicator.Release();
	m_NoteRipple.Release();
	m_MeshCtrl.Release();
	m_BackgroundImage.Release();
}

//******************************************************************************
// ウィンドウクリックイベント受信
//******************************************************************************
int MTScenePianoRollRingLive::OnWindowClicked(
		UINT button,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	
	//左ボタン
	if (button == WM_LBUTTONDOWN) {
		//視線方向制御 ON/OFF
		m_IsMouseCamMode = m_IsMouseCamMode ? false : true;
		m_FirstPersonCam.SetMouseCamMode(m_IsMouseCamMode);
	}
	//右ボタン
	else if (button == WM_RBUTTONDOWN) {
		//何もしない
	}
	//中ボタン
	else if (button == WM_MBUTTONDOWN) {
		//自動回転モード ON/OFF
		m_IsAutoRollMode = m_IsAutoRollMode ? false : true;
		m_FirstPersonCam.SetAutoRollMode(m_IsAutoRollMode);
		if (m_IsAutoRollMode) {
			m_FirstPersonCam.SwitchAutoRllDirecton();
		}
	}
	
	return result;
}

//******************************************************************************
// 演奏開始イベント受信
//******************************************************************************
int MTScenePianoRollRingLive::OnPlayStart(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	_Reset();
	
	m_PictBoard.OnPlayStart();
	
	m_DashboardLive.SetMonitoringStatus(true);
	result = m_DashboardLive.SetMIDIINDeviceName(pD3DDevice, GetParam(_T("MIDI_IN_DEVICE_NAME")));
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 演奏終了イベント受信
//******************************************************************************
int MTScenePianoRollRingLive::OnPlayEnd(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	m_NoteBoxLive.AllNoteOff();
	m_PictBoard.OnPlayEnd();
	
	m_DashboardLive.SetMonitoringStatus(false);
	
//EXIT:;
	return result;
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MTScenePianoRollRingLive::OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;
	
	parser.Parse(param1, param2);
	
	//演奏状態通知
	if (parser.GetMsg() == SMMsgParser::MsgPlayStatus) {
		if (parser.GetPlayStatus() == SMMsgParser::StatusStop) {
			//停止（終了）
		}
		else if (parser.GetPlayStatus() == SMMsgParser::StatusPlay) {
			//演奏
		}
		else if (parser.GetPlayStatus() == SMMsgParser::StatusPause) {
			//一時停止
		}
	}
	//ノートOFF通知
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOff) {
		m_NoteBoxLive.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
		m_NoteRipple.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
	}
	//ノートON通知
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		m_DashboardLive.SetNoteOn();
		m_NoteBoxLive.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
		m_NoteRipple.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
	}
	//ピッチベンド通知
	else if (parser.GetMsg() == SMMsgParser::MsgPitchBend) {
		m_NotePitchBend.SetPitchBend(
							parser.GetPortNo(),
							parser.GetChNo(),
							parser.GetPitchBendValue(),
							parser.GetPitchBendSensitivity()
						);
	}
	//オールノートOFF通知
	else if (parser.GetMsg() == SMMsgParser::MsgAllNoteOff) {
		m_NoteBoxLive.AllNoteOffOnCh(parser.GetPortNo(), parser.GetChNo());
	}
	
	//EXIT:;
	return result;
}

//******************************************************************************
// 巻き戻し
//******************************************************************************
int MTScenePianoRollRingLive::Rewind()
{
	int result = 0;
	
	_Reset();
	
	//視点を設定
	SetViewParam(&m_ViewParamMap);
	
	return result;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTScenePianoRollRingLive::_Reset()
{
	m_DashboardLive.Reset();
	m_FirstPersonCam.Reset();
	m_TimeIndicator.Reset();
	m_PictBoard.Reset();
	m_NoteBoxLive.Reset();
	m_NoteRipple.Reset();
	m_NotePitchBend.Reset();
}

//******************************************************************************
// デフォルト視点取得
//******************************************************************************
void MTScenePianoRollRingLive::GetDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	D3DXVECTOR3 baseVector;
	D3DXVECTOR3 moveVctor;
	float phi = 0.0f;
	float theta= 0.0f;
	
	//デフォルトのカメラ座標
	baseVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	
	//世界座標配置移動ベクトル取得
	moveVctor = m_NoteDesign.GetWorldMoveVector();
	
	//視点情報作成
	viewPointVector.x =  baseVector.x + moveVctor.x + 13.0f;
	viewPointVector.y =  baseVector.y + moveVctor.y;
	viewPointVector.z =  baseVector.z + moveVctor.z;
	phi      =  180.0f;	//-X軸方向
	theta    =  90.0f;	//+X軸方向
	
	pParamMap->clear();
	pParamMap->insert(MTViewParamMapPair("X", viewPointVector.x));
	pParamMap->insert(MTViewParamMapPair("Y", viewPointVector.y));
	pParamMap->insert(MTViewParamMapPair("Z", viewPointVector.z));
	pParamMap->insert(MTViewParamMapPair("Phi", phi));
	pParamMap->insert(MTViewParamMapPair("Theta", theta));
	pParamMap->insert(MTViewParamMapPair("ManualRollAngle", 0.0f));
	pParamMap->insert(MTViewParamMapPair("AutoRollVelocity", 0.0f));
	
	return;
}

//******************************************************************************
// 視点取得
//******************************************************************************
void MTScenePianoRollRingLive::GetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	float phi = 0.0f;
	float theta = 0.0f;
	float manualRollAngle = 0.0f;
	float autoRollVelocity = 0.0f;
	
	//カメラの位置と方向を取得
	m_FirstPersonCam.GetPosition(&viewPointVector);
	m_FirstPersonCam.GetDirection(&phi, &theta);
	
	//再生面に対する視点であるためX軸方向は再生位置を考慮する
	viewPointVector.x -= m_TimeIndicator.GetPos();
	
	//回転角度を取得
	manualRollAngle = m_FirstPersonCam.GetManualRollAngle();
	if (m_IsAutoRollMode) {
		autoRollVelocity = m_FirstPersonCam.GetAutoRollVelocity();
	}
	
	pParamMap->clear();
	pParamMap->insert(MTViewParamMapPair("X", viewPointVector.x));
	pParamMap->insert(MTViewParamMapPair("Y", viewPointVector.y));
	pParamMap->insert(MTViewParamMapPair("Z", viewPointVector.z));
	pParamMap->insert(MTViewParamMapPair("Phi", phi));
	pParamMap->insert(MTViewParamMapPair("Theta", theta));
	pParamMap->insert(MTViewParamMapPair("ManualRollAngle", manualRollAngle));
	pParamMap->insert(MTViewParamMapPair("AutoRollVelocity", autoRollVelocity));
	
	return;
}

//******************************************************************************
// 視点登録
//******************************************************************************
void MTScenePianoRollRingLive::SetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float phi = 0.0f;
	float theta = 0.0f;
	float manualRollAngle = 0.0f;
	float autoRollVelocity = 0.0f;
	MTViewParamMap::iterator itr;
	
	itr = pParamMap->find("X");
	if (itr != pParamMap->end()) {
		viewPointVector.x = itr->second;
	}
	itr = pParamMap->find("Y");
	if (itr != pParamMap->end()) {
		viewPointVector.y = itr->second;
	}
	itr = pParamMap->find("Z");
	if (itr != pParamMap->end()) {
		viewPointVector.z = itr->second;
	}
	itr = pParamMap->find("Phi");
	if (itr != pParamMap->end()) {
		phi = itr->second;
	}
	itr = pParamMap->find("Theta");
	if (itr != pParamMap->end()) {
		theta = itr->second;
	}
	itr = pParamMap->find("ManualRollAngle");
	if (itr != pParamMap->end()) {
		manualRollAngle = itr->second;
	}
	itr = pParamMap->find("AutoRollVelocity");
	if (itr != pParamMap->end()) {
		autoRollVelocity = itr->second;
	}
	
	//再生面に対する視点であるためX軸方向は再生位置を考慮する
	viewPointVector.x += m_TimeIndicator.GetPos();
	
	//カメラの位置と方向を設定
	m_FirstPersonCam.SetPosition(viewPointVector);
	m_FirstPersonCam.SetDirection(phi, theta);
	
	//手動回転角度を設定
	m_FirstPersonCam.SetManualRollAngle(manualRollAngle);
	
	//自動回転速度を設定
	m_IsAutoRollMode = false;
	if (autoRollVelocity != 0.0f) {
		m_IsAutoRollMode = true;
		m_FirstPersonCam.SetAutoRollVelocity(autoRollVelocity);
	}
	m_FirstPersonCam.SetAutoRollMode(m_IsAutoRollMode);
	
	//パラメータの保存
	if (pParamMap != (&m_ViewParamMap)) {
		m_ViewParamMap.clear();
		for (itr = pParamMap->begin(); itr != pParamMap->end(); itr++) {
			m_ViewParamMap.insert(MTViewParamMapPair(itr->first, itr->second));
		}
	}
	
	return;
}

//******************************************************************************
// 静的視点移動
//******************************************************************************
void MTScenePianoRollRingLive::MoveToStaticViewpoint(
		unsigned long viewpointNo
	)
{
	MTScene::MTViewParamMap::iterator itr;
	MTViewParamMap paramMap;

	if (viewpointNo == 1) {
		GetDefaultViewParam(&paramMap);
		SetViewParam(&paramMap);
	}
	else if (viewpointNo == 2) {
		SetViewParam(&m_Viewpoint2);
	}
	else if (viewpointNo == 3) {
		SetViewParam(&m_Viewpoint3);
	}
	else {
		GetDefaultViewParam(&paramMap);
		SetViewParam(&paramMap);
	}

	return;
}

//******************************************************************************
// 視点リセット
//******************************************************************************
void MTScenePianoRollRingLive::ResetViewpoint()
{
	MTViewParamMap paramMap;
	
	//デフォルト視点を取得
	GetDefaultViewParam(&paramMap);
	
	//視点登録
	SetViewParam(&paramMap);
}

//******************************************************************************
// 表示効果設定
//******************************************************************************
void MTScenePianoRollRingLive::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	switch (type) {
		case EffectPianoKeyboard:
			m_PictBoard.SetEnable(isEnable);
			break;
		case EffectRipple:
			m_NoteRipple.SetEnable(isEnable);
			break;
		case EffectPitchBend:
			m_NotePitchBend.SetEnable(isEnable);
			break;
		case EffectStars:
			m_Stars.SetEnable(isEnable);
			break;
		case EffectCounter:
			m_DashboardLive.SetEnable(isEnable);
			break;
		case EffectBackgroundImage:
			m_BackgroundImage.SetEnable(isEnable);
			break;
		default:
			break;
	}
	
	return;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTScenePianoRollRingLive::_LoadConf()
{
	int result = 0;
	TCHAR hexColor[16] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(GetName());
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;

	result = confFile.GetStr(_T("BackGroundRGB"), hexColor, 16, _T("000000"));
	if (result != 0) goto EXIT;

	SetBGColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));

	result = _LoadConfViewpoint(&confFile, 2, &m_Viewpoint2);
	if (result != 0) goto EXIT;

	result = _LoadConfViewpoint(&confFile, 3, &m_Viewpoint3);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル読み込み：視点
//******************************************************************************
int MTScenePianoRollRingLive::_LoadConfViewpoint(
		MTConfFile* pConfFile,
		unsigned long viewpointNo,
		MTViewParamMap* pParamMap
	)
{
	int result = 0;
	int eresult = 0;
	TCHAR sectionStr[32] = {0};
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float phi = 0.0f;
	float theta = 0.0f;
	float manualRollAngle = 0.0f;
	float autoRollVelocity = 0.0f;

	//セクション名作成
	eresult = _stprintf_s(sectionStr, 32, _T("Viewpoint-%d"), viewpointNo);
	if (eresult < 0) {
		result = YN_SET_ERR("Program error.", viewpointNo, 0);
		goto EXIT;
	}

	//セクション設定
	result = pConfFile->SetCurSection(sectionStr);
	if (result != 0) goto EXIT;

	//パラメータ取得
	result = pConfFile->GetFloat(_T("X"), &x, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Y"), &y, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Z"), &z, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Phi"), &phi, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Theta"), &theta, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("ManualRollAngle"), &manualRollAngle, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("AutoRollVelocity"), &autoRollVelocity, 0.0f);
	if (result != 0) goto EXIT;

	//マップ登録
	pParamMap->clear();
	pParamMap->insert(MTViewParamMapPair("X", x));
	pParamMap->insert(MTViewParamMapPair("Y", y));
	pParamMap->insert(MTViewParamMapPair("Z", z));
	pParamMap->insert(MTViewParamMapPair("Phi", phi));
	pParamMap->insert(MTViewParamMapPair("Theta", theta));
	pParamMap->insert(MTViewParamMapPair("ManualRollAngle", manualRollAngle));
	pParamMap->insert(MTViewParamMapPair("AutoRollVelocity", autoRollVelocity));

EXIT:;
	return result;
}

