//******************************************************************************
//
// MIDITrail / MTStaticCaption
//
// 静的キャプション描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 静的な文字列の高速描画を実現する。
// ID3DXFontはGDIを利用しているため使用しない。
// 表示する文字のテクスチャを作成しておき、四角形ポリゴンにそのまま貼り付ける。
// 後から文字列を変更することはできない。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFontTexture.h"


//******************************************************************************
// 静的キャプション描画クラス
//******************************************************************************
class MTStaticCaption
{
public:

	//コンストラクタ／デストラクタ
	MTStaticCaption(void);
	virtual ~MTStaticCaption(void);

	//生成
	//  pFontName   フォント名称
	//  fontSize    フォントサイズ（ポイント）
	//  pCaption    キャプション文字列
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			TCHAR* pFontName,
			unsigned long fontSize,
			TCHAR* pCaptin
		);
	
	//テクスチャサイズ取得
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);

	//色設定
	void SetColor(D3DXCOLOR color);

	//描画
	//  描画位置は座標変換済み頂点として扱う。ウィンドウ左上が(0,0)。
	//  テクスチャサイズを参照した上で画面表示倍率を指定する。
	//  magRate=1.0 ならテクスチャサイズのまま描画される。
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice, float x, float y, float magRate);

	//リソース破棄
	void Release();

private:

	//頂点バッファ構造体
	struct MTSTATICCAPTION_VERTEX {
		D3DXVECTOR3 p;		//頂点座標
		float		rhw;	//除算数
		DWORD		c;		//ディフューズ色
		D3DXVECTOR2 t;		//テクスチャ画像位置
	};

	//頂点バッファーのフォーマットの定義：座標変換済みを指定
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	MTFontTexture m_FontTexture;
	MTSTATICCAPTION_VERTEX* m_pVertex;
	D3DXCOLOR m_Color;

	int _CreateTexture(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pFontName,
			unsigned long fontSize,
			const TCHAR* pCaption
		);

	int _CreateVertex();

	void _SetVertexPosition(
			MTSTATICCAPTION_VERTEX* pVertex,
			float x,
			float y,
			float magRate
		);

	void _SetVertexColor(
			MTSTATICCAPTION_VERTEX* pVertex,
			D3DXCOLOR color
		);

};


