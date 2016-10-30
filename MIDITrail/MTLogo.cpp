//******************************************************************************
//
// MIDITrail / MTLogo
//
// MIDITrail ロゴ描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ポリゴンをタイル状に並べてロゴのテクスチャを貼り付ける。
// ポリゴンの色をタイルごとに更新することでロゴをグラデーションさせる。
//
//   +-++-++-++-+
//   |/||/||/||/|...
//   +-++-++-++-+

#include "StdAfx.h"
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "MTLogo.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTLogo::MTLogo(void)
{
	m_pVertex = NULL;
	m_StartTime = 0;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTLogo::~MTLogo(void)
{
	Release();
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTLogo::Create(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	Release();

	//テクスチャ生成
	result = _CreateTexture(pD3DDevice);
	if (result != 0) goto EXIT;

	//頂点生成
	result = _CreateVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 変換処理
//******************************************************************************
int MTLogo::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	//タイトルグラデーション設定
	_SetGradationColor();

	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTLogo::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

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

	//タイトル文字描画
	hresult = pD3DDevice->DrawPrimitiveUP(
					D3DPT_TRIANGLELIST,		//プリミティブ種別
					2 * MTLOGO_TILE_NUM,		//プリミティブ数
					m_pVertex,				//頂点データ
					sizeof(MTLOGO_VERTEX)	//頂点データのサイズ
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, MTLOGO_TILE_NUM);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 破棄
//******************************************************************************
void MTLogo::Release()
{
	m_FontTexture.Clear();

	delete [] m_pVertex;
	m_pVertex = NULL;
}

//******************************************************************************
// テクスチャ生成
//******************************************************************************
int MTLogo::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long color = 0x00FFFFFF;
	bool isForceFixedPitch = false;

	//フォント設定
	result = m_FontTexture.SetFont(
					_T("Arial"),		//フォント名称
					40,					//フォントサイズ
					color,				//色
					isForceFixedPitch	//固定ピッチ強制
				);
	if (result != 0) goto EXIT;

	//タイル文字一覧テクスチャ作成
	result = m_FontTexture.CreateTexture(pD3DDevice, MTLOGO_TITLE);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// フォントタイル頂点生成
//******************************************************************************
int MTLogo::_CreateVertex()
{
	int result = 0;
	unsigned long i = 0;
	MTLOGO_VERTEX* pVertex = NULL;

	//頂点生成
	try {
		pVertex = new MTLOGO_VERTEX[6*MTLOGO_TILE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//頂点座標設定
	_SetVertexPosition(
			pVertex,		//頂点座標配列
			MTLOGO_POS_X,	//描画位置x
			MTLOGO_POS_Y,	//描画位置y
			MTLOGO_MAG		//拡大率
		);

	for (i = 0; i < 6*MTLOGO_TILE_NUM; i++) {
		//各頂点のディフューズ色
		pVertex[i].c = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f); //R,G,B,A
	}

	m_pVertex = pVertex;
	pVertex = NULL;

EXIT:;
	delete [] pVertex;
	return result;
}

//******************************************************************************
// 頂点位置設定
//******************************************************************************
void MTLogo::_SetVertexPosition(
		MTLOGO_VERTEX* pVertex,
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
	float tileNo = 0.0f;

	//タイルサイズ
	m_FontTexture.GetTextureSize(&texHeight, &texWidth);
	height = texHeight * magRate;
	width  = ((float)texWidth / (float)MTLOGO_TILE_NUM) * magRate;

	//頂点座標：XY平面の(0, 0)を左上とする
	for (i = 0; i < MTLOGO_TILE_NUM; i++) {
		//頂点座標
		pVertex[i*6+0].p = D3DXVECTOR3(width * (i     ),  0.0f,   0.0f);
		pVertex[i*6+1].p = D3DXVECTOR3(width * (i+1.0f),  0.0f,   0.0f);
		pVertex[i*6+2].p = D3DXVECTOR3(width * (i     ), -height, 0.0f);
		pVertex[i*6+3].p = pVertex[i*6+2].p;
		pVertex[i*6+4].p = pVertex[i*6+1].p;
		pVertex[i*6+5].p = D3DXVECTOR3(width * (i+1.0f), -height, 0.0f);

		//テクスチャ座標
		//左上
		pVertex[i*6+0].t.x = 1.0f * tileNo / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+0].t.y = 0.0f;
		//右上
		pVertex[i*6+1].t.x = 1.0f * (tileNo + 1.0f) / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+1].t.y = 0.0f;
		//左下
		pVertex[i*6+2].t.x = 1.0f * tileNo / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+2].t.y = 1.0f;
		//左下
		pVertex[i*6+3].t = pVertex[i*6+2].t;
		//右上
		pVertex[i*6+4].t = pVertex[i*6+1].t;
		//右下
		pVertex[i*6+5].t.x = 1.0f * (tileNo + 1.0f) / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+5].t.y = 1.0f;

		tileNo += 1.0f;
	}

	//法線
	for (i = 0; i < 6*MTLOGO_TILE_NUM; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}

	//指定位置に移動
	for (i = 0; i < 6*MTLOGO_TILE_NUM; i++) {
		pVertex[i].p.x += x;
		pVertex[i].p.y += y;
	}

	return;
}

//******************************************************************************
// グラデーション設定
//******************************************************************************
void MTLogo::_SetGradationColor()
{
	unsigned long i = 0;
	unsigned long sceneTime = 0;
	unsigned long delay = 0;
	unsigned long tileTime = 0;
	float color = 0.0f;
	MTLOGO_VERTEX* pVertex = NULL;

	//シーン経過時間
	if (m_StartTime == 0) {
		m_StartTime = timeGetTime();
	}
	sceneTime = timeGetTime() - m_StartTime;

	//グラデーション処理
	for (i = 0; i < MTLOGO_TILE_NUM; i++) {

		//タイルごとの遅延時間
		delay = i * (MTLOGO_GRADATION_TIME / MTLOGO_TILE_NUM);

		//タイルにとっての経過時間
		if (sceneTime < delay) {
			tileTime = 0;
		}
		else {
			tileTime = sceneTime - delay;
		}

		//タイルの色
		if (tileTime < MTLOGO_GRADATION_TIME) {
			//黒→白
			color = (float)tileTime / (float)MTLOGO_GRADATION_TIME;
		}
		else if (tileTime < (MTLOGO_GRADATION_TIME*2)) {
			//白→黒
			color = 1.0f - ((float)(tileTime - MTLOGO_GRADATION_TIME) / (float)MTLOGO_GRADATION_TIME);
		}
		else {
			//黒
			color = 0.0f;
		}

		//タイルの色を頂点に設定
		pVertex = m_pVertex + (6*i);
		_SetTileColor(pVertex, color);
	}

	return;
}

//******************************************************************************
// タイル色設定
//******************************************************************************
void MTLogo::_SetTileColor(
		MTLOGO_VERTEX* pVertex,
		float color
	)
{
	unsigned long i = 0;

	for (i = 0; i < 6; i++) {
		pVertex[i].c = D3DXCOLOR(color, color, color, 1.0f); //R,G,B,A
	}
}

