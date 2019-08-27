//******************************************************************************
//
// MIDITrail / MTStars
//
// 星描画クラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTStars.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTStars::MTStars(void)
{
	m_pTexture = NULL;
	m_NumOfStars = 2000;
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTStars::~MTStars(void)
{
	Release();
}

//******************************************************************************
// 星生成
//******************************************************************************
int MTStars::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		DXDirLight* pLight
	)
{
	int result = 0;
	MTSTARS_VERTEX* pVertex = NULL;
	D3DMATERIAL9 material;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//パラメータ設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//プリミティブ初期化
	result = m_Primitive.Initialize(
					sizeof(MTSTARS_VERTEX),	//頂点サイズ
					_GetFVFFormat(),		//頂点FVFフォーマット
					D3DPT_POINTLIST			//プリミティブ種別
				);
	if (result != 0) goto EXIT;

	//頂点バッファ生成
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, m_NumOfStars);
	if (result != 0) goto EXIT;

	//バッファのロック
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	//バッファに頂点を書き込む
	result = _CreateVertexOfStars(pVertex, pLight);
	if (result != 0) goto EXIT;

	//バッファのロック解除
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

	//マテリアル作成
	_MakeMaterial(&material);
	m_Primitive.SetMaterial(material);

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTStars::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector
	)
{
	int result = 0;
	D3DXMATRIX worldMatrix;
	
	//現実世界の星は非常に遠くに存在するので
	//視線方向を変えずにカメラを移動しても星は動かないように見える
	//これを擬似的に再現するためカメラに合わせて星も同じだけ移動させる
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixTranslation(&worldMatrix, camVector.x, camVector.y, camVector.z);
	m_Primitive.Transform(worldMatrix);
	
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTStars::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	result = m_Primitive.Draw(pD3DDevice, m_pTexture);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTStars::Release()
{
	m_Primitive.Release();
	
	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

//******************************************************************************
// 星頂点生成
//******************************************************************************
int MTStars::_CreateVertexOfStars(
		MTSTARS_VERTEX* pVertex,
		DXDirLight* pLight
	)
{
	int result = 0;
	float r = 0.0f;
	float phi = 0.0f;
	//float theta = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float cr = 0.0f;
	float cg = 0.0f;
	float cb = 0.0f;
	int i = 0;
	D3DXVECTOR3 normal;

	//ライトの方向によって星の色が変わることを防ぐため
	//法線ベクトルをライトの方向ベクトルの逆向きにする
	normal = -(pLight->GetDirection());

	//頂点座標
	for (i = 0; i < m_NumOfStars; i++) {

		//極座標(theta,phi)に乱数を適用すると北極と南極で星の分布密度が高くなってしまう
		//
		//極座標：球面上に星を配置
		//r     = 500.0f;
		//phi   = ((float)rand() / RAND_MAX) * 360.0f;
		//theta = ((float)rand() / RAND_MAX) * 180.0f;
		//直行座標に変換
		//x = r * sin(D3DXToRadian(theta)) * cos(D3DXToRadian(phi));
		//y = r * cos(D3DXToRadian(theta));
		//z = r * sin(D3DXToRadian(theta)) * sin(D3DXToRadian(phi));

		//球面上に星を一様分布させるため(y, phi)に乱数を適用する
		r   = 500.0f;
		phi = ((float)rand() / RAND_MAX) * 2.0f * 3.1415926f;
		y   = ((float)rand() / RAND_MAX) * 2.0f * r - r;
		x = sqrt((r * r) - (y * y)) * cos(phi);
		z = sqrt((r * r) - (y * y)) * sin(phi);

		//色：グレースケールにする場合
		cr = ((float)rand() / RAND_MAX);
		cg = cr;
		cb = cr;
		//色：カラフルにする場合
		//cr = ((float)rand() / RAND_MAX);
		//cg = ((float)rand() / RAND_MAX);
		//cb = ((float)rand() / RAND_MAX);

		//頂点座標
		pVertex[i].p = D3DXVECTOR3(x, y, z);
		//法線
		pVertex[i].n = normal;
		//ディフューズ色
		pVertex[i].c = D3DXCOLOR(cr, cg, cb, 1.0f);
	}

	return result;
}

//******************************************************************************
// マテリアル作成
//******************************************************************************
void MTStars::_MakeMaterial(
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
	pMaterial->Power = 100.0f;
	//発光色
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTStars::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//星の数
	result = confFile.SetCurSection(_T("Stars"));
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("NumberOfStars"), &m_NumOfStars, 2000);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTStars::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


