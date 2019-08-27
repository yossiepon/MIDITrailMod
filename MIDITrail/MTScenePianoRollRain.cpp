//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain
//
// ピアノロールレインシーン描画クラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTScenePianoRollRain.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRollRain::MTScenePianoRollRain(void)
{
	m_IsEnableLight = TRUE;
	m_IsSingleKeyboard = false;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_CurTickTime = 0;
	m_IsSkipping = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRollRain::~MTScenePianoRollRain(void)
{
	Release();
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTScenePianoRollRain::GetName()
{
	return _T("PianoRollRain");
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTScenePianoRollRain::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//設定ファイル読み込み
	result = _LoadConf();
	if (result != 0) goto EXIT;

	//----------------------------------
	// カメラ
	//----------------------------------
	//カメラ初期化
	result = m_FirstPersonCam.Initialize(hWnd, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	//進行方向
	m_FirstPersonCam.SetProgressDirection(MTFirstPersonCam::DirY);

	//デフォルト視点を取得
	GetDefaultViewParam(&m_ViewParamMap);

	//視点を設定
	SetViewParam(&m_ViewParamMap);

	//----------------------------------
	// ライト
	//----------------------------------
	//ライト初期化
	result = m_DirLight.Initialize();
	if (result != 0) goto EXIT;

	//ライト色
	_SetLightColor(&m_DirLight);

	//ライト方向
	//m_DirLight.SetDirection(D3DXVECTOR3(1.0f, -1.0f, 2.0f));
	m_DirLight.SetDirection(D3DXVECTOR3(1.0f, -2.0f, 0.5f));

	//ライトのデバイス登録
	result = m_DirLight.SetDevice(pD3DDevice, m_IsEnableLight);
	if (result != 0) goto EXIT;

	//----------------------------------
	// 描画オブジェクト
	//----------------------------------
	//ピッチベンド情報初期化
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;

	//シングルキーボードはピッチベンド無効
	if (m_IsSingleKeyboard) {
		m_NotePitchBend.SetEnable(false);
	}
	else {
		m_NotePitchBend.SetEnable(true);
	}

	//ピアノキーボード制御
	result = m_PianoKeyboardCtrl.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend, m_IsSingleKeyboard);
	if (result != 0) goto EXIT;

	//ノートレイン
	result = m_NoteRain.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//ダッシュボード生成
	result = m_Dashboard.Create(pD3DDevice, GetName(), pSeqData, hWnd);
	if (result != 0) goto EXIT;

	//星生成
	result = m_Stars.Create(pD3DDevice, GetName(), &m_DirLight);
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
	//裏を向くポリゴンは描画しない（カリング）することにより負荷を下げる
	//pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); //カリングしない
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW); //カリングする

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
int MTScenePianoRollRain::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;
	D3DXVECTOR3 moveVector;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//カメラ更新
	result = m_FirstPersonCam.Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//カメラ座標取得
	 m_FirstPersonCam.GetPosition(&camVector);

	//回転角度取得
	rollAngle = m_FirstPersonCam.GetManualRollAngle();

	//ピアノキーボード更新
	result = m_PianoKeyboardCtrl.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//ノートレイン更新
	result = m_NoteRain.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//ダッシュボード更新
	result = m_Dashboard.Transform(pD3DDevice, camVector);
	if (result != 0) goto EXIT;

	//星更新
	result = m_Stars.Transform(pD3DDevice, camVector);
	if (result != 0) goto EXIT;

	//メッシュ更新
	moveVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	result = m_MeshCtrl.Transform(pD3DDevice, moveVector);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTScenePianoRollRain::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//更新
	result = Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//背景画像描画
	result = m_BackgroundImage.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//ピアノキーボード描画
	result = m_PianoKeyboardCtrl.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//ノートレイン更新
	result = m_NoteRain.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//星描画
	result = m_Stars.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//メッシュ描画
	result = m_MeshCtrl.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//ダッシュボード描画：座標変換済み頂点を用いるため一番最後に描画する
	result = m_Dashboard.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 破棄
//******************************************************************************
void MTScenePianoRollRain::Release()
{
	m_PianoKeyboardCtrl.Release();
	m_NoteRain.Release();
	m_Dashboard.Release();
	m_Stars.Release();
	m_MeshCtrl.Release();
	m_BackgroundImage.Release();
}

//******************************************************************************
// ウィンドウクリックイベント受信
//******************************************************************************
int MTScenePianoRollRain::OnWindowClicked(
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
int MTScenePianoRollRain::OnPlayStart(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	_Reset();

	return result;
}

//******************************************************************************
// 演奏終了イベント受信
//******************************************************************************
int MTScenePianoRollRain::OnPlayEnd(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	return result;
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MTScenePianoRollRain::OnRecvSequencerMsg(
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
	//演奏チックタイム通知
	else if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		m_Dashboard.SetPlayTimeSec(parser.GetPlayTimeSec());
		m_PianoKeyboardCtrl.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteRain.SetCurTickTime(parser.GetPlayTickTime());
		m_CurTickTime = parser.GetPlayTickTime();

		//ノートを移動せずにカメラとキーボードを移動させる場合
		//m_FirstPersonCam.SetCurTickTime(parser.GetPlayTickTime());
		//m_PianoKeyboardCtrl.SetCurTickTime(parser.GetPlayTickTime());
	}
	//テンポ変更通知
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		m_Dashboard.SetTempoBPM(parser.GetTempoBPM());
	}
	//小節番号通知
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		m_Dashboard.SetBarNo(parser.GetBarNo());
	}
	//拍子記号変更通知
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		m_Dashboard.SetBeat(parser.GetBeatNumerator(), parser.GetBeatDenominator());
	}
	//ノートOFF通知
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOff) {
	}
	//ノートON通知
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		m_Dashboard.SetNoteOn();
	}
	//ピッチベンド通知
	else if (parser.GetMsg() == SMMsgParser::MsgPitchBend) {
		m_NotePitchBend.SetPitchBend(parser.GetPortNo(), parser.GetChNo(), parser.GetPitchBendValue(), parser.GetPitchBendSensitivity());
	}
	//スキップ開始通知
	else if (parser.GetMsg() == SMMsgParser::MsgSkipStart) {
		if (parser.GetSkipStartDirection() == SMMsgParser::SkipBack) {
			m_NotePitchBend.Reset();
		}
		m_PianoKeyboardCtrl.Reset();
		m_PianoKeyboardCtrl.SetSkipStatus(true);
		m_NoteRain.Reset();
		m_NoteRain.SetSkipStatus(true);
		m_NoteRain.SetCurTickTime(m_CurTickTime);
		m_IsSkipping = true;
	}
	//スキップ終了通知
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_Dashboard.SetNotesCount(parser.GetSkipEndNotesCount());
		m_PianoKeyboardCtrl.SetSkipStatus(false);
		m_NoteRain.SetSkipStatus(false);
		m_IsSkipping = false;
	}

//EXIT:;
	return result;
}

//******************************************************************************
// 巻き戻し
//******************************************************************************
int MTScenePianoRollRain::Rewind()
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
void MTScenePianoRollRain::_Reset()
{
	m_Dashboard.Reset();
	m_FirstPersonCam.Reset();
	m_PianoKeyboardCtrl.Reset();
	m_NoteRain.Reset();
	m_NotePitchBend.Reset();
	m_CurTickTime = 0;
}

//******************************************************************************
// デフォルト視点取得
//******************************************************************************
void MTScenePianoRollRain::GetDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	float phi, theta = 0.0f;

	//視点情報作成
	viewPointVector.x = 0.0f;
	viewPointVector.y = 0.0f;
	viewPointVector.z = - (1.5f * 16.0f / 2.0f) - 10.0f;
	phi   = 90.0f;
	theta = 90.0f;

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
void MTScenePianoRollRain::GetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	float phi, theta = 0.0f;
	float manualRollAngle = 0.0f;
	float autoRollVelocity = 0.0f;

	//カメラの位置と方向を取得
	m_FirstPersonCam.GetPosition(&viewPointVector);
	m_FirstPersonCam.GetDirection(&phi, &theta);

	//ノートを移動せずにカメラとキーボードを移動させる場合
	//再生面に対する視点であるためY軸方向は再生位置を考慮する
	//viewPointVector.y -= m_NoteRain.GetPos();

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
void MTScenePianoRollRain::SetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float phi, theta = 0.0f;
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

	//ノートを移動せずにカメラとキーボードを移動させる場合
	//再生面に対する視点であるためY軸方向は再生位置を考慮する
	//viewPointVector.y += m_NoteRain.GetPos();

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
void MTScenePianoRollRain::MoveToStaticViewpoint(
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
void MTScenePianoRollRain::ResetViewpoint()
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
void MTScenePianoRollRain::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	switch (type) {
		case EffectPianoKeyboard:
			m_PianoKeyboardCtrl.SetEnable(isEnable);
			break;
		case EffectRipple:
			break;
		case EffectPitchBend:
			if (!m_IsSingleKeyboard) {
				m_NotePitchBend.SetEnable(isEnable);
			}
			break;
		case EffectStars:
			m_Stars.SetEnable(isEnable);
			break;
		case EffectCounter:
			m_Dashboard.SetEnable(isEnable);
			break;
		case EffectFileName:
			m_Dashboard.SetEnableFileName(isEnable);
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
// 演奏速度設定
//******************************************************************************
void MTScenePianoRollRain::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

//******************************************************************************
// ライト色設定
//******************************************************************************
void MTScenePianoRollRain::_SetLightColor(
		DXDirLight* pLight
	)
{
	D3DXCOLOR diffuse;
	D3DXCOLOR specular;
	D3DXCOLOR ambient;

	//拡散光
	diffuse.r = 1.0f;
	diffuse.g = 1.0f;
	diffuse.b = 1.0f;
	diffuse.a = 1.0f;
	//鏡面反射光
	specular.r = 0.0f;
	specular.g = 0.0f;
	specular.b = 0.0f;
	specular.a = 0.0f;
	//環境光
	ambient.r = 0.5f;
	ambient.g = 0.5f;
	ambient.b = 0.5f;
	ambient.a = 1.0f;

	pLight->SetColor(diffuse, specular, ambient);

	return;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTScenePianoRollRain::_LoadConf()
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
int MTScenePianoRollRain::_LoadConfViewpoint(
		MTConfFile* pConfFile,
		unsigned long viewpointNo,
		MTViewParamMap* pParamMap
	)
{
	int result = 0;
	int eresult = 0;
	TCHAR sectionStr[32] = {0};
	float x, y, z = 0.0f;
	float phi, theta = 0.0f;
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

