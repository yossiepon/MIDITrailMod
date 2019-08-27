//******************************************************************************
//
// MIDITrail / MTScenePianoRollRainLive
//
// ライブモニタ用ピアノロールレインシーン描画クラス
//
// Copyright (C) 2012-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTFirstPersonCam.h"
#include "MTStars.h"
#include "MTPianoKeyboardCtrlLive.h"
#include "MTNoteRainLive.h"
#include "MTDashboardLive.h"
#include "MTNotePitchBend.h"
#include "MTMeshCtrl.h"
#include "MTBackgroundImage.h"
#include "MTConfFile.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// ピアノロールレインシーン描画クラス
//******************************************************************************
class MTScenePianoRollRainLive : public MTScene
{
public:
	
	//コンストラクタ／デストラクタ
	MTScenePianoRollRainLive(void);
	virtual ~MTScenePianoRollRainLive(void);
	
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
	virtual void GetDefaultViewParam(MTViewParamMap* pParamMap);
	void GetViewParam(MTViewParamMap* pParamMap);
	void SetViewParam(MTViewParamMap* pParamMap);
	
	//視点リセット
	void ResetViewpoint();
	void MoveToStaticViewpoint(unsigned long viewpointNo);
	
	//エフェクト設定
	void SetEffect(MTScene::EffectType type, bool isEnable);
	
protected:
	
	//ライト有無
	bool m_IsEnableLight;
	
	//シングルキーボードフラグ
	bool m_IsSingleKeyboard;
	
private:
	
	//ライト
	DXDirLight m_DirLight;
	
	//一人称カメラ
	MTFirstPersonCam m_FirstPersonCam;
	
	//描画オブジェクト
	MTStars m_Stars;
	MTPianoKeyboardCtrlLive m_PianoKeyboardCtrlLive;
	MTNoteRainLive m_NoteRainLive;
	MTNotePitchBend m_NotePitchBend;
	MTDashboardLive m_DashboardLive;
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
	
	void _Reset();
	int _LoadConf();
	int _LoadConfViewpoint(MTConfFile* pConfFile, unsigned long viewpointNo, MTScene::MTViewParamMap* pParamMap);
	
};

