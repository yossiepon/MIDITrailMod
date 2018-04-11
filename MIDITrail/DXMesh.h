//******************************************************************************
//
// MIDITrail / DXMesh
//
// メッシュ描画クラス
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// メッシュ描画クラス
//******************************************************************************
class DXMesh
{
public:

	//コンストラクタ／デストラクタ
	DXMesh(void);
	virtual ~DXMesh(void);

	//リソース解放
	void Release();

	//初期化
	int Initialize(LPDIRECT3DDEVICE9 pD3DDevice, TCHAR* pMeshFilePath);

	//移動制御
	void Transform(D3DXMATRIX worldMatrix);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	D3DXMATRIX m_WorldMatrix;
	LPD3DXMESH m_pMesh;
	unsigned long m_NumMaterials;
	D3DMATERIAL9* m_pMeshMaterials;
	LPDIRECT3DTEXTURE9* m_pMeshTextures;

	int _LoadMeshFile(
			LPDIRECT3DDEVICE9 pD3DDevice,
			TCHAR* pMeshFilePath
		);

	int _GetTextureFilePath(
			TCHAR* pMeshFilePath,
			TCHAR* pTextureFileName,
			TCHAR* pBuf,
			unsigned long bufSize
		);

};


