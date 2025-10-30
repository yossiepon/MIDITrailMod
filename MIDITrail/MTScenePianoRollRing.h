//******************************************************************************
//
// MIDITrail / MTScenePianoRollRing
//
// ピアノロールリングシーン描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTFirstPersonCam.h"
#include "MTNoteBoxRing.h"
#include "MTNoteRippleRing.h"
#include "MTNoteDesignRing.h"
#include "MTNotePitchBend.h"
#include "MTGridRing.h"
#include "MTPictBoardRing.h"
#include "MTDashboard.h"
#include "MTStars.h"
#include "MTTimeIndicatorRing.h"
#include "MTMeshCtrl.h"
#include "MTBackgroundImage.h"
#include "MTConfFile.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// ピアノロールリングシーン描画クラス
//******************************************************************************
class MTScenePianoRollRing : public MTScene
{
public:

	//コンストラクタ／デストラクタl
	MTScenePianoRollRing();
	// >>> modify 20191222 yossiepon begin
	virtual ~MTScenePianoRollRing();
	// <<< modify 20191222 yossiepon end

	//名称取得
	const TCHAR* GetName();

	//生成
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

	//変換
	// >>> modify 20191222 yossiepon begin
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);
	// <<< modify 20191222 yossiepon end

	//描画
	// >>> modify 20191222 yossiepon begin
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	// <<< modify 20191222 yossiepon end

	//破棄
	// >>> modify 20191222 yossiepon begin
	virtual void Release();
	// <<< modify 20191222 yossiepon end

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
	// >>> modify 20191222 yossiepon begin
	virtual int OnRecvSequencerMsg(
	// <<< modify 20191222 yossiepon end
			unsigned long param1,
			unsigned long param2
		);

	//巻き戻し
	int Rewind();

	//視点取得／登録
	void GetDefaultViewParam(MTViewParamMap* pParamMap);
	void GetViewParam(MTViewParamMap* pParamMap);
	void SetViewParam(MTViewParamMap* pParamMap);
	void MoveToStaticViewpoint(unsigned long viewpointNo);

	//視点リセット
	void ResetViewpoint();

	//エフェクト設定
	// >>> modify 20191222 yossiepon begin
	virtual void SetEffect(MTScene::EffectType type, bool isEnable);
	// <<< modify 20191222 yossiepon end

	//演奏速度設定
	void SetPlaySpeedRatio(unsigned long ratio);

protected:

	//ライト有無
	BOOL m_IsEnableLight;

// >>> modify access level to protected 20191222 yossiepon begin
//private:
// <<< modify 20191222 yossiepon end

	//ライト
	DXDirLight m_DirLight;

	//一人称カメラ
	MTFirstPersonCam m_FirstPersonCam;

	//描画オブジェクト
	MTNoteBoxRing m_NoteBox;
	MTNoteRippleRing m_NoteRipple;
	MTNotePitchBend m_NotePitchBend;
	MTGridRing m_GridRing;
	MTPictBoardRing m_PictBoard;
	MTDashboard m_Dashboard;
	MTStars m_Stars;
	MTTimeIndicatorRing m_TimeIndicator;
	MTMeshCtrl m_MeshCtrl;
	MTBackgroundImage m_BackgroundImage;

	//マウス視線移動モード
	bool m_IsMouseCamMode;

	//自動回転モード
	bool m_IsAutoRollMode;

	//視点情報
	MTViewParamMap m_ViewParamMap;
	MTViewParamMap m_Viewpoint2;
	MTViewParamMap m_Viewpoint3;

	//ノートデザインオブジェクト
	MTNoteDesignRing m_NoteDesign;

	//スキップ状態
	bool m_IsSkipping;

	//シーンリセット
	// >>> modify 20191222 yossiepon begin
	virtual void _Reset();
	// <<< modify 20191222 yossiepon end

	void _SetLightColor(DXDirLight* pLight);
	int _LoadConf();
	int _LoadConfViewpoint(MTConfFile* pConfFile, unsigned long viewpointNo, MTScene::MTViewParamMap* pParamMap);

};

