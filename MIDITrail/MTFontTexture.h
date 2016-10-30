//******************************************************************************
//
// MIDITrail / MTFontTexture
//
// フォントテクスチャクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// フォント＞ビットマップ変換クラスを利用して、
// 文字列ビットマップからテクスチャを作成する。
// 1行の文字列にのみ対応する。複数行は対応していない。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFont2Bmp.h"


//******************************************************************************
//  フォントテクスチャクラス
//******************************************************************************
class MTFontTexture
{
public:

	//コンストラクタ／デストラクタ
	MTFontTexture(void);
	virtual ~MTFontTexture(void);

	//クリア
	void Clear();

	//フォント設定
	int SetFont(
			const TCHAR* pFontName,
			unsigned long fontSize,
			unsigned long rgb,
			bool isForceFixedPitch = false
		);

	//テクスチャ生成
	int CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pStr);

	//テクスチャインターフェースポインタ参照
	//  テクスチャオブジェクトは本クラスで管理するため、
	//  テクスチャを使用している期間は本クラスのインスタンスを破棄してはならない。
	LPDIRECT3DTEXTURE9 GetTexture();

	//テクスチャサイズ取得
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);
	
private:

	LPDIRECT3DTEXTURE9 m_pTexture;
	MTFont2Bmp m_Font2Bmp;
	unsigned long m_RGB;

	unsigned long m_TexHeight;
	unsigned long m_TexWidth;

};

