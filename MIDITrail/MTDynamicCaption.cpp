//******************************************************************************
//
// MIDITrail / MTDynamicCaption
//
// 動的キャプション描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTDynamicCaption.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTDynamicCaption::MTDynamicCaption(void)
{
	m_pVertex = NULL;
	m_Chars[0] = _T('\0');
	m_CaptionSize = 0;
	m_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTDynamicCaption::~MTDynamicCaption(void)
{
	Release();
}

//******************************************************************************
// フォントタイル生成
//******************************************************************************
int MTDynamicCaption::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pFontName,
		unsigned long fontSize,
		const TCHAR* pCharacters,
		unsigned long captionSize
   )
{
	int result = 0;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pFontName == NULL) || (fontSize == 0)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pCharacters == NULL) ||(captionSize == 0)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	m_CaptionSize = captionSize;
	
	//テクスチャ生成
	result = _CreateTexture(pD3DDevice, pFontName, fontSize, pCharacters);
	if (result != 0) goto EXIT;
	
	//タイルの頂点を生成する
	result = _CreateVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// テクスチャサイズ取得
//******************************************************************************
void MTDynamicCaption::GetTextureSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	m_FontTexture.GetTextureSize(pHeight, pWidth);
}

//******************************************************************************
// 文字列設定
//******************************************************************************
int MTDynamicCaption::SetString(
		TCHAR* pStr
	)
{
	int result = 0;
	unsigned long i = 0;
	D3DXVECTOR2 v0, v1, v2, v3;
	
	if (pStr == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	for (i= 0; i < 6*m_CaptionSize; i++) {
		m_pVertex[i].t = D3DXVECTOR2(0.0f, 0.0f);
	}
	for (i= 0; i < m_CaptionSize; i++) {
		if (pStr[i] == _T('\0')) break;

		result = _GetTextureUV(pStr[i], &v0, &v1, &v2, &v3);
		if (result != 0) goto EXIT;
		
		// 0+--+1
		//  | /|
		//  |/ |
		// 2+--+3
		m_pVertex[6*i+0].t = v0;
		m_pVertex[6*i+1].t = v1;
		m_pVertex[6*i+2].t = v2;
		m_pVertex[6*i+3].t = v2;
		m_pVertex[6*i+4].t = v1;
		m_pVertex[6*i+5].t = v3;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 文字列設定
//******************************************************************************
void MTDynamicCaption::SetColor(
		D3DXCOLOR color
	)
{
	m_Color = color;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTDynamicCaption::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float x,
		float y,
		float magRate
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DXMATRIX mtxWorld;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pVertex == NULL) {
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
					D3DPT_TRIANGLELIST,				//プリミティブ種別
					2 * m_CaptionSize,				//プリミティブ数
					m_pVertex,						//頂点データ
					sizeof(MTDYNAMICCAPTION_VERTEX)	//頂点データのサイズ
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, m_CaptionSize);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTDynamicCaption::Release()
{
	m_FontTexture.Clear();
	
	delete [] m_pVertex;
	m_pVertex = NULL;
}

//******************************************************************************
// テクスチャ生成
//******************************************************************************
int MTDynamicCaption::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pFontName,
		unsigned long fontSize,
		const TCHAR* pCharacters
	)
{
	int result = 0;
	errno_t eresult = 0;
	unsigned long color = 0x00FFFFFF;
	bool isForceFixedPitch = true;

	//タイル文字一覧を格納
	eresult = _tcscpy_s(m_Chars, MTDYNAMICCAPTION_MAX_CHARS, pCharacters);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//フォント設定：固定ピッチ強制
	result = m_FontTexture.SetFont(pFontName, fontSize, color, isForceFixedPitch);
	if (result != 0) goto EXIT;

	//タイル文字一覧テクスチャ作成
	result = m_FontTexture.CreateTexture(pD3DDevice, pCharacters);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// フォントタイル頂点生成
//******************************************************************************
int MTDynamicCaption::_CreateVertex()
{
	int result = 0;
	unsigned long i = 0;
	MTDYNAMICCAPTION_VERTEX* pVertex = NULL;

	//頂点生成
	try {
		pVertex = new MTDYNAMICCAPTION_VERTEX[6*m_CaptionSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", m_CaptionSize, 0);
		goto EXIT;
	}

	//頂点座標設定
	_SetVertexPosition(
			pVertex,	//頂点座標配列
			0.0f,		//描画位置x
			0.0f,		//描画位置y
			1.0f		//拡大率
		);

	for (i = 0; i < 6*m_CaptionSize; i++) {
		//各頂点の除算数
		pVertex[i].rhw = 1.0f;
		//各頂点のディフューズ色
		pVertex[i].c = m_Color;
		//各頂点のテクスチャ座標
		pVertex[i].t = D3DXVECTOR2(0.0f, 0.0f);
	}

	m_pVertex = pVertex;
	pVertex = NULL;

EXIT:;
	delete [] pVertex;
	return result;
}

//******************************************************************************
// テクスチャUV座標取得
//******************************************************************************
int MTDynamicCaption::_GetTextureUV(
		TCHAR target,
		D3DXVECTOR2* pV0,
		D3DXVECTOR2* pV1,
		D3DXVECTOR2* pV2,
		D3DXVECTOR2* pV3
	)
{
	int result = 0;
	bool isFound = false;
	unsigned long i = 0;
	unsigned long charsNum = 0;
	float fontNo = 0;
	float fontWidth = 0.0f;

	charsNum = _tcslen(m_Chars);
	for (i = 0; i < charsNum; i++) {
		if (m_Chars[i] == target) {
			isFound = true;
			fontNo = (float)i;
			break;
		}
	}

	fontWidth = 1.0f / (float)charsNum;

	//見つかった場合は該当する文字のUV座標を設定
	if (isFound) {
		//左上
		pV0->x = 1.0f * fontNo / (float)charsNum;
		pV0->y = 0.0f;
		//右上
		pV1->x = 1.0f * (fontNo + 1.0f) / (float)charsNum;
		pV1->y = 0.0f;
		//左下
		pV2->x = 1.0f * fontNo / (float)charsNum;
		pV2->y = 1.0f;
		//右下
		pV3->x = 1.0f * (fontNo + 1.0f) / (float)charsNum;
		pV3->y = 1.0f;
	}
	//見つからない場合はテクスチャ無効とする
	else {
		//左上
		pV0->x = 0.0f;
		pV0->y = 0.0f;
		//右上
		pV1->x = 0.0f;
		pV1->y = 0.0f;
		//左下
		pV2->x = 0.0f;
		pV2->y = 0.0f;
		//右下
		pV3->x = 0.0f;
		pV3->y = 0.0f;
	}

	return result;
}

//******************************************************************************
// 頂点位置設定
//******************************************************************************
void MTDynamicCaption::_SetVertexPosition(
		MTDYNAMICCAPTION_VERTEX* pVertex,
		float x,
		float y,
		float magRate
	)
{
	unsigned long i = 0;
	unsigned long texHeight = 0;
	unsigned long texWidth = 0;
	unsigned long charsNum = 0;
	float height = 0.0f;
	float width = 0.0f;

	charsNum = _tcslen(m_Chars);

	//描画サイズ
	m_FontTexture.GetTextureSize(&texHeight, &texWidth);
	height = texHeight * magRate;
	width  = ((float)texWidth / (float)charsNum) * magRate;

	//頂点座標
	for (i = 0; i < m_CaptionSize; i++) {
		pVertex[i*6+0].p = D3DXVECTOR3(width * (i     ), 0.0f,   0.0f);
		pVertex[i*6+1].p = D3DXVECTOR3(width * (i+1.0f), 0.0f,   0.0f);
		pVertex[i*6+2].p = D3DXVECTOR3(width * (i     ), height, 0.0f);
		pVertex[i*6+3].p = pVertex[i*6+2].p;
		pVertex[i*6+4].p = pVertex[i*6+1].p;
		pVertex[i*6+5].p = D3DXVECTOR3(width * (i+1.0f), height, 0.0f);
	}

	//描画位置に移動
	for (i = 0; i < 6*m_CaptionSize; i++) {
		pVertex[i].p.x += x;
		pVertex[i].p.y += y;
	}

	return;
}

//******************************************************************************
// 頂点色設定
//******************************************************************************
void MTDynamicCaption::_SetVertexColor(
		MTDYNAMICCAPTION_VERTEX* pVertex,
		D3DXCOLOR color
	)
{
	unsigned long i = 0;

	for (i = 0; i < 6*m_CaptionSize; i++) {
		pVertex[i].c = color;
	}

	return;
}

