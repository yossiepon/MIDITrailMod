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

	// データ依存コンポーネント: pSeqData が NULL の場合はスキップ
	if (pSeqData == NULL) goto EXIT;

	// グリッド
	result = m_Grid.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// タイムインジケータ
	result = m_TimeIndicator.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// ピクチャボード
	result = m_PictBoard.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// ダッシュボード
	result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, hWnd);
	if (result != 0) goto EXIT;

	// ピッチベンド
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;

	// ノートトラッカー
	result = m_NoteTracker.Create(pSeqData);
	if (result != 0) goto EXIT;

	// 波紋
	result = m_Ripple.Create(pDevice, pContext, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;
	m_NoteTracker.AddListener(&m_Ripple, NoteEventType::Note);

	// 歌詞
	result = m_Lyrics.Create(pDevice, pContext, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;
	m_NoteTracker.AddListener(&m_Lyrics, NoteEventType::Lyric);
	m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

	// ノートボックス
	result = m_NoteBox.Create(pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend);
	if (result != 0) goto EXIT;
	if (m_Is2D) {
		m_NoteBox.SetLightEnable(false);
	}

	// キーボード
	result = m_KeyboardCtrl.Create(pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend, false);
	if (result != 0) goto EXIT;

	// コンポーネント登録（Update/Reset 自動ディスパッチ）
	_RegisterComponent(&m_Stars);
	_RegisterComponent(&m_BackgroundImage);
	_RegisterComponent(&m_Grid);
	_RegisterComponent(&m_TimeIndicator);
	_RegisterComponent(&m_PictBoard);
	_RegisterComponent(&m_Dashboard);
	_RegisterComponent(&m_NotePitchBend);
	_RegisterComponent(&m_NoteTracker);
	_RegisterComponent(&m_Ripple);
	_RegisterComponent(&m_Lyrics);
	_RegisterComponent(&m_NoteBox);
	_RegisterComponent(&m_KeyboardCtrl);

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
	m_KeyboardCtrl.Release();
	m_NoteBox.Release();
	m_Ripple.Release();
	m_Lyrics.Release();
	m_NoteTracker.Release();
	m_Stars.Release();
	m_Grid.Release();
	m_TimeIndicator.Release();
	m_PictBoard.Release();
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

	// グリッド
	result = m_Grid.Draw(pContext, viewProj, lightDir, rollAngle);
	if (result != 0) goto EXIT;

	// ノートボックス
	result = m_NoteBox.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	// カメラ位置と再生位置の前後関係で描画順を切り替え（奥から手前へ）
	// PictBoard は Keyboard 実装後に置き換え（現状は無効化）
	if (m_TimeIndicator.GetPos() > camPos.x) {
		// カメラが再生位置より手前: Indicator → Lyrics → Ripple → Keyboard
		result = m_TimeIndicator.Draw(pContext, viewProj, lightDir, rollAngle);
		if (result != 0) goto EXIT;
		result = m_Lyrics.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_KeyboardCtrl.Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}
	else {
		// カメラが再生位置より奥: Keyboard → Ripple → Lyrics → Indicator
		result = m_KeyboardCtrl.Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
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
	return 0;
}

//******************************************************************************
// 再生終了
//******************************************************************************
int MTScenePianoRoll3D11::OnPlayEnd()
{
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
		m_KeyboardCtrl.Reset();
		m_KeyboardCtrl.SetSkipStatus(true);
		m_Ripple.SetSkipStatus(true);
		m_Lyrics.SetSkipStatus(true);
		m_NoteTracker.Seek(0);
		m_IsSkipping = true;
	}
	// スキップ終了通知
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		// NoteTracker リスナーがカウントを管理するため SetNotesCount は不要
		m_NoteBox.SetSkipStatus(false);
		m_KeyboardCtrl.SetSkipStatus(false);
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
		m_KeyboardCtrl.SetEnable(isEnable);
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
		m_Grid.SetEnable(isEnable);
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
