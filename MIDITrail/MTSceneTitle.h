//******************************************************************************
//
// MIDITrail / MTSceneTitle
//
// タイトルシーン描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXCamera.h"
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTLogo.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//カメラZ座標
#define MTSCENETITLE_CAMERA_POSZ  (-80.0f)

//カメラZ座標変化量
#define MTSCENETITLE_CAMERA_POSZ_DELTA  (0.05f)


//******************************************************************************
// タイトルシーン描画クラス
//******************************************************************************
class MTSceneTitle : public MTScene
{
public:

	//コンストラクタ／デストラクタl
	MTSceneTitle(void);
	virtual ~MTSceneTitle(void);

	//名称取得
	const TCHAR* GetName();

	//生成
	int Create(
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

private:

	//カメラ位置Z
	float m_CamPosZ;

	//カメラ
	DXCamera m_Camera;

	//ライト
	DXDirLight m_DirLight;

	//ロゴ描画オブジェクト
	MTLogo m_Logo;

};

