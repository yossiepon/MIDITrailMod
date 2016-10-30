//******************************************************************************
//
// MIDITrail / DXCamera
//
// カメラクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// カメラクラス
//******************************************************************************
class DXCamera
{
public:

	//コンストラクタ／デストラクタ
	DXCamera(void);
	virtual ~DXCamera(void);

	//初期化
	int Initialize();

	//基本パラメータ設定
	void SetBaseParam(
			float viewAngle,
			float nearPlane,
			float farPlane
		);

	//カメラ位置設定
	void SetPosition(
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 camLookAtVector,
			D3DXVECTOR3 camUpVector
		);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//カメラの画角
	float m_ViewAngle;

	//Nearプレーン：0だとZ軸順制御がおかしくなる
	float m_NearPlane;

	//Farプレーン
	float m_FarPlane;

	//カメラ位置
	D3DXVECTOR3 m_CamVector;

	//注目点
	D3DXVECTOR3 m_CamLookAtVector;

	//カメラの上方向
	D3DXVECTOR3 m_CamUpVector;

	void _Clear();

	int _GetProjMatrix(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXMATRIX* pViewMatrix
		);
	int _GetViewMatrix(
			D3DXMATRIX* pViewMatrix
		);

};

