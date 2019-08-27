//******************************************************************************
//
// MIDITrail / MTGridBox
//
// グリッドボックス描画クラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridBox.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTGridBox::MTGridBox(void)
{
	m_BarNum = 0;
	m_isVisible = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTGridBox::~MTGridBox(void)
{
	Release();
}

//******************************************************************************
// グリッド生成
//******************************************************************************
int MTGridBox::Create(
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

	//頂点バッファ生成：1直方体8頂点 + (小節線2頂点 * 小節数) + (ポート分割線4頂点 * (ポート数-1))
	vertexNum = 8 + (2 * m_BarNum) + (4 * (m_PortList.GetSize() - 1));
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//インデックスバッファ生成：(1直方体12辺 * 2頂点) + (小節線2頂点 * 小節数) + (ポート分割線4頂点 * (ポート数-1))
	indexNum = 24 + (2 * m_BarNum) + (4 * (m_PortList.GetSize() - 1));
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

	//小節線の頂点とインデックスを生成
	result = _CreateVertexOfBar(
					&(pVertex[8]),	//頂点バッファ書き込み位置
					&(pIndex[24]),	//インデックスバッファ書き込み位置
					8,				//頂点インデックスオフセット
					&barList		//小節リスト
				);
	if (result != 0) goto EXIT;

	//ポート区切り線の頂点とインデックスを生成
	result = _CreateVertexOfPortSplitLine(
					&(pVertex[8 + (2 * m_BarNum)]),
					&(pIndex[24 + (2 * m_BarNum)]),
					8 + (2 * m_BarNum),
					totalTickTime
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
int MTGridBox::Transform(
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
int MTGridBox::Draw(
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
void MTGridBox::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// グリッド頂点生成
//******************************************************************************
int MTGridBox::_CreateVertexOfGrid(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long totalTickTime
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned char lastPortNo = 0;
	D3DXVECTOR3 vectorFirstPortStart[4];
	D3DXVECTOR3 vectorFirstPortEnd[4];
	D3DXVECTOR3 vectorFinalPortStart[4];
	D3DXVECTOR3 vectorFinalPortEnd[4];

	//     +   1+----+3   +
	//    /|   / 上 /    /|gridH    y x
	//   + | 0+----+2   + |右       |/
	// 左| +   7+----+5 | +      z--+0
	//   |/    / 下 /   |/
	//   +   6+----+4   + ← 4 が原点(0,0,0)
	//        gridW

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	//グリッドボックス頂点座標取得
	//  ポートごとにグリッドを描画したいが
	//  今のところは全ポートを包括したグリッドだけ描画する
	m_NoteDesign.GetGridBoxVirtexPos(
			0,
			0,
			&(vectorFirstPortStart[0]),
			&(vectorFirstPortStart[1]),
			&(vectorFirstPortStart[2]),
			&(vectorFirstPortStart[3])
		);
	m_NoteDesign.GetGridBoxVirtexPos(
			totalTickTime,
			0,
			&(vectorFirstPortEnd[0]),
			&(vectorFirstPortEnd[1]),
			&(vectorFirstPortEnd[2]),
			&(vectorFirstPortEnd[3])
		);
	m_NoteDesign.GetGridBoxVirtexPos(
			0,
			lastPortNo,
			&(vectorFinalPortStart[0]),
			&(vectorFinalPortStart[1]),
			&(vectorFinalPortStart[2]),
			&(vectorFinalPortStart[3])
		);
	m_NoteDesign.GetGridBoxVirtexPos(
			totalTickTime,
			lastPortNo,
			&(vectorFinalPortEnd[0]),
			&(vectorFinalPortEnd[1]),
			&(vectorFinalPortEnd[2]),
			&(vectorFinalPortEnd[3])
		);

	//頂点座標・・・法線が異なるので頂点を8個に集約できない
	//上の面
	pVertex[0].p = vectorFinalPortStart[0];
	pVertex[1].p = vectorFinalPortEnd[0];
	pVertex[2].p = vectorFirstPortStart[1];
	pVertex[3].p = vectorFirstPortEnd[1];
	//下の面
	pVertex[4].p = vectorFirstPortStart[3];
	pVertex[5].p = vectorFirstPortEnd[3];
	pVertex[6].p = vectorFinalPortStart[2];
	pVertex[7].p = vectorFinalPortEnd[2];

	//各頂点の法線
	for (i = 0; i < 8; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}

	//各頂点のディフューズ色
	for (i = 0; i < 8; i++) {
		pVertex[i].c = m_NoteDesign.GetGridLineColor();
	}

	//インデックス：DrawIndexdPrimitive呼び出しが1回で済むようにLINELISTとする
	unsigned long index[24] = {
		0, 1,  // 1 上面の辺
		1, 3,  // 2 ：
		3, 2,  // 3 ：
		2, 0,  // 4 ：
		6, 7,  // 5 下面の辺
		7, 5,  // 6 ：
		5, 4,  // 7 ：
		4, 6,  // 8 ：
		0, 6,  // 9 縦の線
		1, 7,  //10 ：
		3, 5,  //11 ：
		2, 4   //12 ：
	};
	for (i = 0; i < 24; i++) {
		pIndex[i] = index[i];
	}

	return result;
}

//******************************************************************************
// 小節線頂点生成
//******************************************************************************
int MTGridBox::_CreateVertexOfBar(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long vartexIndexOffset,
		SMBarList* pBarList
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long tickTime = 0;
	unsigned char lastPortNo = 0;
	D3DXVECTOR3 vectorStart[4];

	//     +   1+----+3   +
	//    /|   / 上 /    /|gridH    y x
	//   + | 0+----+2   + |右       |/
	// 左| +   7+----+5 | +      z--+0
	//   |/    / 下 /   |/
	//   +   6+----+4   + ← 4 が原点(0,0,0)
	//        gridW

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	//頂点座標：小節線は左面のy軸に沿う
	for (i = 0; i < pBarList->GetSize(); i++) {
		result = pBarList->GetBar(i, &tickTime);
		if (result != 0) goto EXIT;

		//グリッドボックス頂点座標取得
		m_NoteDesign.GetGridBoxVirtexPos(
				tickTime,
				lastPortNo,
				&(vectorStart[0]),
				&(vectorStart[1]),
				&(vectorStart[2]),
				&(vectorStart[3])
			);

		pVertex[(i*2)+0].p = vectorStart[0];
		pVertex[(i*2)+1].p = vectorStart[2];
	}

	//各頂点の法線
	for (i = 0; i < pBarList->GetSize(); i++) {
		pVertex[(i*2)+0].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVertex[(i*2)+1].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}

	//各頂点のディフューズ色
	for (i = 0; i < pBarList->GetSize(); i++) {
		pVertex[(i*2)+0].c = m_NoteDesign.GetGridLineColor();
		pVertex[(i*2)+1].c = m_NoteDesign.GetGridLineColor();
	}

	//インデックス・・・DrawIndexdPrimitive呼び出しが1回で済むようにLINELISTとする
	for (i = 0; i < pBarList->GetSize(); i++) {
		pIndex[(i*2)+0] = vartexIndexOffset + (i*2);
		pIndex[(i*2)+1] = vartexIndexOffset + (i*2)+1;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ポート区切り線頂点生成
//******************************************************************************
int MTGridBox::_CreateVertexOfPortSplitLine(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long vartexIndexOffset,
		unsigned long totalTickTime
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long j = 0;
	unsigned long count = 0;
	unsigned char portNo = 0;
	unsigned char lastPortNo = 0;
	D3DXVECTOR3 vectorStart[4];
	D3DXVECTOR3 vectorEnd[4];

	//     +   1+----+3   +
	//    /|   / 上 /    /|gridH    y x
	//   + | 0+----+2   + |右       |/
	// 左| +   7+----+5 | +      z--+0
	//   |/    / 下 /   |/
	//   +   6+----+4   + ← 4 が原点(0,0,0)
	//        gridW

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	//頂点座標：2ポート目から区切り線を生成する
	count = 0;
	for (i = 1; i < m_PortList.GetSize(); i++) {
		result = m_PortList.GetPort(i, &portNo);
		if (result != 0) goto EXIT;

		//グリッドボックス頂点座標取得
		m_NoteDesign.GetGridBoxVirtexPos(
				0,
				portNo,
				&(vectorStart[0]),
				&(vectorStart[1]),
				&(vectorStart[2]),
				&(vectorStart[3])
			);
		m_NoteDesign.GetGridBoxVirtexPos(
				totalTickTime,
				portNo,
				&(vectorEnd[0]),
				&(vectorEnd[1]),
				&(vectorEnd[2]),
				&(vectorEnd[3])
			);

		pVertex[(count*4)+0].p = vectorStart[1];
		pVertex[(count*4)+1].p = vectorEnd[1];
		pVertex[(count*4)+2].p = vectorStart[3];
		pVertex[(count*4)+3].p = vectorEnd[3];
		count++;
	}

	//座標以外の情報登録
	count = 0;
	for (i = 1; i < m_PortList.GetSize(); i++) {
		for (j = 0; j < 4; j++) {

			//法線
			pVertex[(count*4)+j].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

			//ディフューズ色
			pVertex[(count*4)+j].c = m_NoteDesign.GetGridLineColor();

			//インデックス
			pIndex[(count*4)+j] = vartexIndexOffset + (count*4) + j;
		}
		count++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// マテリアル作成
//******************************************************************************
void MTGridBox::_MakeMaterial(
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

