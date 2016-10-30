//******************************************************************************
//
// MIDITrail / MTLogo
//
// MIDITrail ロゴ描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFontTexture.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//タイトル文字列
#define MTLOGO_TITLE  _T("MIDITrail")

//ロゴ描画位置情報
#define MTLOGO_POS_X  (20.0f)   //描画位置x
#define MTLOGO_POS_Y  (-15.0f)  //描画位置y
#define MTLOGO_MAG    (0.1f)    //拡大率

//タイル分割数
#define MTLOGO_TILE_NUM  (40)

//グラデーション時間間隔(msec)
#define MTLOGO_GRADATION_TIME  (1000)


//******************************************************************************
// MIDITrail ロゴ描画クラス
//******************************************************************************
class MTLogo
{
public:

	//コンストラクタ／デストラクタl
	MTLogo(void);
	virtual ~MTLogo(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice);

	//変換
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//破棄
	void Release();

private:

	//頂点バッファ構造体
	struct MTLOGO_VERTEX {
		D3DXVECTOR3 p;		//頂点座標
		D3DXVECTOR3 n;		//法線
		DWORD		c;		//ディフューズ色
		D3DXVECTOR2	t;		//テクスチャ画像位置
	};

	//頂点バッファーのフォーマットの定義：座標変換済みを指定
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

private:

	//フォントテクスチャ
	MTFontTexture m_FontTexture;
	MTLOGO_VERTEX* m_pVertex;

	unsigned long m_StartTime;
	unsigned long m_GradationTime;

	int _CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice);

	int _CreateVertex();

	int _GetTextureUV(
			TCHAR target,
			D3DXVECTOR2* pV0,
			D3DXVECTOR2* pV1,
			D3DXVECTOR2* pV2,
			D3DXVECTOR2* pV3
		);

	void _SetVertexPosition(
			MTLOGO_VERTEX* pVertex,
			float x,
			float y,
			float magRate
		);

	void _SetGradationColor();

	void _SetTileColor(
			MTLOGO_VERTEX* pVertex,
			float color
		);

};

