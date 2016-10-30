//******************************************************************************
//
// MIDITrail / MTFontTexture
//
// フォントテクスチャクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTFontTexture.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTFontTexture::MTFontTexture(void)
{
	m_RGB = 0x00FFFFFF;
	m_TexHeight = 0;
	m_TexWidth = 0;
	m_pTexture = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTFontTexture::~MTFontTexture(void)
{
	Clear();
}

//******************************************************************************
// クリア
//******************************************************************************
void MTFontTexture::Clear()
{
	if (m_pTexture) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
	m_Font2Bmp.Clear();
}

//******************************************************************************
// フォント設定
//******************************************************************************
int MTFontTexture::SetFont(
		const TCHAR* pFontName,
		unsigned long fontSize,
		unsigned long rgb,
		bool isForceFixedPitch
	)
{
	int result = 0;

	result = m_Font2Bmp.SetFont(pFontName, fontSize, isForceFixedPitch);
	if (result != 0) goto EXIT;

	m_RGB = 0x00FFFFFF & rgb;

EXIT:;
	return result;
}

//******************************************************************************
// テクスチャ生成
//******************************************************************************
int MTFontTexture::CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pStr
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DLOCKED_RECT lockedRect;
	DWORD bmpHeight = 0;
	DWORD bmpWidth = 0;
	DWORD x, y = 0;
	DWORD alpha = 0;
	DWORD argb = 0;
	BYTE bmpPixcel = 0;
	DWORD* pDestPixcel = 0;
	int grayLevelNum = 0;

	if (m_pTexture) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}

	//フォントBMP作成
	result = m_Font2Bmp.CreateBmp(pStr);
	if (result != 0) goto EXIT;

	m_Font2Bmp.GetBmpSize(&bmpHeight, &bmpWidth);

	//テクスチャ作成
	hresult = pD3DDevice->CreateTexture(
							bmpWidth,			//テクスチャの幅（ピクセル単位）
							bmpHeight,			//テクスチャの高さ（ピクセル単位）
							1,					//テクスチャレベル
							D3DUSAGE_DYNAMIC,	//使用方法：頂点バッファが動的メモリを使用する
							D3DFMT_A8R8G8B8,	//フォーマット：32bitアルファ付ARGBフォーマット
							D3DPOOL_DEFAULT,	//テクスチャ配置先メモリクラス：デフォルト
							&m_pTexture,		//作成されたテクスチャ
							NULL				//予約
						);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//テクスチャをロックしてポインタを取得する
	hresult = m_pTexture->LockRect(
						0,					//リソースレベル
						&lockedRect,		//ロック済み領域
						NULL,				//ロックする矩形：テクスチャ全体
						D3DLOCK_DISCARD		//ロックの種類：書き込み専用
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//ロックされた領域をゼロクリア
	ZeroMemory(lockedRect.pBits, lockedRect.Pitch * bmpHeight);

	//グレイレベル数：GGO_GRAY4_BITMAPなので17段階
	grayLevelNum = 17;

	//テクスチャ上にビットマップを書き込む
	for (y = 0; y < bmpHeight; y++) {
		for (x = 0; x < bmpWidth; x++) {

			//ビットマップ側画素：8bit → 0x00～0x10(=16)が返る → アルファ値にマッピング
			bmpPixcel = m_Font2Bmp.GetBmpPixcel(x, y);

			//テクスチャ側画素：32bit ARGB
			alpha = (0xFF * bmpPixcel) / (grayLevelNum - 1);
			if (bmpPixcel == 0) {
				argb = (alpha << 24) | 0x00000000;
			}
			else {
				argb = (alpha << 24) | m_RGB;
			}

			//書き込み
			pDestPixcel = (DWORD*)((BYTE*)lockedRect.pBits + (lockedRect.Pitch * y) + (4 * x));
			*pDestPixcel = argb;
		}
	}
	
	//アンロック
	hresult = m_pTexture->UnlockRect(0);  //リソースレベル0
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	m_TexHeight = bmpHeight;
	m_TexWidth = bmpWidth;

EXIT:;
	m_Font2Bmp.Clear();
	return result;
}

//******************************************************************************
// テクスチャポインタ取得
//******************************************************************************
LPDIRECT3DTEXTURE9 MTFontTexture::GetTexture()
{
	return m_pTexture;
}

//******************************************************************************
// テクスチャサイズ取得
//******************************************************************************
void MTFontTexture::GetTextureSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	*pHeight = m_TexHeight;
	*pWidth = m_TexWidth;
}

