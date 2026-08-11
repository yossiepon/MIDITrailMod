//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D11
//
// DX11 PianoRoll 3D/2D scene.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll3D11.h"
#include "MTGridBox11.h"
#include "MTGridBoxLive11.h"
#include "MTPianoKeyboardCtrlRoll11.h"
#include "MTPianoKeyboardCtrlRollLive11.h"
#include "SMMsgParser.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRoll3D11::MTScenePianoRoll3D11(bool isLive, bool is2D)
{
	m_IsLive = isLive;
	m_Is2D = is2D;
	m_hWnd = NULL;
	m_pGrid = NULL;
	m_pNoteBoxLive = NULL;
	m_pKeyboardCtrl = NULL;

	// シーン固有のプロパティ
	m_Traits.cameraDir = MTCameraDirX;
	m_Traits.cameraTracksPlayback = true;
	m_Traits.lightEnabled = !is2D;  // 3D: ライトON, 2D: ライトOFF
	m_Traits.lightCount = 2;
	m_Traits.viewpointCompensation = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRoll3D11::~MTScenePianoRoll3D11()
{
	Release();
}

//******************************************************************************
// シーン名取得
//******************************************************************************
const TCHAR* MTScenePianoRoll3D11::GetName() const
{
	if (m_IsLive) {
		return m_Is2D ? _T("PianoRoll2DLive") : _T("PianoRoll3DLive");
	}
	return m_Is2D ? _T("PianoRoll2D") : _T("PianoRoll3D");
}

//******************************************************************************
// 生成
//******************************************************************************
int MTScenePianoRoll3D11::Create(
		HWND hWnd,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData
	)
{
	int result = 0;

	m_pDevice = pDevice;
	m_pContext = pContext;
	m_hWnd = hWnd;

	// 設定読み込み
	_LoadConf();

	// カメラ初期化
	result = m_Camera.Initialize(hWnd, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	m_Camera.SetProgressDirection(MTFirstPersonCam::DirX);

	// デフォルト視点を設定
	{
		MTViewParamMap defaultView;
		_ComputeDefaultViewParam(&defaultView);
		m_Camera.SetViewParam(&defaultView);
	}

	// 星
	result = m_Stars.Create(pDevice, pContext, GetName());
	if (result != 0) goto EXIT;

	// 背景画像
	result = m_BackgroundImage.Create(pDevice, pContext, hWnd);
	if (result != 0) goto EXIT;

	// ピッチベンド（Live/Playback 共通で必要）
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;

	if (m_IsLive) {
		// === Live モード ===

		// Live Notes
		try { m_pNoteBoxLive = new MTNoteAABBLive11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = m_pNoteBoxLive->Create(pDevice, pContext, GetName(), &m_NotePitchBend,
		                                m_Is2D ? MTAABBLiveMode::Roll2D : MTAABBLiveMode::Roll3D);
		if (result != 0) goto EXIT;
		m_pNoteBoxLive->SetLightEnable(!m_Is2D);

		// NoteTrackerLive
		result = m_NoteTrackerLive.Create();
		if (result != 0) goto EXIT;

		// Grid (Live)
		try { m_pGrid = new MTGridBoxLive11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTGridBoxLive11*)m_pGrid)->Create(pDevice, pContext, GetName());
		if (result != 0) goto EXIT;

		// TimeIndicator (pSeqData=NULL OK)
		result = m_TimeIndicator.Create(pDevice, pContext, GetName(), NULL);
		if (result != 0) goto EXIT;

		// Ripple (NoteDesignLive11 injection)
		result = m_NoteDesignLive.Initialize(GetName());
		if (result != 0) goto EXIT;
		result = m_Ripple.Create(pDevice, pContext, GetName(), NULL, &m_NotePitchBend, &m_NoteDesignLive);
		if (result != 0) goto EXIT;
		m_NoteTrackerLive.AddListener(&m_Ripple, NoteEventType::Note);

		// Dashboard (Live monitor mode)
		result = m_Dashboard.Create(pDevice, pContext, GetName(), NULL, hWnd);
		if (result != 0) goto EXIT;
		m_Dashboard.SetMonitorMode(true, _T(""));
		m_NoteTrackerLive.AddListener(&m_Dashboard, NoteEventType::Note);

		// Keyboard (Live)
		try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRollLive11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTPianoKeyboardCtrlRollLive11*)m_pKeyboardCtrl)->Create(
			pDevice, pContext, GetName(), &m_NotePitchBend, true);
		if (result != 0) goto EXIT;
		m_NoteTrackerLive.AddListener((MTPianoKeyboardCtrlRollLive11*)m_pKeyboardCtrl, NoteEventType::Note);

		// コンポーネント登録
		_RegisterComponent(&m_Stars);
		_RegisterComponent(&m_BackgroundImage);
		_RegisterComponent(m_pGrid);
		_RegisterComponent(&m_TimeIndicator);
		_RegisterComponent(&m_Dashboard);
		_RegisterComponent(&m_NotePitchBend);
		_RegisterComponent(&m_NoteTrackerLive);
		_RegisterComponent(&m_Ripple);
		_RegisterComponent(m_pNoteBoxLive);
		_RegisterComponent(m_pKeyboardCtrl);
	}
	else {
		// === Playback モード ===

		if (pSeqData == NULL) goto EXIT;

		// Grid (Playback)
		try { m_pGrid = new MTGridBox11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTGridBox11*)m_pGrid)->Create(pDevice, pContext, GetName(), pSeqData);
		if (result != 0) goto EXIT;

		// TimeIndicator
		result = m_TimeIndicator.Create(pDevice, pContext, GetName(), pSeqData);
		if (result != 0) goto EXIT;

		// Dashboard
		result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, hWnd);
		if (result != 0) goto EXIT;

		// NoteTracker
		result = m_NoteTracker.Create(pSeqData);
		if (result != 0) goto EXIT;

		// Ripple / Lyrics
		result = m_Ripple.Create(pDevice, pContext, GetName(), pSeqData, &m_NotePitchBend);
		if (result != 0) goto EXIT;
		m_NoteTracker.AddListener(&m_Ripple, NoteEventType::Note);
		result = m_Lyrics.Create(pDevice, pContext, GetName(), pSeqData, &m_NotePitchBend);
		if (result != 0) goto EXIT;
		m_NoteTracker.AddListener(&m_Lyrics, NoteEventType::Lyric);
		m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

		// NoteBox (Instanced)
		result = m_NoteBox.Create(pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend,
		                          m_Is2D ? MTAABBMode::Roll2D : MTAABBMode::Roll3D);
		if (result != 0) goto EXIT;

		// Keyboard (Playback)
		try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRoll11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTPianoKeyboardCtrlRoll11*)m_pKeyboardCtrl)->Create(
			pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend, false);
		if (result != 0) goto EXIT;

		// コンポーネント登録
		_RegisterComponent(&m_Stars);
		_RegisterComponent(&m_BackgroundImage);
		_RegisterComponent(m_pGrid);
		_RegisterComponent(&m_TimeIndicator);
		_RegisterComponent(&m_Dashboard);
		_RegisterComponent(&m_NotePitchBend);
		_RegisterComponent(&m_NoteTracker);
		_RegisterComponent(&m_Ripple);
		_RegisterComponent(&m_Lyrics);
		_RegisterComponent(&m_NoteBox);
		_RegisterComponent(m_pKeyboardCtrl);
	}

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTScenePianoRoll3D11::Release()
{
	m_NoteTracker.RemoveListener(&m_Ripple);
	m_NoteTracker.RemoveListener(&m_Lyrics);
	delete m_pKeyboardCtrl;
	m_pKeyboardCtrl = NULL;
	m_NoteBox.Release();
	delete m_pNoteBoxLive;
	m_pNoteBoxLive = NULL;
	m_Ripple.Release();
	m_Lyrics.Release();
	m_NoteTracker.Release();
	m_Stars.Release();
	delete m_pGrid;
	m_pGrid = NULL;
	m_TimeIndicator.Release();
	m_BackgroundImage.Release();
	m_Dashboard.Release();

	MTSceneBase11::Release();
}

//******************************************************************************
// フレーム更新
//******************************************************************************
int MTScenePianoRoll3D11::_UpdateComponents(
		const MTSceneUpdateContext& ctx
	)
{
	return 0;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTScenePianoRoll3D11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;

	// 背景画像（最初に描画）
	result = m_BackgroundImage.Draw(pContext);
	if (result != 0) goto EXIT;

	// シーン固有コンポーネント描画
	result = _DrawSceneComponents(pContext, viewProj, rollAngle, camPos);
	if (result != 0) goto EXIT;

	// ダッシュボード（最後に描画）
	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		result = m_Dashboard.Draw(pContext,
		                          rect.right - rect.left,
		                          rect.bottom - rect.top);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// シーン固有コンポーネント描画
//******************************************************************************
int MTScenePianoRoll3D11::_DrawSceneComponents(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;
	Vector4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);

	// Grid
	if (m_pGrid != NULL) {
		result = m_pGrid->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

	// Notes
	if (m_pNoteBoxLive != NULL) {
		result = m_pNoteBoxLive->Draw(pContext, viewProj, lightDir);
	}
	else {
		result = m_NoteBox.Draw(pContext, viewProj, lightDir);
	}
	if (result != 0) goto EXIT;

	// カメラ位置と再生位置の前後関係で描画順を切り替え（奥から手前へ）
	if (m_TimeIndicator.GetPos() > camPos.x) {
		result = m_TimeIndicator.Draw(pContext, viewProj, lightDir, rollAngle);
		if (result != 0) goto EXIT;
		result = m_Lyrics.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		if (m_pKeyboardCtrl != NULL) {
			result = m_pKeyboardCtrl->Draw(pContext, viewProj, lightDir);
			if (result != 0) goto EXIT;
		}
	}
	else {
		if (m_pKeyboardCtrl != NULL) {
			result = m_pKeyboardCtrl->Draw(pContext, viewProj, lightDir);
			if (result != 0) goto EXIT;
		}
		result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_Lyrics.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_TimeIndicator.Draw(pContext, viewProj, lightDir, rollAngle);
		if (result != 0) goto EXIT;
	}

	// 星
	result = m_Stars.Draw(pContext, viewProj, rollAngle);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 再生開始
//******************************************************************************
int MTScenePianoRoll3D11::OnPlayStart()
{
	_Reset();
	if (m_IsLive) {
		m_isMonitoringActive = true;
		m_Dashboard.SetMonitoringStatus(true);
		m_Dashboard.SetMIDIINDeviceName(GetParam("MIDI_IN_DEVICE_NAME"));
	}
	return 0;
}

//******************************************************************************
// 再生終了
//******************************************************************************
int MTScenePianoRoll3D11::OnPlayEnd()
{
	m_isMonitoringActive = false;
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
		m_Dashboard.SetMonitoringStatus(false);
	}
	return 0;
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MTScenePianoRoll3D11::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	parser.Parse(param1, param2);

	// テンポ変更通知
	if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		m_Dashboard.SetTempoBPM(parser.GetTempoBPM());
	}
	// 小節番号通知
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		m_Dashboard.SetBarNo(parser.GetBarNo());
	}
	// 拍子記号変更通知
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		m_Dashboard.SetBeat(parser.GetBeatNumerator(), parser.GetBeatDenominator());
	}
	// ピッチベンド通知
	else if (parser.GetMsg() == SMMsgParser::MsgPitchBend) {
		m_NotePitchBend.SetPitchBend(
			parser.GetPortNo(), parser.GetChNo(),
			parser.GetPitchBendValue(), parser.GetPitchBendSensitivity());
	}
	// スキップ開始通知
	else if (parser.GetMsg() == SMMsgParser::MsgSkipStart) {
		if (parser.GetSkipStartDirection() == SMMsgParser::SkipBack) {
			m_NotePitchBend.Reset();
		}
		m_NoteBox.Reset();
		m_NoteBox.SetSkipStatus(true);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->Reset();
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(true);
		m_Ripple.SetSkipStatus(true);
		m_Lyrics.SetSkipStatus(true);
		m_NoteTracker.Seek(0);
		m_IsSkipping = true;
	}
	// スキップ終了通知
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		// NoteTracker リスナーがカウントを管理するため SetNotesCount は不要
		m_NoteBox.SetSkipStatus(false);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(false);
		m_Ripple.SetSkipStatus(false);
		m_Lyrics.SetSkipStatus(false);
		m_IsSkipping = false;
		m_NoteTracker.Seek(m_PlayTimeMSec);
	}

	return result;
}

//******************************************************************************
// 表示トグル
//******************************************************************************
void MTScenePianoRoll3D11::SetEffect(MTEffectType type, bool isEnable)
{
	switch (type) {
	case MTEffectPianoKeyboard:
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetEnable(isEnable);
		break;
	case MTEffectRipple:
		m_Ripple.SetEnable(isEnable);
		break;
	case MTEffectLyrics:
		m_Lyrics.SetEnable(isEnable);
		break;
	case MTEffectStars:
		m_Stars.SetEnable(isEnable);
		break;
	case MTEffectGridBox:
		if (m_pGrid) m_pGrid->SetEnable(isEnable);
		break;
	case MTEffectTimeIndicator:
		m_TimeIndicator.SetEnable(isEnable);
		break;
	case MTEffectBackgroundImage:
		m_BackgroundImage.SetEnable(isEnable);
		break;
	case MTEffectCounter:
		m_Dashboard.SetEnable(isEnable);
		break;
	case MTEffectFileName:
		m_Dashboard.SetEnableFileName(isEnable);
		break;
	case MTEffectPitchBend:
		m_NotePitchBend.SetEnable(isEnable);
		break;
	default:
		break;
	}
}

//******************************************************************************
// 再生速度設定
//******************************************************************************
void MTScenePianoRoll3D11::SetPlaySpeedRatio(unsigned long ratio)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

//******************************************************************************
// ノート数取得
//******************************************************************************
unsigned long MTScenePianoRoll3D11::GetNoteCount() const
{
	return m_NoteBox.GetNoteCount();
}

//******************************************************************************
// Live note events
//******************************************************************************
void MTScenePianoRoll3D11::SetNoteOnLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo, unsigned char velocity)
{
	if (!m_isMonitoringActive) return;
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOn(portNo, chNo, noteNo, velocity);
		m_NoteTrackerLive.SetNoteOn(portNo, chNo, noteNo, velocity);
	}
}

void MTScenePianoRoll3D11::SetNoteOffLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOff(portNo, chNo, noteNo);
		m_NoteTrackerLive.SetNoteOff(portNo, chNo, noteNo);
	}
}

void MTScenePianoRoll3D11::AllNoteOffLive()
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
	}
}

void MTScenePianoRoll3D11::AllNoteOffOnChLive(
		unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOffOnCh(portNo, chNo);
		m_NoteTrackerLive.AllNoteOffOnCh(portNo, chNo);
	}
}

//******************************************************************************
// デフォルト視点計算
//******************************************************************************
void MTScenePianoRoll3D11::_ComputeDefaultViewParam(MTViewParamMap* pParamMap)
{
	// E4 (noteNo=64) の中心、Z=-18 後方
	// WorldMoveVector 適用後の座標系に合わせる
	float noteStep = 0.1f;
	float defaultY = noteStep * 64.0f;
	float defaultZ = -18.0f;

	(*pParamMap)["X"] = 0.0f;
	(*pParamMap)["Y"] = defaultY;
	(*pParamMap)["Z"] = defaultZ;
	(*pParamMap)["Phi"] = 90.0f;
	(*pParamMap)["Theta"] = 90.0f;
	(*pParamMap)["ManualRollAngle"] = 0.0f;
	(*pParamMap)["AutoRollVelocity"] = 0.0f;
}

//******************************************************************************
// 視点補正量
//******************************************************************************
float MTScenePianoRoll3D11::_GetViewpointCompensation() const
{
	return m_TimeIndicator.GetPos();
}

//******************************************************************************
// リセット
//******************************************************************************
void MTScenePianoRoll3D11::_Reset()
{
	MTSceneBase11::_Reset();
}
