//******************************************************************************
//
// MIDITrail / MTStars
//
// 星描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 星をランダムに配置して描画する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "DXPrimitive.h"


//******************************************************************************
// 星描画クラス
//******************************************************************************
class MTStars
{
public:

	//コンストラクタ／デストラクタ
	MTStars(void);
	virtual ~MTStars(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, DXDirLight* pLight);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//破棄
	void Release();

	//表示設定
	void SetEnable(bool isEnable);

private:

	int m_NumOfStars;
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;

	//表示可否
	bool m_isEnable;

	//頂点バッファ構造体
	struct MTSTARS_VERTEX {
		D3DXVECTOR3 p;		//頂点座標
		D3DXVECTOR3 n;		//法線
		DWORD		c;		//ディフューズ色
	};

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateVertexOfStars(MTSTARS_VERTEX* pVertex, DXDirLight* pLight);
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	int _LoadConfFile(const TCHAR* pSceneName);

};


