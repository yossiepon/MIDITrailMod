//******************************************************************************
//
// MIDITrail / MTStars11
//
// DX11 star particle renderer.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTStars11.h"
#include <cstdlib>
#include <cmath>

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ / デストラクタ
//******************************************************************************
MTStars11::MTStars11()
{
	m_NumOfStars = 2000;
	m_isReady = false;
}

MTStars11::~MTStars11()
{
	Release();
}

//******************************************************************************
// 生成
//******************************************************************************
int MTStars11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName
	)
{
	int result = 0;

	Release();

	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	// 頂点バッファ生成（ポイントリスト）
	result = m_Primitive.CreateVertexBuffer(pDevice, m_NumOfStars);
	if (result != 0) goto EXIT;

	m_Primitive.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_Primitive.SetLightEnable(false);

	// 頂点データ書き込み
	result = _CreateVertices(pContext);
	if (result != 0) goto EXIT;

	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTStars11::Release()
{
	m_Primitive.Release();
	m_isReady = false;
}

//******************************************************************************
// 更新：カメラ位置に追従（星は無限遠を擬似的に表現）
//******************************************************************************
int MTStars11::Update(const MTSceneUpdateContext& ctx)
{
	Matrix world = Matrix::CreateTranslation(ctx.camPos);
	m_Primitive.SetWorldMatrix(world);
	return 0;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTStars11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle
	)
{
	if (!m_isEnable || !m_isReady) return 0;

	Vector4 lightDir(0.0f, 0.0f, 0.0f, 0.0f);
	return m_Primitive.Draw(pContext, viewProj, lightDir);
}

//******************************************************************************
// 頂点生成：球面上に一様分布
//******************************************************************************
int MTStars11::_CreateVertices(ID3D11DeviceContext* pContext)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;

	for (int i = 0; i < m_NumOfStars; i++) {
		float r   = 500.0f;
		float phi = ((float)rand() / RAND_MAX) * 2.0f * XM_PI;
		float y   = ((float)rand() / RAND_MAX) * 2.0f * r - r;
		float xzr = sqrtf(r * r - y * y);
		float x   = xzr * cosf(phi);
		float z   = xzr * sinf(phi);

		// グレースケールの明るさ
		float brightness = ((float)rand() / RAND_MAX);

		pVertex[i].pos[0] = x;
		pVertex[i].pos[1] = y;
		pVertex[i].pos[2] = z;
		pVertex[i].normal[0] = 0.0f;
		pVertex[i].normal[1] = 1.0f;
		pVertex[i].normal[2] = 0.0f;
		pVertex[i].uv[0] = 0.0f;
		pVertex[i].uv[1] = 0.0f;

		unsigned char c = (unsigned char)(brightness * 255.0f);
		pVertex[i].color = 0xFF000000 | (c << 16) | (c << 8) | c;
	}

	m_Primitive.UnlockVertex(pContext);

EXIT:;
	return result;
}

//******************************************************************************
// 設定読み込み
//******************************************************************************
int MTStars11::_LoadConfFile(const TCHAR* pSceneName)
{
	int result = 0;
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Stars"));
	if (result != 0) goto EXIT;
	confFile.GetInt(_T("NumberOfStars"), &m_NumOfStars, 2000);

EXIT:;
	return result;
}
