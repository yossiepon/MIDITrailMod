//******************************************************************************
//
// MIDITrail / MTGridBox
//
// グリッドボックス描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// グリッドボックスと小節線を描画する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
//  グリッドボックス描画クラス
//******************************************************************************
class MTGridBox
{
public:

	//コンストラクタ／デストラクタ
	MTGridBox(void);
	virtual ~MTGridBox(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

// >>> modify function to virtual 20180404 yossiepon begin
	//更新
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify function to virtual 20180404 yossiepon end

	//解放
	void Release();

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	DXPrimitive m_Primitive;

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	unsigned long m_BarNum;
	SMPortList m_PortList;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	MTNoteDesign m_NoteDesign;

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	bool m_isVisible;

	//頂点バッファ構造体
	struct MTGRIDBOX_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
	};

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateVertexOfGrid(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pIbIndex,
			unsigned long totalTickTime
		);

	int _CreateVertexOfBar(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pIbIndex,
			unsigned long vartexIndexOffset,
			SMBarList* pBarList
		);

	int _CreateVertexOfPortSplitLine(
			MTGRIDBOX_VERTEX* pVertex,
			unsigned long* pIndex,
			unsigned long vartexIndexOffset,
			unsigned long totalTickTime
		);

	void _MakeMaterial(D3DMATERIAL9* pMaterial);

};

