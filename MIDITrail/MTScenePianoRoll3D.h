//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D
//
// ピアノロール3Dシーン描画クラス
//
// Copyright (C) 2010-2016 WADA Masashi. All Rights Reserved.
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
#include "MTTimeIndicator.h"
#include "MTMeshCtrl.h"
#include "MTBackgroundImage.h"
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
// >>> modify 20120728 yossiepon begin
	virtual ~MTScenePianoRoll3D();
// <<< modify 20120728 yossiepon end

	//名称取得
	const TCHAR* GetName();

	//生成
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

	//変換
// >>> modify 20120728 yossiepon begin
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify 20120728 yossiepon end

	//描画
// >>> modify 20120728 yossiepon begin
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify 20120728 yossiepon end

	//破棄
// >>> modify 20120728 yossiepon begin
	virtual void Release();
// <<< modify 20120728 yossiepon end

	//ウィンドウクリックイベント受信
	int OnWindowClicked(
			UINT button,
			WPARAM wParam,
			LPARAM lParam
		);

	//演奏開始イベント受信
	int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);

	//演奏終了イベント受信
	int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);

	//シーケンサメッセージ受信
// >>> modify 20120728 yossiepon begin
	virtual int OnRecvSequencerMsg(
// <<< modify 20120728 yossiepon end
			unsigned long param1,
			unsigned long param2
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
// >>> modify 20120728 yossiepon begin
	virtual void SetEffect(MTScene::EffectType type, bool isEnable);
// <<< modify 20120728 yossiepon end

	//演奏速度設定
	void SetPlaySpeedRatio(unsigned long ratio);

protected:

	//ライト有無
	BOOL m_IsEnableLight;

private:

	//ライト
	DXDirLight m_DirLight;

// >>> modify access level to protected 20161223 yossiepon begin
protected:
// <<< modify 20161223 yossiepon end

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
	MTMeshCtrl m_MeshCtrl;
	MTBackgroundImage m_BackgroundImage;

// >>> modify access level 20161223 yossiepon begin
private:
// <<< modify access level 20161223 yossiepon end

	//マウス視線移動モード
	bool m_IsMouseCamMode;

	//自動回転モード
	bool m_IsAutoRollMode;

	//視点情報
	MTViewParamMap m_ViewParamMap;

	//ノートデザインオブジェクト
	MTNoteDesign m_NoteDesign;

// >>> modify access level to protected 20161223 yossiepon begin
protected:
// <<< modify 20161223 yossiepon end

	//スキップ状態
	bool m_IsSkipping;

// >>> modify 20120728 yossiepon begin
	virtual void _Reset();
// <<< modify 20120728 yossiepon end
	void _SetLightColor(DXDirLight* pLight);

// >>> modify access level 20161223 yossiepon begin
private:
// <<< modify access level 20161223 yossiepon end

	int _LoadConf();

};

