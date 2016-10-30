//******************************************************************************
//
// MIDITrail / MTPictBoard
//
// ピクチャボード描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ピクチャボードを描画する。
// 画像ファイルは .bmp .dds .dib .jpg .png .tga を指定可能。
// （D3DXCreateTextureFromFile がサポートしている画像）

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
//  ピクチャボード描画クラス
//******************************************************************************
class MTPictBoard
{
public:

	//コンストラクタ／デストラクタ
	MTPictBoard(void);
	virtual ~MTPictBoard(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

	//演奏チックタイム登録
	void SetCurTickTime(unsigned long curTickTime);

	//リセット
	void Reset();

	//演奏開始終了
	void OnPlayStart();
	void OnPlayEnd();

	//表示設定
	void SetEnable(bool isEnable);

private:

	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DXIMAGE_INFO m_ImgInfo;
	unsigned long m_CurTickTime;
	bool m_isPlay;
	bool m_isEnable;
	MTNoteDesign m_NoteDesign;

	//頂点バッファ構造体
	struct MTPICTBOARD_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
		D3DXVECTOR2 t;	//テクスチャ画像位置
	};

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	int _CreateVertexOfBoard(
			MTPICTBOARD_VERTEX* pVertex,
			unsigned long* pIbIndex
		);

	int _LoadTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);

};

