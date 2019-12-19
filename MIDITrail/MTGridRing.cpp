//******************************************************************************
//
// MIDITrail / MTGridRing
//
// グリッドリング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridRing.h"
#include "DXH.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTGridRing::MTGridRing(void)
{
	m_BarNum = 0;
	m_isVisible = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTGridRing::~MTGridRing(void)
{
	Release();
}

//******************************************************************************
// グリッド生成
//******************************************************************************
int MTGridRing::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
   )
{
	int result = 0;
	SMBarList barList;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTGRIDBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long totalTickTime = 0;
	D3DMATERIAL9 material;
	D3DXCOLOR lineColor;

	Release();

	if ((pD3DDevice == NULL) || (pSeqData == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//シーケンスデータ：時間情報取得
	totalTickTime = pSeqData->GetTotalTickTime();

	//シーケンスデータ：小節リスト取得
	result = pSeqData->GetBarList(&barList);
	if (result != 0) goto EXIT;

	//シーケンスデータ：ポートリスト取得
	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	//小節数
	m_BarNum = barList.GetSize();

	//プリミティブ初期化
	result = m_Primitive.Initialize(
					sizeof(MTGRIDBOX_VERTEX),	//頂点サイズ
					_GetFVFFormat(),			//頂点FVFフォーマット
					D3DPT_LINELIST				//プリミティブ種別
				);
	if (result != 0) goto EXIT;

	//頂点バッファ生成：1リング128頂点 * 2(先端/終端)
	vertexNum = 128 * 2;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//インデックスバッファ生成：1リング128辺 * 2(始点/終点) * 2(先端/終端)
	indexNum = 128 * 2 * 2;
	result = m_Primitive.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//バッファのロック
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//グリッドボックスの頂点とインデックスを生成
	result = _CreateVertexOfGrid(
					pVertex,		//頂点バッファ書き込み位置
					pIndex,			//インデックスバッファ書き込み位置
					totalTickTime	//トータルチックタイム
				);
	if (result != 0) goto EXIT;

	//バッファのロック解除
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_Primitive.UnlockIndex();
	if (result != 0) goto EXIT;

	//マテリアル作成
	_MakeMaterial(&material);
	m_Primitive.SetMaterial(material);

	//グリッドの色を確認
	lineColor = m_NoteDesign.GetGridLineColor();
	if (((DWORD)lineColor & 0xFF000000) == 0) {
		//透明なら描画しない
		m_isVisible = false;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTGridRing::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
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

	//移動行列
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//行列の合成
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//変換行列設定
	m_Primitive.Transform(worldMatrix);

	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTGridRing::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (m_isVisible) {
		result = m_Primitive.Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTGridRing::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// グリッド頂点生成
//******************************************************************************
int MTGridRing::_CreateVertexOfGrid(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long totalTickTime
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long virtexIndex = 0;
	unsigned long virtexIndexStart = 0;
	D3DXVECTOR3 basePosStart;
	D3DXVECTOR3 basePosEnd;
	D3DXVECTOR3 rotatedPos;
	float angle = 0.0f;

	//基準座標取得
	m_NoteDesign.GetGridRingBasePos(totalTickTime, &basePosStart, &basePosEnd);

	//----------------------------------
	//先端リング
	//----------------------------------
	//頂点作成
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePosStart;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
	for (i = 1; i < 128; i++) {
		virtexIndex++;
		
		//回転後の頂点
		angle = (360.0f / 128.0f) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePosStart, angle);
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

	virtexIndex++;

	//----------------------------------
	//終端リング
	//----------------------------------
	//頂点作成
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePosEnd;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
	for (i = 1; i < 128; i++) {
		virtexIndex++;
		
		//回転後の頂点
		angle = (360.0f / 128.0f) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePosEnd, angle);
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
// マテリアル作成
//******************************************************************************
void MTGridRing::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));

	//拡散光
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//環境光：影の色
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//鏡面反射光
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//鏡面反射光の鮮明度
	pMaterial->Power = 10.0f;
	//発光色
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}


