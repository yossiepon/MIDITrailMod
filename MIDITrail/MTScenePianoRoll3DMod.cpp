//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DMod
//
// ピアノロール3Dシーン描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTScenePianoRoll3DMod.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRoll3DMod::MTScenePianoRoll3DMod()
{
	m_IsSingleKeyboard = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRoll3DMod::~MTScenePianoRoll3DMod()
{
	Release();
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTScenePianoRoll3DMod::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	//基底クラスのシーン生成処理を呼び出す
	result = MTScenePianoRoll3D::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;

	//----------------------------------
	// ライト2
	//----------------------------------
	//ライト2初期化
	result = m_DirLightBack.Initialize();
	if (result != 0) goto EXIT;

	//ライト2色
	_SetLightColor(&m_DirLightBack);

	//ライト2方向
	m_DirLightBack.SetDirection(D3DXVECTOR3(-1.0f, 1.0f, -2.0f));

	//ライトのデバイス登録
	result = m_DirLightBack.SetDevice(pD3DDevice, 1, m_IsEnableLight);
	if (result != 0) goto EXIT;

	//----------------------------------
	// 描画オブジェクト
	//----------------------------------

	//グリッドボックス生成
	result = m_GridBoxMod.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//ノートボックス生成
	result = m_NoteBoxMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//ノート波紋生成
	result = m_NoteRippleMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//ノート歌詞生成
	result = m_NoteLyrics.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//ピクチャボード無効
	m_PictBoard.SetEnable(false);

	//ピアノキーボード制御
	result = m_PianoKeyboardCtrlMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend, m_IsSingleKeyboard);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 変換処理
//******************************************************************************
int MTScenePianoRoll3DMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;

	//基底クラスの変換処理を呼び出す
	result = MTScenePianoRoll3D::Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//カメラ座標取得
	m_FirstPersonCam.GetPosition(&camVector);

	//回転角度取得
	rollAngle = m_FirstPersonCam.GetManualRollAngle();

	//グリッドボックス更新
	result = m_GridBoxMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//ノートボックス更新
	result = m_NoteBoxMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//ノート波紋更新
	result = m_NoteRippleMod.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

	//ノート歌詞更新
	result = m_NoteLyrics.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

	//ピアノキーボード更新
	result = m_PianoKeyboardCtrlMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTScenePianoRoll3DMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	D3DXVECTOR3 camVector;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//更新
	result = Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//カメラ座標取得
	m_FirstPersonCam.GetPosition(&camVector);

	//背景画像描画
	result = m_BackgroundImage.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//星描画
	result = m_Stars.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//グリッドボックス描画
	result = m_GridBoxMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//ノートボックス描画
	result = m_NoteBoxMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	// カメラ位置が演奏位置より手前側であれば
	if(m_TimeIndicator.GetPos() > camVector.x) {

		//メッシュ＞タイムインジケータ＞歌詞＞波紋＞キーボードの順で奥から描画

		//メッシュ描画
		result = m_MeshCtrl.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//タイムインジケータ描画
		result = m_TimeIndicator.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//ノート歌詞描画
		result = m_NoteLyrics.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//ノート波紋描画
		result = m_NoteRippleMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//ピアノキーボード描画
		result = m_PianoKeyboardCtrlMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

	} else {

		//キーボード＞波紋＞歌詞＞タイムインジケータ＞メッシュの順で奥から描画

		//ピアノキーボード描画
		result = m_PianoKeyboardCtrlMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//ノート波紋描画
		result = m_NoteRippleMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//ノート歌詞描画
		result = m_NoteLyrics.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//タイムインジケータ描画
		result = m_TimeIndicator.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//メッシュ描画
		result = m_MeshCtrl.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

	}

	//ダッシュボード描画：座標変換済み頂点を用いるため一番最後に描画する
	result = m_Dashboard.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 破棄
//******************************************************************************
void MTScenePianoRoll3DMod::Release()
{
	m_NoteBoxMod.Release();
	m_NoteRippleMod.Release();
	m_NoteLyrics.Release();
	m_PianoKeyboardCtrlMod.Release();

	MTScenePianoRoll3D::Release();
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MTScenePianoRoll3DMod::OnRecvSequencerMsg(
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
		m_Dashboard.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_FirstPersonCam.SetCurTickTime(parser.GetPlayTickTime());
		m_TimeIndicator.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteRippleMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteRippleMod.SetCurTickTime(parser.GetPlayTickTime());
		m_PictBoard.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteBoxMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteBoxMod.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteLyrics.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteLyrics.SetCurTickTime(parser.GetPlayTickTime());
		m_PianoKeyboardCtrlMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_PianoKeyboardCtrlMod.SetCurTickTime(parser.GetPlayTickTime());
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
		// NOP
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
		m_NoteBoxMod.Reset();
		m_NoteBoxMod.SetSkipStatus(true);
		m_NoteRippleMod.Reset();
		m_NoteRippleMod.SetSkipStatus(true);
		m_NoteLyrics.Reset();
		m_NoteLyrics.SetSkipStatus(true);
		m_PianoKeyboardCtrlMod.Reset();
		m_PianoKeyboardCtrlMod.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	//スキップ終了通知
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_Dashboard.SetNotesCount(parser.GetSkipEndNotesCount());
		m_NoteBoxMod.SetSkipStatus(false);
		m_NoteRippleMod.SetSkipStatus(false);
		m_NoteLyrics.SetSkipStatus(false);
		m_PianoKeyboardCtrlMod.SetSkipStatus(false);
		m_IsSkipping = false;
	}

//EXIT:;
	return result;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTScenePianoRoll3DMod::_Reset()
{
	MTScenePianoRoll3D::_Reset();

	m_NoteBoxMod.Reset();
	m_NoteRippleMod.Reset();
	m_NoteLyrics.Reset();
	m_PianoKeyboardCtrlMod.Reset();
}

//******************************************************************************
// 表示効果設定
//******************************************************************************
void MTScenePianoRoll3DMod::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	switch (type) {
		case EffectPianoKeyboard:
			m_PianoKeyboardCtrlMod.SetEnable(isEnable);
			break;
		case EffectRipple:
			m_NoteRippleMod.SetEnable(isEnable);
			m_NoteLyrics.SetEnable(isEnable);
			break;
		case EffectPitchBend:
			m_NotePitchBend.SetEnable(isEnable);
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
