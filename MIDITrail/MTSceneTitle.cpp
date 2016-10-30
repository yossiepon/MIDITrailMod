//******************************************************************************
//
// MIDITrail / MTSceneTitle
//
// タイトルシーン描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************


#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTSceneTitle.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTSceneTitle::MTSceneTitle(void)
{
	m_CamPosZ = MTSCENETITLE_CAMERA_POSZ;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTSceneTitle::~MTSceneTitle(void)
{
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTSceneTitle::GetName()
{
	return _T("Title");
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTSceneTitle::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//----------------------------------
	// カメラ
	//----------------------------------
	//カメラ初期化
	result = m_Camera.Initialize();
	if (result != 0) goto EXIT;

	//基本パラメータ設定
	m_Camera.SetBaseParam(
			45.0f,		//画角
			1.0f,		//Nearプレーン
			1000.0f		//Farプレーン
		);

	//カメラ位置設定
	m_Camera.SetPosition(
			D3DXVECTOR3(0.0f, 0.0f, m_CamPosZ),	//カメラ位置
			D3DXVECTOR3(0.0f, 0.0f, 0.0f), 		//注目点
			D3DXVECTOR3(0.0f, 1.0f, 0.0f)		//カメラ上方向
		);
	
	//----------------------------------
	// ライト
	//----------------------------------
	//ライト初期化
	result = m_DirLight.Initialize();
	if (result != 0) goto EXIT;

	//ライト方向
	m_DirLight.SetDirection(D3DXVECTOR3(1.0f, -1.0f, 2.0f));

	//ライトのデバイス登録
//	result = m_DirLight.SetDevice(pD3DDevice, TRUE);  //ライトあり
	result = m_DirLight.SetDevice(pD3DDevice, FALSE); //ライトなし
	if (result != 0) goto EXIT;

	//----------------------------------
	// 描画オブジェクト
	//----------------------------------
	//ロゴ生成
	result = m_Logo.Create(pD3DDevice);
	if (result != 0) goto EXIT;

	//----------------------------------
	// レンダリングステート
	//----------------------------------
	//画面描画モード
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//Z深度比較：ON
	 pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	//ディザリング:ON 高品質描画
	pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);

	//マルチサンプリングアンチエイリアス：有効
	pD3DDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	//レンダリングステート設定：通常のアルファ合成
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

EXIT:;
	return result;
}

//******************************************************************************
// 変換処理
//******************************************************************************
int MTSceneTitle::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//カメラ位置設定
	m_CamPosZ += MTSCENETITLE_CAMERA_POSZ_DELTA;
	m_Camera.SetPosition(
			D3DXVECTOR3(0.0f, 0.0f, m_CamPosZ),	//カメラ位置
			D3DXVECTOR3(0.0f, 0.0f, 0.0f), 		//注目点
			D3DXVECTOR3(0.0f, 1.0f, 0.0f)		//カメラ上方向
		);

	//カメラ更新
	result = m_Camera.Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//ロゴ更新
	result = m_Logo.Transform(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTSceneTitle::Draw(
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

	//ロゴ描画
	result = m_Logo.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 破棄
//******************************************************************************
void MTSceneTitle::Release()
{
	m_Logo.Release();
}

