//******************************************************************************
//
// MIDITrail / DXPrimitive
//
// プリミティブ描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DrawPrimitive, DrawIndexedPrimitive の操作をラップするクラス。
// インデックスバッファを作成しなければDrawPrimitive
// インデックスバッファを作成するとDrawIndexedPrimitive
// が使用される。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// プリミティブ描画クラス
//******************************************************************************
class DXPrimitive
{
public:

	//コンストラクタ／デストラクタ
	DXPrimitive(void);
	virtual ~DXPrimitive(void);

	//リソース解放
	void Release();
	
	//初期化
	int Initialize(
			unsigned long vertexSize,
			unsigned long fvfFormat,
			D3DPRIMITIVETYPE type
		);

	//頂点バッファ／インデックスバッファの生成
	int CreateVertexBuffer(LPDIRECT3DDEVICE9 pD3DDevice, unsigned long vertexNum);
	int CreateIndexBuffer(LPDIRECT3DDEVICE9 pD3DDevice, unsigned long indexNum);

	//頂点データ／インデックスデータ登録
	//  バッファのロック／アンロック制御は自動的に行われる
	//  本メソッドに指定したデータは利用者側が破棄する
	int SetAllVertex(LPDIRECT3DDEVICE9 pD3DDevice, void* pVertex);
	int SetAllIndex(LPDIRECT3DDEVICE9 pD3DDevice, unsigned long* pIndex);

	//マテリアル登録（省略可）
	void SetMaterial(D3DMATERIAL9 material);

	//移動制御
	void Transform(D3DXMATRIX worldMatrix);

	//描画
	int Draw(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPDIRECT3DTEXTURE9 pTexture = NULL,
			int drawPrimitiveNum = -1
		);

// >>> add 20180413 yossiepon begin
	int Draw(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPDIRECT3DINDEXBUFFER9 pIndexBuffer,
			LPDIRECT3DTEXTURE9 pTexture = NULL,
			int drawPrimitiveNum = -1
		);
// <<< add 20180413 yossiepon end

// >>> add 20120728 yossiepon begin

	int DrawLyrics(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPDIRECT3DTEXTURE9 *pTextures = NULL,
			int drawPrimitiveNum = -1
		);

// <<< add 20120728 yossiepon end

	//頂点バッファ／インデックスバッファのロック制御
	//  バッファの内容を書き換えるにはロックしてバッファのポインタを取得する
	//  バッファの内容を書き終えたらアンロックする
	int LockVertex(void** pPtrVertex, unsigned long offset = 0, unsigned long size = 0);
	int UnlockVertex();
	int LockIndex(unsigned long** pPtrIndex, unsigned long offset = 0, unsigned long size = 0);
	int UnlockIndex();

private:

	//頂点情報
	unsigned long m_VertexSize;
	unsigned long m_FVFFormat;
	D3DPRIMITIVETYPE m_PrimitiveType;
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	unsigned long m_VertexNum;
	bool m_IsVertexLocked;
	
	//インデックス情報
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
	unsigned long m_IndexNum;
	bool m_IsIndexLocked;
	
	//描画情報
	D3DMATERIAL9 m_Material;
	D3DXMATRIX m_WorldMatrix;

	int _GetPrimitiveNum(unsigned long* pNum);
	void _GetDefaultMaterial(D3DMATERIAL9* pMaterial);

};

