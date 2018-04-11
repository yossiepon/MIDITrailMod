//******************************************************************************
//
// MIDITrail / MTBackgroundImage
//
// 背景画像描画クラス
//
// Copyright (C) 2016 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 背景画像を描画する。
// 画像ファイルは .bmp .dds .dib .jpg .png .tga を指定可能。
// （D3DXCreateTextureFromFile がサポートしている画像）

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "YNBaseLib.h"
#include "DXPrimitive.h"

using namespace YNBaseLib;


//******************************************************************************
//  背景画像描画クラス
//******************************************************************************
class MTBackgroundImage
{
public:

	//コンストラクタ／デストラクタ
	MTBackgroundImage(void);
	virtual ~MTBackgroundImage(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, HWND hWnd);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

	//リセット
	void Reset();

	//表示設定
	void SetEnable(bool isEnable);

private:

	HWND m_hWnd;
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DXIMAGE_INFO m_ImgInfo;
	bool m_isEnable;
	bool m_isFilterLinear;

	//設定ファイル
	YNConfFile m_ConfFile;

	//頂点バッファ構造体
	struct MTBACKGROUNDIMAGE_VERTEX {
		D3DXVECTOR3 p;		//頂点座標
		float		rhw;	//除算数
		DWORD		c;		//ディフューズ色
		D3DXVECTOR2 t;		//テクスチャ画像位置
	};

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	//頂点生成
	int _CreateVertexOfBackground(
			MTBACKGROUNDIMAGE_VERTEX* pVertex,
			unsigned long* pIbIndex
		);

	//設定ファイル初期化
	int _InitConfFile();

	//テクスチャ画像読み込み
	int _LoadTexture(LPDIRECT3DDEVICE9 pD3DDevice);

};

