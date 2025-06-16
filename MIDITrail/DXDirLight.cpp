//******************************************************************************
//
// MIDITrail / DXDirLight
//
// ディレクショナルライトクラス
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXDirLight.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
DXDirLight::DXDirLight(void)
{
	ZeroMemory(&m_Light, sizeof(D3DLIGHT9));
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DXDirLight::~DXDirLight(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int DXDirLight::Initialize()
{
	int result = 0;

	ZeroMemory(&m_Light, sizeof(D3DLIGHT9));

	//ライトタイプ：平行光線
	m_Light.Type = D3DLIGHT_DIRECTIONAL;

	//拡散光
	m_Light.Diffuse.r  = 1.0f;
	m_Light.Diffuse.g  = 1.0f;
	m_Light.Diffuse.b  = 1.0f;
	m_Light.Diffuse.a  = 1.0f;

	//鏡面反射光
	m_Light.Specular.r = 0.0f;
	m_Light.Specular.g = 0.0f;
	m_Light.Specular.b = 0.0f;
	m_Light.Specular.a = 0.0f;

	//環境光
	m_Light.Ambient.r  = 0.2f;
	m_Light.Ambient.g  = 0.2f;
	m_Light.Ambient.b  = 0.2f;
	m_Light.Ambient.a  = 1.0f;

	//方向：ベクトルは正規化されていなければならない
	m_Light.Direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	return result;
}

//******************************************************************************
// ライト色設定
//******************************************************************************
void DXDirLight::SetColor(
		D3DXCOLOR diffuse,
		D3DXCOLOR specular,
		D3DXCOLOR ambient
	)
{
	m_Light.Diffuse  = diffuse;
	m_Light.Specular = specular;
	m_Light.Ambient  = ambient;
}

//******************************************************************************
// ライト方向設定
//******************************************************************************
void DXDirLight::SetDirection(
		D3DXVECTOR3 dirVector
	)
{
	D3DXVECTOR3 normalizedVector;

	//ベクトル正規化
	D3DXVec3Normalize(&normalizedVector, &dirVector);

	//ライト情報構造体に登録
	m_Light.Direction = normalizedVector;
}

//******************************************************************************
// ライト方向取得
//******************************************************************************
D3DXVECTOR3 DXDirLight::GetDirection()
{
	return m_Light.Direction;
}

//******************************************************************************
// デバイス登録：インデックス0
//******************************************************************************
int DXDirLight::SetDevice(
		LPDIRECT3DDEVICE9 pD3DDevice,
		BOOL isLightON
	)
{
	return SetDevice(pD3DDevice, 0, isLightON);
}

//******************************************************************************
// デバイス登録：インデックス指定
//******************************************************************************
int DXDirLight::SetDevice(
		LPDIRECT3DDEVICE9 pD3DDevice,
		DWORD index,
		BOOL isLightON
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	//ライティングモード
	hresult = pD3DDevice->SetRenderState(D3DRS_LIGHTING, isLightON);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, isLightON);
		goto EXIT;
	}

	//スペキュラ光
	//  スペキュラを有効にすると通常のライトに比べて2倍の負荷が生じるため無効にする
	//  TODO: 外部から設定できるようにする
	hresult = pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, isLightON);
		goto EXIT;
	}

	// ライトをレンダリングパイプラインに設定
	hresult = pD3DDevice->SetLight(index, &m_Light);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//ライト有効化
	hresult = pD3DDevice->LightEnable(index, isLightON);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, isLightON);
		goto EXIT;
	}

EXIT:;
	return result;
}

