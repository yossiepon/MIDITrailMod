//******************************************************************************
//
// MIDITrail / MTTimeIndicator11
//
// DX11 time indicator renderer.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTTimeIndicator11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ / デストラクタ
//******************************************************************************
MTTimeIndicator11::MTTimeIndicator11()
{
	m_CurPos = 0.0f;
	m_CurTickTime = 0;
	m_isEnableLine = false;
	m_isEnable = true;
	m_isReady = false;
}

MTTimeIndicator11::~MTTimeIndicator11()
{
	Release();
}

//******************************************************************************
// 生成
//******************************************************************************
int MTTimeIndicator11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	if (pSeqData == NULL) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = _CreatePrimitive(pDevice, pContext);
	if (result != 0) goto EXIT;

	result = _CreatePrimitiveLine(pDevice, pContext);
	if (result != 0) goto EXIT;

	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTTimeIndicator11::Release()
{
	m_Primitive.Release();
	m_PrimitiveLine.Release();
	m_isReady = false;
}

//******************************************************************************
// 面プリミティブ生成
//******************************************************************************
int MTTimeIndicator11::_CreatePrimitive(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	// DX9 版は TRIANGLESTRIP(4頂点4インデックス) だが
	// DXPrimitive11 は TRIANGLELIST なので 4 頂点 6 インデックスで 2 三角形
	result = m_Primitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_Primitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;

	m_Primitive.SetLightEnable(false);

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	// 再生面の頂点座標取得
	{
		Vector3 vectorLU, vectorRU, vectorLD, vectorRD;
		m_NoteDesign.GetPlaybackSectionVirtexPos(
				0, &vectorLU, &vectorRU, &vectorLD, &vectorRD);

		//  0+----+1
		//   |    |
		//   |    |
		//  2+----+3

		auto setVtx = [&](unsigned long i, const Vector3& pos) {
			pVertex[i].pos[0] = pos.x;
			pVertex[i].pos[1] = pos.y;
			pVertex[i].pos[2] = pos.z;
			pVertex[i].normal[0] = -1.0f;
			pVertex[i].normal[1] = 0.0f;
			pVertex[i].normal[2] = 0.0f;
			pVertex[i].uv[0] = 0.0f;
			pVertex[i].uv[1] = 0.0f;
		};

		Color sectionColor = m_NoteDesign.GetPlaybackSectionColor();
		unsigned char cr = (unsigned char)(sectionColor.R() * 255.0f);
		unsigned char cg = (unsigned char)(sectionColor.G() * 255.0f);
		unsigned char cb = (unsigned char)(sectionColor.B() * 255.0f);
		unsigned char ca = (unsigned char)(sectionColor.A() * 255.0f);
		DWORD color = (ca << 24) | (cr << 16) | (cg << 8) | cb;

		setVtx(0, vectorLU);
		setVtx(1, vectorRU);
		setVtx(2, vectorLD);
		setVtx(3, vectorRD);

		for (int i = 0; i < 4; i++) {
			pVertex[i].color = color;
		}

		// 再生面の幅がゼロに近い場合はラインを描画する
		float delta = vectorLU.z - vectorRU.z;
		if (delta < 0) delta = -delta;
		if (delta < 0.1f) {
			m_isEnableLine = true;
		}

		// TRIANGLELIST (2 三角形)
		pIndex[0] = 0; pIndex[1] = 1; pIndex[2] = 2;
		pIndex[3] = 2; pIndex[4] = 1; pIndex[5] = 3;
	}

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

EXIT:;
	return result;
}

//******************************************************************************
// ラインプリミティブ生成
//******************************************************************************
int MTTimeIndicator11::_CreatePrimitiveLine(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;

	result = m_PrimitiveLine.CreateVertexBuffer(pDevice, 2);
	if (result != 0) goto EXIT;

	m_PrimitiveLine.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_PrimitiveLine.SetLightEnable(false);

	result = m_PrimitiveLine.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;

	{
		Vector3 vectorLU, vectorRU, vectorLD, vectorRD;
		m_NoteDesign.GetPlaybackSectionVirtexPos(
				0, &vectorLU, &vectorRU, &vectorLD, &vectorRD);

		Color lineColor = m_NoteDesign.GetGridLineColor();
		unsigned char cr = (unsigned char)(lineColor.R() * 255.0f);
		unsigned char cg = (unsigned char)(lineColor.G() * 255.0f);
		unsigned char cb = (unsigned char)(lineColor.B() * 255.0f);
		unsigned char ca = (unsigned char)(lineColor.A() * 255.0f);
		DWORD color = (ca << 24) | (cr << 16) | (cg << 8) | cb;

		pVertex[0].pos[0] = vectorLU.x;
		pVertex[0].pos[1] = vectorLU.y;
		pVertex[0].pos[2] = vectorLU.z;
		pVertex[0].normal[0] = 0.0f;
		pVertex[0].normal[1] = 0.0f;
		pVertex[0].normal[2] = -1.0f;
		pVertex[0].color = color;
		pVertex[0].uv[0] = 0.0f;
		pVertex[0].uv[1] = 0.0f;

		pVertex[1].pos[0] = vectorLD.x;
		pVertex[1].pos[1] = vectorLD.y;
		pVertex[1].pos[2] = vectorLD.z;
		pVertex[1].normal[0] = 0.0f;
		pVertex[1].normal[1] = 0.0f;
		pVertex[1].normal[2] = -1.0f;
		pVertex[1].color = color;
		pVertex[1].uv[0] = 0.0f;
		pVertex[1].uv[1] = 0.0f;
	}

	m_PrimitiveLine.UnlockVertex(pContext);

EXIT:;
	return result;
}

//******************************************************************************
// 更新
//******************************************************************************
void MTTimeIndicator11::Transform(float rollAngle)
{
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
	Matrix world = Matrix::CreateRotationX(XMConvertToRadians(rollAngle))
	             * Matrix::CreateTranslation(moveVec.x + m_CurPos, moveVec.y, moveVec.z);

	m_Primitive.SetWorldMatrix(world);
	m_PrimitiveLine.SetWorldMatrix(world);
}

//******************************************************************************
// 描画
//******************************************************************************
int MTTimeIndicator11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir,
		float rollAngle
	)
{
	if (!m_isEnable || !m_isReady) return 0;

	if (m_isEnableLine) {
		return m_PrimitiveLine.Draw(pContext, viewProj, lightDir);
	}
	else {
		return m_Primitive.Draw(pContext, viewProj, lightDir);
	}
}

//******************************************************************************
// チックタイム設定
//******************************************************************************
void MTTimeIndicator11::SetCurTickTime(unsigned long curTickTime)
{
	m_CurTickTime = curTickTime;
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
}

//******************************************************************************
// リセット
//******************************************************************************
void MTTimeIndicator11::Reset()
{
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
}

//******************************************************************************
// 現在位置取得
//******************************************************************************
float MTTimeIndicator11::GetPos() const
{
	return m_CurPos;
}

//******************************************************************************
// 移動ベクトル取得
//******************************************************************************
Vector3 MTTimeIndicator11::GetMoveVector() const
{
	return Vector3(m_CurPos, 0.0f, 0.0f);
}
