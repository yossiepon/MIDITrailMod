//******************************************************************************
//
// MIDITrail / DXCamera
//
// カメラクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXCamera.h"
#include "YNBaseLib.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
DXCamera::DXCamera(void)
{
	_Clear();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DXCamera::~DXCamera(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int DXCamera::Initialize()
{
	_Clear();
	return 0;
}

//******************************************************************************
// 基本パラメータ設定
//******************************************************************************
void DXCamera::SetBaseParam(
		float viewAngle,
		float nearPlane,
		float farPlane
	)
{
	m_ViewAngle = viewAngle;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
}

//******************************************************************************
// カメラ位置設定
//******************************************************************************
void DXCamera::SetPosition(
		D3DXVECTOR3 camVector,
		D3DXVECTOR3 camLookAtVector,
		D3DXVECTOR3 camUpVector
	)
{
	m_CamVector = camVector;
	m_CamLookAtVector = camLookAtVector;
	m_CamUpVector = camUpVector;
}

//******************************************************************************
// 変換
//******************************************************************************
int DXCamera::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DXMATRIX viewMatrix;
	D3DXMATRIX projMatrix;

	//射影行列を取得
	result = _GetProjMatrix(pD3DDevice, &projMatrix);
	if (result != 0) goto EXIT;

	//射影行列をレンダリングパイプラインに設定
	hresult = pD3DDevice->SetTransform(D3DTS_PROJECTION, &projMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//ビューイング行列を取得
	result = _GetViewMatrix(&viewMatrix);
	if (result != 0) goto EXIT;

	//ビューイング行列をレンダリングパイプラインに設定
	hresult = pD3DDevice->SetTransform(D3DTS_VIEW, &viewMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// クリア
//******************************************************************************
void DXCamera::_Clear()
{
	m_ViewAngle = 45.0f;
	m_NearPlane = 1.0f;
	m_FarPlane = 1000.0f;
	m_CamVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_CamLookAtVector = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_CamUpVector = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
}

//******************************************************************************
// 射影列取得
//******************************************************************************
int DXCamera::_GetProjMatrix(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXMATRIX* pViewMatrix
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DVIEWPORT9 viewPort;
	float aspect = 0.0f;

	//行列初期化
	D3DXMatrixIdentity(pViewMatrix);

	//ビューポート取得
	hresult = pD3DDevice->GetViewport(&viewPort);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//アスペクト比
	aspect = (float)viewPort.Width / (float)viewPort.Height;
	
	//左手系射影マトリックス作成
	D3DXMatrixPerspectiveFovLH(
			pViewMatrix,				//生成された行列
			D3DXToRadian(m_ViewAngle),	//カメラの画角
			aspect,						//アスペクト比
			m_NearPlane,				//nearプレーン
			m_FarPlane					//farプレーン
		);

EXIT:;
	return result;
}

//******************************************************************************
// ビュー変換行列取得
//******************************************************************************
int DXCamera::_GetViewMatrix(
		D3DXMATRIX* pViewMatrix
	)
{
	int result = 0;

	//ビュー変換行列生成
	D3DXMatrixLookAtLH(
			pViewMatrix,		//作成された行列
			&m_CamVector,		//カメラ位置
			&m_CamLookAtVector,	//注目点
			&m_CamUpVector		//カメラの上方向
		);

	return result;
}

