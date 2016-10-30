//******************************************************************************
//
// MIDITrail / DXDirLight
//
// ディレクショナルライトクラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// ディレクショナルライトクラス
//******************************************************************************
class DXDirLight
{
public:

	//コンストラクタ／デストラクタ
	DXDirLight(void);
	virtual ~DXDirLight(void);

	//初期化
	int Initialize();

	//ライト色設定
	void SetColor(D3DXCOLOR diffuse, D3DXCOLOR specular, D3DXCOLOR ambient);

	//ライト方向登録
	void SetDirection(D3DXVECTOR3 dirVector);

	//ライト方向取得
	D3DXVECTOR3 GetDirection();

	//デバイスへのライト登録
	int SetDevice(
			LPDIRECT3DDEVICE9 pD3DDevice,
			BOOL isLightON
		);

// >>> add 20121229 yossiepon begin
	//デバイスへのライト登録
	int SetDevice(
			LPDIRECT3DDEVICE9 pD3DDevice,
			DWORD index,
			BOOL isLightON
		);
// <<< add 20121229 yossiepon end

private:

	D3DLIGHT9 m_Light;

};


