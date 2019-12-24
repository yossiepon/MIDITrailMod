//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingMod
//
// ピアノロールリングシーン描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include "Commdlg.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTScenePianoRollRingMod.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRollRingMod::MTScenePianoRollRingMod()
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRollRingMod::~MTScenePianoRollRingMod()
{
	Release();
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTScenePianoRollRingMod::GetName()
{
	return _T("PianoRollRing");
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTScenePianoRollRingMod::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	//基底クラスのシーン生成処理を呼び出す
	result = MTScenePianoRollRing::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;

	//----------------------------------
	// 描画オブジェクト
	//----------------------------------
	//ノート波紋生成
	result = m_NoteRippleMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//グリッドリング生成
	result = m_GridRingMod.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	//タイムインジケータ生成
	result = m_TimeIndicatorMod.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 変換処理
//******************************************************************************
int MTScenePianoRollRingMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;

	//基底クラスの変換処理を呼び出す
	result = MTScenePianoRollRing::Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//カメラ座標取得
	m_FirstPersonCam.GetPosition(&camVector);

	//回転角度取得
	rollAngle = m_FirstPersonCam.GetManualRollAngle();

	//グリッドリング更新
	result = m_GridRingMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//タイムインジケータ更新
	result = m_TimeIndicatorMod.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

	//ノート波紋更新
	result = m_NoteRippleMod.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTScenePianoRollRingMod::Draw(
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

	//グリッドリング描画
	result = m_GridRingMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//ノートボックス描画
	result = m_NoteBox.Draw(pD3DDevice);
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
	result = m_TimeIndicatorMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//ノート波紋描画
	result = m_NoteRippleMod.Draw(pD3DDevice);
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
void MTScenePianoRollRingMod::Release()
{
	m_GridRingMod.Release();
	m_TimeIndicatorMod.Release();
	m_NoteRippleMod.Release();

	MTScenePianoRollRing::Release();
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MTScenePianoRollRingMod::OnRecvSequencerMsg(
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
		m_TimeIndicatorMod.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteRippleMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteRippleMod.SetCurTickTime(parser.GetPlayTickTime());
		m_PictBoard.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteBox.SetCurTickTime(parser.GetPlayTickTime());
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
		//m_NoteRippleMod.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
	}
	//ノートON通知
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		m_Dashboard.SetNoteOn();
		//m_NoteRippleMod.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
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
		m_NoteBox.Reset();
		m_NoteBox.SetSkipStatus(true);
		m_NoteRippleMod.Reset();
		m_NoteRippleMod.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	//スキップ終了通知
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_Dashboard.SetNotesCount(parser.GetSkipEndNotesCount());
		m_NoteBox.SetSkipStatus(false);
		m_NoteRippleMod.SetSkipStatus(false);
		m_IsSkipping = false;
	}

	//EXIT:;
	return result;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTScenePianoRollRingMod::_Reset()
{
	MTScenePianoRollRing::_Reset();

	m_TimeIndicatorMod.Reset();
	m_NoteRippleMod.Reset();
}

//******************************************************************************
// 表示効果設定
//******************************************************************************
void MTScenePianoRollRingMod::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	switch (type) {
		case EffectRipple:
			m_NoteRippleMod.SetEnable(isEnable);
			break;
		case EffectTimeIndicator:
			m_TimeIndicatorMod.SetEnable(isEnable);
			break;
		case EffectGridBox:
			m_GridRingMod.SetEnable(isEnable);
			break;
		default:
			MTScenePianoRollRing::SetEffect(type, isEnable);
			break;
	}

	return;
}
