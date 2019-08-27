//******************************************************************************
//
// MIDITrail / DXRenderer
//
// レンダラクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXScene.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//デバイスロスト
#define DXRENDERER_ERR_DEVICE_LOST  (100)

#define DX_MULTI_SAMPLE_TYPE_MIN    (2)
#define DX_MULTI_SAMPLE_TYPE_MAX    (16)


//******************************************************************************
// レンダラクラス
//******************************************************************************
class DXRenderer
{
public:

	//コンストラクタ／デストラクタ
	DXRenderer();
	virtual ~DXRenderer();

public:

	//初期化
	int Initialize(HWND hWnd, unsigned long multiSampleType = 0, bool isFullScreen = false);

	//デバイス取得
	LPDIRECT3DDEVICE9 GetDevice();

	//描画
	int RenderScene(DXScene* pScene);

	//終了処理
	void Terminate();

	//アンチエイリアスサポート確認
	int IsSupportAntialias(unsigned long multiSampleNum, bool* pIsSupport);

	//インデックスバッファサポート確認
	int IsSupportIndexBuffer(bool* pIsSupport, unsigned long* pMaxVertexIndex);

private:

	HWND m_hWnd;
	LPDIRECT3D9 m_pD3D;
	LPDIRECT3DDEVICE9 m_pD3DDevice;
	D3DPRESENT_PARAMETERS m_D3DPP;

	int _RecoverDevice();
	int _CheckAntialiasSupport(
			D3DPRESENT_PARAMETERS d3dpp,
			D3DMULTISAMPLE_TYPE multiSampleType,
			bool* pIsSupport,
			unsigned long* pQualityLevels
		);
	D3DMULTISAMPLE_TYPE _EnumMultiSampleType(unsigned long multiSampleType);

};


