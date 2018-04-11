//******************************************************************************
//
// MIDITrail / MTMeshCtrl
//
// メッシュ制御クラス
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXMesh.h"


//******************************************************************************
// メッシュ制御クラス
//******************************************************************************
class MTMeshCtrl
{
public:

	//コンストラクタ／デストラクタ
	MTMeshCtrl(void);
	virtual ~MTMeshCtrl(void);

	//生成
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName
		);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 moveVector);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

private:

	DXMesh m_Mesh;
	TCHAR m_MeshFilePath[_MAX_PATH];
	float m_PositionX;
	float m_PositionY;
	float m_PositionZ;

	int _LoadConfFile(const TCHAR* pSceneName);

};


