//******************************************************************************
//
// MIDITrail / DXPrimitive
//
// プリミティブ描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXPrimitive.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
DXPrimitive::DXPrimitive(void)
{
	m_VertexSize = 0;
	m_FVFFormat = 0;
	m_PrimitiveType = D3DPT_TRIANGLELIST;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	ZeroMemory(&m_Material, sizeof(D3DMATERIAL9));
	D3DXMatrixIdentity(&m_WorldMatrix);
	m_VertexNum = 0;
	m_IndexNum = 0;
	m_IsVertexLocked = false;
	m_IsIndexLocked = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DXPrimitive::~DXPrimitive(void)
{
	Release();
}

//******************************************************************************
// 解放
//******************************************************************************
void DXPrimitive::Release()
{
	if (m_pVertexBuffer != NULL) {
		m_pVertexBuffer->Release();
		m_pVertexBuffer = NULL;
	}
	if (m_pIndexBuffer != NULL) {
		m_pIndexBuffer->Release();
		m_pIndexBuffer = NULL;
	}
	m_VertexNum = 0;
	m_IndexNum = 0;
	m_IsVertexLocked = false;
	m_IsIndexLocked = false;
}

//******************************************************************************
// 初期化
//******************************************************************************
int DXPrimitive::Initialize(
		unsigned long vertexSize,
		unsigned long fvfFormat,
		D3DPRIMITIVETYPE type
	)
{
	int result = 0;

	Release();

	m_VertexSize = vertexSize;
	m_FVFFormat = fvfFormat;
	m_PrimitiveType = type;
	_GetDefaultMaterial(&m_Material);

	return result;
}

//******************************************************************************
// 頂点バッファ生成
//******************************************************************************
int DXPrimitive::CreateVertexBuffer(
		LPDIRECT3DDEVICE9 pD3DDevice,
		unsigned long vertexNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pVertexBuffer != NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//頂点バッファ生成
	if (vertexNum > 0) {
		hresult = pD3DDevice->CreateVertexBuffer(
						m_VertexSize * vertexNum,	//頂点バッファの全体サイズ(byte)
						D3DUSAGE_WRITEONLY,			//頂点バッファの使用方法
						m_FVFFormat,				//頂点のFVFフォーマット
						D3DPOOL_MANAGED,			//リソース配置場所となるメモリクラス
						&m_pVertexBuffer,			//作成された頂点バッファ
						NULL						//予約パラメータ
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, vertexNum);
			goto EXIT;
		}
	}

	m_VertexNum = vertexNum;

EXIT:;
	return result;
}

//******************************************************************************
// インデックスバッファ生成
//******************************************************************************
int DXPrimitive::CreateIndexBuffer(
		LPDIRECT3DDEVICE9 pD3DDevice,
		unsigned long indexNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	
	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pIndexBuffer != NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//インデックスバッファ生成
	if (indexNum > 0) {
		hresult = pD3DDevice->CreateIndexBuffer(
						sizeof(unsigned long) * indexNum,
												//インデックスバッファの全体サイズ(byte)
						D3DUSAGE_WRITEONLY,		//使用方法
						D3DFMT_INDEX32,			//インデックスバッファのフォーマット
						D3DPOOL_MANAGED,		//リソース配置場所となるメモリクラス
						&m_pIndexBuffer,		//作成されたインデックスバッファ
						NULL					//予約パラメータ
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, indexNum);
			goto EXIT;
		}
	}

	m_IndexNum = indexNum;

EXIT:;
	return result;
}

//******************************************************************************
// 頂点登録
//******************************************************************************
int DXPrimitive::SetAllVertex(
		LPDIRECT3DDEVICE9 pD3DDevice,
		void* pVertex
	)
{
	int result = 0;
	void* pBuf = NULL;

	//頂点バッファのロック
	result = LockVertex(&pBuf);
	if (result != 0) goto EXIT;

	//バッファに頂点データを書き込む
	try {
		memcpy(pBuf, pVertex, (m_VertexSize * m_VertexNum));
	}
	catch (...) {
		result = YN_SET_ERR("Memory access error.", (DWORD)pVertex, m_VertexNum);
		goto EXIT;
	}

EXIT:;
	UnlockVertex();
	return result;
}

//******************************************************************************
// インデックス登録
//******************************************************************************
int DXPrimitive::SetAllIndex(
		LPDIRECT3DDEVICE9 pD3DDevice,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long* pBuf = NULL;

	//頂点バッファのロック
	result = LockIndex(&pBuf);
	if (result != 0) goto EXIT;

	//バッファに頂点データを書き込む
	try {
		memcpy(pBuf, pIndex, (sizeof(unsigned long)* m_IndexNum));
	}
	catch (...) {
		result = YN_SET_ERR("Memory access error.", (DWORD)pIndex, m_IndexNum);
		goto EXIT;
	}

EXIT:;
	UnlockIndex();
	return result;
}

//******************************************************************************
// マテリアル設定
//******************************************************************************
void DXPrimitive::SetMaterial(
		D3DMATERIAL9 material
	)
{
	m_Material = material;
}

//******************************************************************************
// 移動
//******************************************************************************
void DXPrimitive::Transform(
		D3DXMATRIX worldMatrix
	)
{
	m_WorldMatrix = worldMatrix;
}

//******************************************************************************
// 描画
//******************************************************************************
int DXPrimitive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice,
		LPDIRECT3DTEXTURE9 pTexture,
		int drawPrimitiveNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long primitiveNum = 0;

	if (m_IsVertexLocked || m_IsIndexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//頂点が存在しなければ何もしない
	if (m_pVertexBuffer == NULL) goto EXIT;

	//レンダリングパイプラインに頂点バッファを設定
	hresult = pD3DDevice->SetStreamSource(
					0,					//ストリーム番号
					m_pVertexBuffer,	//ストリームデータ
					0,					//頂点データ開始オフセット位置(bytes)
					m_VertexSize		//頂点データ構造体サイズ
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, m_VertexSize);
		goto EXIT;
	}

	//レンダリングパイプラインにインデックスバッファを設定
	if (m_pIndexBuffer != NULL) {
		hresult = pD3DDevice->SetIndices(m_pIndexBuffer);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", (DWORD)hresult, (DWORD)m_pIndexBuffer);
			goto EXIT;
		}
	}

	//レンダリングパイプラインに頂点バッファFVFフォーマットを設定
	hresult = pD3DDevice->SetFVF(m_FVFFormat);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, m_FVFFormat);
		goto EXIT;
	}

	//レンダリングパイプラインにマテリアルを設定
	hresult = pD3DDevice->SetMaterial(&m_Material);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//レンダリングパイプラインにテクスチャを設定：ステージ0
	hresult = pD3DDevice->SetTexture(0, pTexture);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pTexture);
		goto EXIT;
	}

	//レンダリングパイプラインに移動マトリックスをセット
	hresult = pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//プリミティブ数取得
	result = _GetPrimitiveNum(&primitiveNum);
	if (result != 0) goto EXIT;

	//直接プリミティブ数を指定された場合はそれに従う
	if (drawPrimitiveNum > 0) {
		//バッファサイズを超えた指定はエラー
		if ((unsigned long)drawPrimitiveNum > primitiveNum) {
			result = YN_SET_ERR("Program error.", drawPrimitiveNum, primitiveNum);
			goto EXIT;
		}
		primitiveNum = drawPrimitiveNum;
	}

	//インデックス付きプリミティブの描画
	if (m_pIndexBuffer != NULL) {
		hresult = pD3DDevice->DrawIndexedPrimitive(
						m_PrimitiveType,	//プリミティブ種別
						0,					//頂点バッファ開始インデックス
						0,					//頂点バッファ最小インデックス
						m_VertexNum,		//参照する頂点の数
						0,					//インデックスバッファ開始インデックス
						primitiveNum		//プリミティブ数
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, primitiveNum);
			goto EXIT;
		}
	}
	//インデックスなしプリミティブの描画
	else {
		hresult = pD3DDevice->DrawPrimitive(
						m_PrimitiveType,	//プリミティブ種別
						0,					//頂点バッファ開始インデックス
						primitiveNum		//プリミティブ数
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, primitiveNum);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

// >>> add 20120728 yossiepon begin

//******************************************************************************
// 歌詞描画
//******************************************************************************
int DXPrimitive::DrawLyrics(
		LPDIRECT3DDEVICE9 pD3DDevice,
		LPDIRECT3DTEXTURE9 *pTextures,
		int drawPrimitiveNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long primitiveNum = 0;

	if (m_IsVertexLocked || m_IsIndexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//頂点が存在しなければ何もしない
	if (m_pVertexBuffer == NULL) goto EXIT;

	//プリミティブ数取得
	primitiveNum = drawPrimitiveNum;

	for(unsigned long i = 0; i < primitiveNum / 2; i++) {

		//レンダリングパイプラインに頂点バッファを設定
		hresult = pD3DDevice->SetStreamSource(
						0,					//ストリーム番号
						m_pVertexBuffer,	//ストリームデータ
						m_VertexSize * 6 * i,	//頂点データ開始オフセット位置(bytes)
						m_VertexSize		//頂点データ構造体サイズ
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, m_VertexSize);
			goto EXIT;
		}

		//レンダリングパイプラインに頂点バッファFVFフォーマットを設定
		hresult = pD3DDevice->SetFVF(m_FVFFormat);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, m_FVFFormat);
			goto EXIT;
		}

		//レンダリングパイプラインにマテリアルを設定
		hresult = pD3DDevice->SetMaterial(&m_Material);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}

		//レンダリングパイプラインにテクスチャを設定：ステージ0
		hresult = pD3DDevice->SetTexture(0, pTextures[i]);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pTextures[i]);
			goto EXIT;
		}

		//レンダリングパイプラインに移動マトリックスをセット
		hresult = pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}

		//インデックスなしプリミティブの描画
		hresult = pD3DDevice->DrawPrimitive(
						m_PrimitiveType,	//プリミティブ種別
						0, //i * 2,				//頂点バッファ開始インデックス
						2					//プリミティブ数
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, primitiveNum);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

// <<< add 20120728 yossiepon end

//******************************************************************************
// 頂点バッファロック
//******************************************************************************
int DXPrimitive::LockVertex(
		void** pPtrVertex,
		unsigned long offset,	//省略時はゼロ
		unsigned long size		//省略時はゼロ
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (m_IsVertexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((m_VertexSize * m_VertexNum) < (offset + size)) {
		result = YN_SET_ERR("Program error.", offset, size);
		goto EXIT;
	}

	//頂点バッファのロックとバッファメモリポインタ取得
	if (m_pVertexBuffer != NULL) {
		hresult = m_pVertexBuffer->Lock(
						offset,		//ロックする頂点のオフセット
						size,		//ロックする頂点のサイズ(byte)
						pPtrVertex,	//バッファメモリポインタ
						0			//ロッキングフラグ
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pPtrVertex);
			goto EXIT;
		}
	}

	m_IsVertexLocked = true;

EXIT:;
	return result;
}

//******************************************************************************
// 頂点バッファロック解除
//******************************************************************************
int DXPrimitive::UnlockVertex()
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	
	if (m_IsVertexLocked) {
		if (m_pVertexBuffer != NULL) {
			hresult = m_pVertexBuffer->Unlock();
			if (FAILED(hresult)) {
				result = YN_SET_ERR("DirectX API error.", hresult, 0);
				goto EXIT;
			}
		}
		m_IsVertexLocked = false;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// インデックスバッファロック
//******************************************************************************
int DXPrimitive::LockIndex(
		unsigned long** pPtrIndex,
		unsigned long offset,	//省略時はゼロ
		unsigned long size		//省略時はゼロ
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (m_IsIndexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((sizeof(unsigned long) * m_IndexNum) < (offset + size)) {
		result = YN_SET_ERR("Program error.", offset, size);
		goto EXIT;
	}

	//インデックスバッファのロックとバッファメモリポインタ取得
	if (m_pIndexBuffer != NULL) {
		hresult = m_pIndexBuffer->Lock(
						offset,		//ロックするインデックスのオフセット(byte)
						size,		//ロックするインデックスのサイズ(byte)
						(void**)pPtrIndex,	//バッファメモリポインタ
						0			//ロッキングフラグ
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pPtrIndex);
			goto EXIT;
		}
	}

	m_IsIndexLocked = true;

EXIT:;
	return result;
}

//******************************************************************************
// インデックスバッファロック解除
//******************************************************************************
int DXPrimitive::UnlockIndex()
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (m_IsIndexLocked) {
		if (m_pIndexBuffer != NULL) {
			hresult = m_pIndexBuffer->Unlock();
			if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
				goto EXIT;
			}
		}
		m_IsIndexLocked = false;
	}

EXIT:;
	return result;
}

//******************************************************************************
// プリミティブ数取得
//******************************************************************************
int DXPrimitive::_GetPrimitiveNum(
		unsigned long* pNum
	)
{
	int result = 0;
	unsigned long vertexNum = 0;

	vertexNum = m_VertexNum;
	if (m_pIndexBuffer != NULL) {
		vertexNum = m_IndexNum;
	}

	if (vertexNum == 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	switch (m_PrimitiveType) {
		case D3DPT_POINTLIST:
			*pNum = vertexNum;
			break;
			
		case D3DPT_LINELIST:
			if ((vertexNum % 2) != 0) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum / 2;
			break;
			
		case D3DPT_LINESTRIP:
			if (vertexNum < 2) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum - 1;
			break;
			
		case D3DPT_TRIANGLELIST:
			if ((vertexNum % 3) != 0) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum / 3;
			break;
			
		case D3DPT_TRIANGLESTRIP:
			if (vertexNum < 3) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum - 2;
			break;
			
		case D3DPT_TRIANGLEFAN:
			if (vertexNum < 3) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum - 2;
			break;
	}

EXIT:;
	return result;
}

//******************************************************************************
// デフォルトマテリアル
//******************************************************************************
void DXPrimitive::_GetDefaultMaterial(
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
	pMaterial->Specular.r = 0.0f;
	pMaterial->Specular.g = 0.0f;
	pMaterial->Specular.b = 0.0f;
	pMaterial->Specular.a = 0.0f;
	//鏡面反射光の鮮明度
	pMaterial->Power = 0.0f;
	//発光色
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}


