//******************************************************************************
//
// MIDITrail / MTGridBox11
//
// DX11 grid box renderer.
// Draws wireframe grid and bar lines using line primitives.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridBox11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ / デストラクタ
//******************************************************************************
MTGridBox11::MTGridBox11()
{
	m_isEnable = true;
	m_isReady = false;
}

MTGridBox11::~MTGridBox11()
{
	Release();
}

//******************************************************************************
// 生成
//******************************************************************************
int MTGridBox11::Create(
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

	result = _CreateVertices(pDevice, pContext, pSeqData);
	if (result != 0) goto EXIT;

	m_Primitive.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_Primitive.SetLightEnable(false);
	m_Primitive.SetDepthWrite(false);

	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTGridBox11::Release()
{
	m_Primitive.Release();
	m_isReady = false;
}

//******************************************************************************
// 更新
//******************************************************************************
void MTGridBox11::Transform(float rollAngle)
{
	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
	Matrix world = Matrix::CreateRotationX(XMConvertToRadians(rollAngle))
	             * Matrix::CreateTranslation(moveVec);
	m_Primitive.SetWorldMatrix(world);
}

//******************************************************************************
// 描画
//******************************************************************************
int MTGridBox11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir,
		float rollAngle
	)
{
	if (!m_isEnable || !m_isReady) return 0;
	return m_Primitive.Draw(pContext, viewProj, lightDir);
}

//******************************************************************************
// 頂点生成
//******************************************************************************
int MTGridBox11::_CreateVertices(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMBarList barList;
	SMPortList portList;
	unsigned long totalTickTime = 0;
	unsigned long barNum = 0;
	unsigned long portNum = 0;

	totalTickTime = pSeqData->GetTotalTickTime();
	result = pSeqData->GetBarList(&barList);
	if (result != 0) goto EXIT;
	result = pSeqData->GetPortList(&portList);
	if (result != 0) goto EXIT;

	barNum = barList.GetSize();
	portNum = portList.GetSize();

	// 頂点数: 直方体8頂点 + 小節線(2*barNum) + ポート分割線(4*(portNum-1))
	// インデックス数: 直方体12辺*2 + 小節線(2*barNum) + ポート分割線(4*(portNum-1))
	{
		unsigned long vertexNum = 8 + (2 * barNum) + (4 * (portNum > 0 ? portNum - 1 : 0));
		unsigned long indexNum  = 24 + (2 * barNum) + (4 * (portNum > 0 ? portNum - 1 : 0));

		result = m_Primitive.CreateVertexBuffer(pDevice, vertexNum);
		if (result != 0) goto EXIT;
		result = m_Primitive.CreateIndexBuffer(pDevice, indexNum);
		if (result != 0) goto EXIT;

		DXPRIMITIVE11_VERTEX* pVertex = NULL;
		unsigned long* pIndex = NULL;

		result = m_Primitive.LockVertex(pContext, &pVertex);
		if (result != 0) goto EXIT;
		result = m_Primitive.LockIndex(pContext, &pIndex);
		if (result != 0) goto EXIT;

		// グリッド色
		Color gridColor = m_NoteDesign.GetGridLineColor();
		unsigned char cr = (unsigned char)(gridColor.R() * 255.0f);
		unsigned char cg = (unsigned char)(gridColor.G() * 255.0f);
		unsigned char cb = (unsigned char)(gridColor.B() * 255.0f);
		unsigned char ca = (unsigned char)(gridColor.A() * 255.0f);
		DWORD color = (ca << 24) | (cr << 16) | (cg << 8) | cb;

		// 直方体の8頂点
		unsigned char lastPortNo = 0;
		portList.GetPort(portList.GetSize() - 1, &lastPortNo);

		Vector3 startFirst[4], endFirst[4];
		Vector3 startFinal[4], endFinal[4];

		m_NoteDesign.GetGridBoxVirtexPos(0, 0, &startFirst[0], &startFirst[1], &startFirst[2], &startFirst[3]);
		m_NoteDesign.GetGridBoxVirtexPos(totalTickTime, 0, &endFirst[0], &endFirst[1], &endFirst[2], &endFirst[3]);
		m_NoteDesign.GetGridBoxVirtexPos(0, lastPortNo, &startFinal[0], &startFinal[1], &startFinal[2], &startFinal[3]);
		m_NoteDesign.GetGridBoxVirtexPos(totalTickTime, lastPortNo, &endFinal[0], &endFinal[1], &endFinal[2], &endFinal[3]);

		// 直方体頂点 (DX9 と同じ配置)
		//     1+----+3        y x
		//    / 上 /           |/
		//  0+----+2         z--+0
		//    7+----+5
		//    / 下 /
		//  6+----+4

		auto setVtx = [&](unsigned long i, const Vector3& pos) {
			pVertex[i].pos[0] = pos.x;
			pVertex[i].pos[1] = pos.y;
			pVertex[i].pos[2] = pos.z;
			pVertex[i].normal[0] = 0.0f;
			pVertex[i].normal[1] = 1.0f;
			pVertex[i].normal[2] = 0.0f;
			pVertex[i].color = color;
			pVertex[i].uv[0] = 0.0f;
			pVertex[i].uv[1] = 0.0f;
		};

		setVtx(0, Vector3(endFinal[0].x,   startFinal[0].y, startFinal[0].z));
		setVtx(1, Vector3(startFinal[0].x,  startFinal[0].y, startFinal[0].z));
		setVtx(2, Vector3(endFirst[1].x,    startFirst[1].y, startFirst[1].z));
		setVtx(3, Vector3(startFirst[1].x,  startFirst[1].y, startFirst[1].z));
		setVtx(4, Vector3(endFirst[3].x,    startFirst[3].y, startFirst[3].z));
		setVtx(5, Vector3(startFirst[3].x,  startFirst[3].y, startFirst[3].z));
		setVtx(6, Vector3(endFinal[2].x,    startFinal[2].y, startFinal[2].z));
		setVtx(7, Vector3(startFinal[2].x,  startFinal[2].y, startFinal[2].z));

		// 12辺のインデックス
		unsigned long edges[] = {
			0,1, 2,3, 4,5, 6,7,  // 横4辺
			0,2, 1,3, 4,6, 5,7,  // 縦4辺
			0,6, 1,7, 2,4, 3,5   // 奥行き4辺
		};
		for (unsigned long i = 0; i < 24; i++) {
			pIndex[i] = edges[i];
		}

		// 小節線
		unsigned long vi = 8;
		unsigned long ii = 24;
		for (unsigned long bar = 0; bar < barNum; bar++) {
			unsigned long barTickTime = 0;
			barList.GetBar(bar, &barTickTime);

			Vector3 barStart[4], barEnd[4];
			m_NoteDesign.GetGridBoxVirtexPos(barTickTime, 0, &barStart[0], &barStart[1], &barStart[2], &barStart[3]);
			m_NoteDesign.GetGridBoxVirtexPos(barTickTime, lastPortNo, &barEnd[0], &barEnd[1], &barEnd[2], &barEnd[3]);

			// 上辺に小節線（左端から右端）
			setVtx(vi,     Vector3(barEnd[0].x,  barEnd[0].y,  barEnd[0].z));
			setVtx(vi + 1, Vector3(barStart[1].x, barStart[1].y, barStart[1].z));
			pIndex[ii]     = vi;
			pIndex[ii + 1] = vi + 1;

			vi += 2;
			ii += 2;
		}

		// ポート分割線
		for (unsigned long p = 1; p < portNum; p++) {
			unsigned char portNo = 0;
			portList.GetPort(p, &portNo);

			Vector3 ps[4], pe[4];
			m_NoteDesign.GetGridBoxVirtexPos(0, portNo, &ps[0], &ps[1], &ps[2], &ps[3]);
			m_NoteDesign.GetGridBoxVirtexPos(totalTickTime, portNo, &pe[0], &pe[1], &pe[2], &pe[3]);

			// 上辺と下辺にポート分割線
			setVtx(vi,     Vector3(pe[0].x,  ps[2].y, ps[2].z));
			setVtx(vi + 1, Vector3(ps[0].x,  ps[2].y, ps[2].z));
			setVtx(vi + 2, Vector3(pe[1].x,  ps[3].y, ps[3].z));
			setVtx(vi + 3, Vector3(ps[1].x,  ps[3].y, ps[3].z));
			pIndex[ii]     = vi;
			pIndex[ii + 1] = vi + 1;
			pIndex[ii + 2] = vi + 2;
			pIndex[ii + 3] = vi + 3;

			vi += 4;
			ii += 4;
		}

		m_Primitive.UnlockVertex(pContext);
		m_Primitive.UnlockIndex(pContext);
	}

EXIT:;
	return result;
}
