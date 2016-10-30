//******************************************************************************
//
// MIDITrail / DXMesh
//
// メッシュ描画クラス
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXMesh.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
DXMesh::DXMesh(void)
{
	m_pMesh = NULL;
	m_NumMaterials = 0;
	m_pMeshMaterials = NULL;
	m_pMeshTextures = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DXMesh::~DXMesh(void)
{
	Release();
}

//******************************************************************************
// 解放
//******************************************************************************
void DXMesh::Release()
{
	unsigned long i = 0;

	for (i = 0; i < m_NumMaterials; i++) {
		if (m_pMeshTextures[i] != NULL) {
			(m_pMeshTextures[i])->Release();
			m_pMeshTextures[i] = NULL;
		}
	}
	delete [] m_pMeshMaterials;
	delete [] m_pMeshTextures;
	m_pMeshMaterials = NULL;
	m_pMeshTextures = NULL;
	m_NumMaterials = 0;

	if (m_pMesh != NULL) {
		m_pMesh->Release();
		m_pMesh = NULL;
	}

	return;
}

//******************************************************************************
// 初期化
//******************************************************************************
int DXMesh::Initialize(
		LPDIRECT3DDEVICE9 pD3DDevice,
		TCHAR* pMeshFilePath
	)
{
	int result = 0;

	//ファイル未指定なら何もしない
	if (_tcslen(pMeshFilePath) == 0) goto EXIT;

	//メッシュファイル読み込み
	result = _LoadMeshFile(pD3DDevice, pMeshFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メッシュファイル読み込み
//******************************************************************************
int DXMesh::_LoadMeshFile(
		LPDIRECT3DDEVICE9 pD3DDevice,
		TCHAR* pMeshFilePath
	)
{
	int result = 0;
	unsigned long i = 0;
	HRESULT hresult = D3D_OK;
	LPD3DXBUFFER pMaterialBuffer = NULL;
	TCHAR textureFilePath[_MAX_PATH];
	D3DXMATERIAL* pMaterials = NULL;

	//メッシュファイル読み込み
	hresult = D3DXLoadMeshFromX(
					pMeshFilePath,		//メッシュファイルパス
					D3DXMESH_MANAGED,	//メッシュ作成オプション
					pD3DDevice,			//デバイスオブジェクト
					NULL,				//隣接性データ
					&pMaterialBuffer,	//マテリアル
					NULL,				//エフェクトインスタンス
					&m_NumMaterials,	//マテリアル数
					&m_pMesh			//メッシュ
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//マテリアル先頭位置
	pMaterials = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();

	//配列バッファ生成
	try {
		m_pMeshMaterials = new D3DMATERIAL9[m_NumMaterials];
		m_pMeshTextures = new LPDIRECT3DTEXTURE9[m_NumMaterials];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//バッファクリア
	for (i = 0; i < m_NumMaterials; i++) {
		m_pMeshTextures[i] = NULL;
	}

	//テクスチャファイル読み込み
	for (i = 0; i < m_NumMaterials; i++) {
		m_pMeshMaterials[i] = pMaterials[i].MatD3D;
		//m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;
		
		if (pMaterials[i].pTextureFilename == NULL) continue;
		
		//テクスチャファイルパス作成
		result = _GetTextureFilePath(
						pMeshFilePath,
						pMaterials[i].pTextureFilename,
						textureFilePath,
						_MAX_PATH
					);
		if (result != 0) goto EXIT;
		
		//テクスチャファイル読み込み
		hresult = D3DXCreateTextureFromFile(
						pD3DDevice,
						textureFilePath,
						&(m_pMeshTextures[i])
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	if (pMaterialBuffer != NULL) {
		pMaterialBuffer->Release();
	}
	return result;
}

//******************************************************************************
// テクスチャファイルパス取得
//******************************************************************************
int DXMesh::_GetTextureFilePath(
		TCHAR* pMeshFilePath,
		TCHAR* pTextureFileName,
		TCHAR* pBuf,
		unsigned long bufSize
	)
{
	int result = 0;
	DWORD apiresult = 0;
	errno_t eresult = 0;
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	//パス要素の分割
	eresult = _tsplitpath_s(
					pMeshFilePath,	//メッシュファイルパス
					drive,			//ドライブ文字列バッファ
					_MAX_DRIVE,		//バッファサイズ
					dir,			//ディレクトリ文字列バッファ
					_MAX_DIR,		//バッファサイズ
					fname,			//ファイル名文字列バッファ
					_MAX_FNAME,		//バッファサイズ
					ext,			//拡張子文字列バッファ
					_MAX_EXT		//バッファサイズ
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//パス作成
	eresult = _tmakepath_s(
					pBuf,			//パス格納先バッファ
					bufSize,		//バッファサイズ
					drive,			//ドライブ文字列
					dir,			//ディレクトリ文字列
					pTextureFileName,	//ファイル名文字列
					NULL			//拡張子文字列
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
void DXMesh::Transform(
		D3DXMATRIX worldMatrix
	)
{
	m_WorldMatrix = worldMatrix;
}

//******************************************************************************
// 描画
//******************************************************************************
int DXMesh::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long i = 0;

	//メッシュを読み込んでいなければ何もしない
	if (m_pMesh == NULL) goto EXIT;

	//移動マトリクスをセット
	hresult = pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

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

	//メッシュ描画
	for (i = 0; i < m_NumMaterials; i++) {
		//マテリアル設定
		hresult = pD3DDevice->SetMaterial(&(m_pMeshMaterials[i]));
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
		
		//テクスチャ設定：ステージ0
		hresult = pD3DDevice->SetTexture(0, m_pMeshTextures[i]);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
		
		//メッシュサブセット描画
		hresult = m_pMesh->DrawSubset(i);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}


