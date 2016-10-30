//******************************************************************************
//
// MIDITrail / MTGridBoxLive
//
// ライブモニタ用グリッドボックス描画クラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// グリッドボックスを描画する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXPrimitive.h"
#include "MTNoteDesign.h"


//******************************************************************************
// ライブモニタ用グリッドボックス描画クラス
//******************************************************************************
class MTGridBoxLive
{
public:
	
	//コンストラクタ／デストラクタ
	MTGridBoxLive(void);
	virtual ~MTGridBoxLive(void);
	
	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	
	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
	
	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//解放
	void Release();
	
private:
	
	DXPrimitive m_Primitive;
	MTNoteDesign m_NoteDesign;
	bool m_isVisible;
	
	//頂点バッファ構造体
	struct MTGRIDBOXLIVE_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
	};
	
	//頂点バッファFVFフォーマット
	unsigned long _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }
	
	int _CreateVertexOfGrid(
			MTGRIDBOXLIVE_VERTEX* pVertex,
			unsigned long* pIbIndex
		);
	
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	
};


