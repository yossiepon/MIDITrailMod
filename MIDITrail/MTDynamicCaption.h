//******************************************************************************
//
// MIDITrail / MTDynamicCaption
//
// 動的キャプション描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 動的な文字列の高速描画を実現する。
// ID3DXFontはGDIを利用しているため使用しない。
// 表示する文字のテクスチャを作成しておき（例："ABCD...0123..."）、
// タイル状に並ぶ四角形ポリゴンに貼り付ける。
// 文字列の変更は、頂点データのテクスチャUV座標を更新するだけ。
// このため、以下の制限がある。
// (1) あらかじめ指定した文字しか使用できない。  
//     ＞テクスチャ画像固定のため
// (2) あらかじめ指定した文字数しか描画できない。
//     ＞ポリゴン数固定のため
// (3) 固定ピッチフォントしか使用できない。
//     ＞テクスチャ画像の文字位置を特定するのが困難なので
//
//   +-----------+
//   |A B C ... Z| テクスチャ画像
//   +-----------+
//   +-+-+-+-+
//   |N|E|K|O| ポリゴン上に1文字ずつ貼り付けられたテクスチャ
//   +-+-+-+-+

// BUG:
// シングルバイト文字しか扱えない。
// テクスチャビットマップの横幅は4の倍数になるように補正される。
// このため1文字の切り出しで誤差が生じる場合がある。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "MTFontTexture.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//キャプション最大文字数
#define MTDYNAMICCAPTION_MAX_CHARS  (256)

//******************************************************************************
// フォントタイル描画クラス
//******************************************************************************
class MTDynamicCaption
{
public:

	//コンストラクタ／デストラクタ
	MTDynamicCaption(void);
	virtual ~MTDynamicCaption(void);

	//生成
	//  pFontName   フォント名称
	//  fontSize    フォントサイズ（ポイント）
	//  pCharacters 任意文字を指定する（最大255文字）例："0123456789"
	//  captionSize キャプション文字数
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pFontName,
			unsigned long fontSize,
			const TCHAR* pCharacters,
			unsigned long captionSize
		);

	//テクスチャサイズ取得
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);

	//文字列設定
	//  Createで指定していない文字は描画されない
	//  Createで指定したキャプション文字数を超えた文字は描画しない
	int SetString(TCHAR* pStr);

	//色設定
	void SetColor(D3DXCOLOR color);

	//描画
	//  描画位置は座標変換済み頂点として扱う：ウィンドウ左上が(0,0)
	//  テクスチャサイズを参照した上で画面表示倍率を指定する
	//  magRate=1.0 ならテクスチャサイズのまま描画する
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice, float x, float y, float magRate);

	//リソース破棄
	void Release();

private:

	//頂点バッファ構造体
	struct MTDYNAMICCAPTION_VERTEX {
		D3DXVECTOR3 p;		//頂点座標
		float		rhw;	//除算数
		DWORD		c;		//ディフューズ色
		D3DXVECTOR2	t;		//テクスチャ画像位置
	};

	//頂点バッファーのフォーマットの定義：座標変換済みを指定
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	TCHAR m_Chars[MTDYNAMICCAPTION_MAX_CHARS];
	unsigned long m_CaptionSize;
	MTFontTexture m_FontTexture;
	MTDYNAMICCAPTION_VERTEX* m_pVertex;
	D3DXCOLOR m_Color;

	int _CreateTexture(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pFontName,
			unsigned long fontSize,
			const TCHAR* pCharacters
		);

	int _CreateVertex();

	int _GetTextureUV(
			TCHAR target,
			D3DXVECTOR2* pV0,
			D3DXVECTOR2* pV1,
			D3DXVECTOR2* pV2,
			D3DXVECTOR2* pV3
		);

	void _SetVertexPosition(
			MTDYNAMICCAPTION_VERTEX* pVertex,
			float x,
			float y,
			float magRate
		);

	void _SetVertexColor(
			MTDYNAMICCAPTION_VERTEX* pVertex,
			D3DXCOLOR color
		);

};


