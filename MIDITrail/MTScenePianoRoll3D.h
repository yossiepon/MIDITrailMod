//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D
//
// ピアノロール3Dシーン描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTFirstPersonCam.h"
#include "MTNoteBox.h"
#include "MTNoteRipple.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "MTGridBox.h"
#include "MTPictBoard.h"
#include "MTDashboard.h"
#include "MTStars.h"
#include "MTPianoKeyboardCtrl.h"
#include "MTTimeIndicator.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// ピアノロール3Dシーン描画クラス
//******************************************************************************
class MTScenePianoRoll3D : public MTScene
{
public:

	//コンストラクタ／デストラクタl
	MTScenePianoRoll3D();
	~MTScenePianoRoll3D();

	//名称取得
	const TCHAR* GetName();

	//生成
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

	//変換
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//破棄
	void Release();

	//ウィンドウクリックイベント受信
	int OnWindowClicked(
			unsigned long button,
			unsigned long wParam,
			unsigned long lParam
		);

	//演奏開始イベント受信
	int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);

	//演奏終了イベント受信
	int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);

	//シーケンサメッセージ受信
	int OnRecvSequencerMsg(
			unsigned long wParam,
			unsigned long lParam
		);

	//巻き戻し
	int Rewind();

	//視点取得／登録
	void GetDefaultViewParam(MTViewParamMap* pParamMap);
	void GetViewParam(MTViewParamMap* pParamMap);
	void SetViewParam(MTViewParamMap* pParamMap);

	//視点リセット
	void ResetViewpoint();

	//エフェクト設定
	void SetEffect(MTScene::EffectType type, bool isEnable);

	//演奏速度設定
	void SetPlaySpeedRatio(unsigned long ratio);

protected:

	//ライト有無
	BOOL m_IsEnableLight;

private:

	//ライト
	DXDirLight m_DirLight;

	//一人称カメラ
	MTFirstPersonCam m_FirstPersonCam;

	//描画オブジェクト
	MTNoteBox m_NoteBox;
	MTNoteRipple m_NoteRipple;
	MTNotePitchBend m_NotePitchBend;
	MTGridBox m_GridBox;
	MTPictBoard m_PictBoard;
	MTDashboard m_Dashboard;
	MTStars m_Stars;
	MTTimeIndicator m_TimeIndicator;

	MTPianoKeyboardCtrl m_PianoKeyboardCtrl;

	//マウス視線移動モード
	bool m_IsMouseCamMode;

	//自動回転モード
	bool m_IsAutoRollMode;

	//視点情報
	MTViewParamMap m_ViewParamMap;

	//ノートデザインオブジェクト
	MTNoteDesign m_NoteDesign;

	//スキップ状態
	bool m_IsSkipping;

	void _Reset();
	void _SetLightColor(DXDirLight* pLight);

};

