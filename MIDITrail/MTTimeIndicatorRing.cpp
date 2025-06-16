//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRing
//
// タイムインジケータリング描画クラス
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTTimeIndicatorRing.h"
#include "DXH.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTTimeIndicatorRing::MTTimeIndicatorRing(void)
{
	m_CurPos = 0.0f;
	m_CurTickTime = 0;
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTTimeIndicatorRing::~MTTimeIndicatorRing(void)
{
	Release();
}

//******************************************************************************
// タイムインジケータ生成
//******************************************************************************
int MTTimeIndicatorRing::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMBarList barList;

	Release();

	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//プリミティブ生成：タイムライン
	result = _CreatePrimitiveLine(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// プリミティブ生成
//******************************************************************************
int MTTimeIndicatorRing::_CreatePrimitiveLine(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTTIMEINDICATOR_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	//プリミティブ初期化
	result = m_PrimitiveLine.Initialize(
					sizeof(MTTIMEINDICATOR_VERTEX),	//頂点サイズ
					_GetFVFFormat(),				//頂点FVFフォーマット
					D3DPT_LINELIST					//プリミティブ種別
				);
	if (result != 0) goto EXIT;

	//頂点バッファ生成：1サークル128頂点
	vertexNum = 128;
	result = m_PrimitiveLine.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//インデックスバッファ生成：1サークル128辺 * 2(始点/終点)
	indexNum = 128 * 2;
	result = m_PrimitiveLine.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//バッファのロック
	result = m_PrimitiveLine.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveLine.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//バッファに頂点とインデックスを書き込む
	result = _CreateVertexOfIndicatorLine(pVertex, pIndex);
	if (result != 0) goto EXIT;

	//バッファのロック解除
	result = m_PrimitiveLine.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveLine.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTTimeIndicatorRing::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//行列初期化
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//回転行列
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//演奏位置
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	//移動行列
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x + m_CurPos, moveVector.y, moveVector.z);

	//行列の合成
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//変換行列設定
	m_PrimitiveLine.Transform(worldMatrix);

	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTTimeIndicatorRing::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	//テクスチャステージ設定
	//  カラー演算：引数1を使用  引数1：テクスチャ
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// アルファ演算：引数1を使用  引数1：テクスチャ
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//テクスチャフィルタ
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	result = m_PrimitiveLine.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTTimeIndicatorRing::Release()
{
	m_PrimitiveLine.Release();
}

//******************************************************************************
// タイムインジケータライン頂点生成
//******************************************************************************
int MTTimeIndicatorRing::_CreateVertexOfIndicatorLine(
		MTTIMEINDICATOR_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long virtexIndex = 0;
	unsigned long virtexIndexStart = 0;
	D3DXVECTOR3 basePos;
	D3DXVECTOR3 rotatedPos;
	float angle = 0.0f;

	//基準座標
	basePos = D3DXVECTOR3(m_NoteDesign.GetPlayPosX(0),
							m_NoteDesign.GetPortOriginY(0),
							m_NoteDesign.GetPortOriginZ(0));

	//頂点作成
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
	for (i = 1; i < 128; i++) {
		virtexIndex++;
		
		//回転後の頂点
		angle = (360.0f / 128.0f) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
		
		//インデックスバッファ（前回の頂点から今回の頂点）
		pIndex[(virtexIndex - 1) * 2]     = virtexIndex - 1;
		pIndex[(virtexIndex - 1) * 2 + 1] = virtexIndex;
	}
	//終点と始点をつなぐ線
	pIndex[virtexIndex * 2]     = virtexIndex;
	pIndex[virtexIndex * 2 + 1] = virtexIndexStart;

	return result;
}

//******************************************************************************
// チックタイム設定
//******************************************************************************
void MTTimeIndicatorRing::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
}

//******************************************************************************
// リセット
//******************************************************************************
void MTTimeIndicatorRing::Reset()
{
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
}

//******************************************************************************
// 現在位置取得
//******************************************************************************
float MTTimeIndicatorRing::GetPos()
{
	return m_CurPos;
}

//******************************************************************************
// 移動ベクトル取得
//******************************************************************************
D3DXVECTOR3 MTTimeIndicatorRing::GetMoveVector()
{
	return D3DXVECTOR3(m_CurPos, 0.0f, 0.0f);
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTTimeIndicatorRing::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


