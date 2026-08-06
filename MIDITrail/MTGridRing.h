//******************************************************************************
//
// MIDITrail / MTGridRing
//
// グリッドリング描画クラス
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


//******************************************************************************
//  グリッドリング描画クラス
//******************************************************************************
class MTGridRing
{
public:

	//コンストラクタ／デストラクタ
	MTGridRing(void);
	virtual ~MTGridRing(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

	//表示設定
	void SetEnable(bool isEnable);

private:

	DXPrimitive m_Primitive;
	SMPortList m_PortList;
	MTNoteDesignRing m_NoteDesign;
	bool m_isVisible;
	bool m_isEnable;

	//頂点バッファ構造体
	struct MTGRIDBOX_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
	};

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	//グリッド頂点生成
	int _CreateVertexOfGrid(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pIbIndex,
			unsigned long totalTickTime,
			SMBarList* pBarList
		);

	//リング頂点生成
	int _CreateVertexOfRing(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pVirtexIndex,
			unsigned long* pIndex,
			D3DXVECTOR3 basePos
		);

	void _MakeMaterial(D3DMATERIAL9* pMaterial);

};

