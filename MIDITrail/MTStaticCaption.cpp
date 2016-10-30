//******************************************************************************
//
// MIDITrail / MTStaticCaption
//
// 静的キャプション描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTStaticCaption.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTStaticCaption::MTStaticCaption(void)
{
	m_pVertex = NULL;
	m_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTStaticCaption::~MTStaticCaption(void)
{
	Release();
}

//******************************************************************************
// 静的キャプション生成
//******************************************************************************
int MTStaticCaption::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		TCHAR* pFontName,
		unsigned long fontSize,
		TCHAR* pCaption
   )
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pFontName == NULL) || (fontSize == 0) || (pCaption == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	Release();

	//テクスチャ生成
	result = _CreateTexture(pD3DDevice, pFontName, fontSize, pCaption);
	if (result != 0) goto EXIT;

	//頂点を生成する
	result = _CreateVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// テクスチャサイズ取得
//******************************************************************************
void MTStaticCaption::GetTextureSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	m_FontTexture.GetTextureSize(pHeight, pWidth);
}

//******************************************************************************
// 文字列設定
//******************************************************************************
void MTStaticCaption::SetColor(
		D3DXCOLOR color
	)
{
	m_Color = color;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTStaticCaption::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float x,
		float y,
		float magRate
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DXMATRIX mtxWorld;

	if ((pD3DDevice == NULL) || (m_pVertex == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//頂点座標設定
	_SetVertexPosition(
			m_pVertex,	//頂点座標配列
			x,			//描画位置x
			y,			//描画位置y
			magRate		//拡大率
		);

	//頂点色設定
	_SetVertexColor(m_pVertex, m_Color);

	//レンダリングパイプラインにテクスチャを設定
	hresult = pD3DDevice->SetTexture(
					0,							//ステージ識別子
					m_FontTexture.GetTexture()	//テクスチャオブジェクト
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//テクスチャステージ設定
	//  カラー演算：引数1を使用  引数1：ポリゴン
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	// アルファ演算：引数1を使用  引数1：テクスチャ
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//テクスチャフィルタ
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//レンダリングパイプラインに頂点バッファFVFフォーマットを設定
	hresult = pD3DDevice->SetFVF(_GetFVFFormat());
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//レンダリングパイプラインにマテリアルを設定
	//なし

	//全ボード描画
	hresult = pD3DDevice->DrawPrimitiveUP(
					D3DPT_TRIANGLESTRIP,			//プリミティブ種別
					2,								//プリミティブ数
					m_pVertex,						//頂点データ
					sizeof(MTSTATICCAPTION_VERTEX)	//頂点データのサイズ
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTStaticCaption::Release()
{
	m_FontTexture.Clear();

	delete [] m_pVertex;
	m_pVertex = NULL;
}

//******************************************************************************
// テクスチャ生成
//******************************************************************************
int MTStaticCaption::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pFontName,
		unsigned long fontSize,
		const TCHAR* pCaption
	)
{
	int result = 0;
	bool isForceFixedPitch = false;
	unsigned long color = 0x00FFFFFF;

	//フォント設定：固定ピッチ強制
	result = m_FontTexture.SetFont(pFontName, fontSize, color, isForceFixedPitch);
	if (result != 0) goto EXIT;

	//タイル文字一覧テクスチャ作成
	result = m_FontTexture.CreateTexture(pD3DDevice, pCaption);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 頂点生成
//******************************************************************************
int MTStaticCaption::_CreateVertex()
{
	int result = 0;
	unsigned long i = 0;
	MTSTATICCAPTION_VERTEX* pVertex = NULL;
	
	//頂点生成
	try {
		pVertex = new MTSTATICCAPTION_VERTEX[4];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//頂点座標設定
	_SetVertexPosition(
			pVertex,	//頂点座標配列
			0.0f,		//描画位置x
			0.0f,		//描画位置y
			1.0f		//拡大率
		);

	for (i = 0; i < 4; i++) {
		//各頂点の除算数
		pVertex[i].rhw = 1.0f;
		//各頂点のディフューズ色
		pVertex[i].c = m_Color;
	}

	//テクスチャ座標
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = D3DXVECTOR2(1.0f, 1.0f);

	m_pVertex = pVertex;
	pVertex = NULL;

EXIT:;
	delete [] pVertex;
	return result;
}

//******************************************************************************
// 頂点位置設定
//******************************************************************************
void MTStaticCaption::_SetVertexPosition(
		MTSTATICCAPTION_VERTEX* pVertex,
		float x,
		float y,
		float magRate
	)
{
	unsigned long i = 0;
	unsigned long texHeight = 0;
	unsigned long texWidth = 0;
	float height = 0.0f;
	float width = 0.0f;

	//描画サイズ
	m_FontTexture.GetTextureSize(&texHeight, &texWidth);
	height = (float)texHeight * magRate;
	width  = (float)texWidth  * magRate;

	//頂点座標
	pVertex[0].p = D3DXVECTOR3(0.0f , 0.0f,   0.0f);
	pVertex[1].p = D3DXVECTOR3(width, 0.0f,   0.0f);
	pVertex[2].p = D3DXVECTOR3(0.0f , height, 0.0f);
	pVertex[3].p = D3DXVECTOR3(width, height, 0.0f);

	//描画位置に移動
	for (i = 0; i < 4; i++) {
		pVertex[i].p.x += x;
		pVertex[i].p.y += y;
	}

	return;
}

//******************************************************************************
// 頂点色設定
//******************************************************************************
void MTStaticCaption::_SetVertexColor(
		MTSTATICCAPTION_VERTEX* pVertex,
		D3DXCOLOR color
	)
{
	unsigned long i = 0;

	for (i = 0; i < 4; i++) {
		pVertex[i].c = color;
	}

	return;
}

