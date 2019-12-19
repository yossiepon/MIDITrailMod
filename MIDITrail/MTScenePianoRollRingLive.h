//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingLive
//
// ライブモニタ用ピアノロールリングシーン描画クラス
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
#include "MTNoteBoxRingLive.h"
#include "MTNoteRippleRing.h"
#include "MTNoteDesignRing.h"
#include "MTNotePitchBend.h"
#include "MTGridRingLive.h"
#include "MTPictBoardRing.h"
#include "MTDashboardLive.h"
#include "MTStars.h"
#include "MTTimeIndicatorRing.h"
#include "MTMeshCtrl.h"
#include "MTBackgroundImage.h"
#include "MTConfFile.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// ライブモニタ用ピアノロールリングシーン描画クラス
//******************************************************************************
class MTScenePianoRollRingLive : public MTScene
{
public:
	
	//コンストラクタ／デストラクタl
	MTScenePianoRollRingLive();
	~MTScenePianoRollRingLive();
	
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
			UINT button,
			WPARAM wParam,
			LPARAM lParam
		);
	
	//演奏開始イベント受信
	int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//演奏終了イベント受信
	int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//シーケンサメッセージ受信
	int OnRecvSequencerMsg(
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
	void SetEffect(MTScene::EffectType type, bool isEnable);
	
protected:
	
	//ライト有無
	bool m_IsEnableLight;
	
private:
	
	//ライト
	DXDirLight m_DirLight;
	
	//一人称カメラ
	MTFirstPersonCam m_FirstPersonCam;
	
	//描画オブジェクト
	MTNoteBoxRingLive m_NoteBoxLive;
	MTNoteRippleRing m_NoteRipple;
	MTNotePitchBend m_NotePitchBend;
	MTGridRingLive m_GridBoxLive;
	MTPictBoardRing m_PictBoard;
	MTDashboardLive m_DashboardLive;
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
	
	void _Reset();
	int _LoadConf();
	int _LoadConfViewpoint(MTConfFile* pConfFile, unsigned long viewpointNo, MTScene::MTViewParamMap* pParamMap);

};


