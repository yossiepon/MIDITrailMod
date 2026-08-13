//******************************************************************************
//
// MIDITrail / MTGridBoxLive11
//
// Grid box renderer (Live).
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridBoxLive11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Create
//******************************************************************************
int MTGridBoxLive11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName
	)
{
	int result = 0;

	Release();

	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;

	{
		Color lineColor = m_NoteDesign.GetGridLineColor();
		if (lineColor.A() < 0.01f) {
			m_isVisible = false;
		}
	}

	result = _CreateVertices(pDevice, pContext);
	if (result != 0) goto EXIT;

	_SetupPrimitive();
	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// Create vertices (8-vertex wireframe cuboid, DX9 MTGridBoxLive pattern)
//******************************************************************************
int MTGridBoxLive11::_CreateVertices(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	result = m_Primitive.CreateVertexBuffer(pDevice, 8);
	if (result != 0) goto EXIT;
	result = m_Primitive.CreateIndexBuffer(pDevice, 24);
	if (result != 0) goto EXIT;

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	{
		Vector3 startCorners[4], endCorners[4];

		m_NoteDesign.GetGridBoxVirtexPosLive(
			0, 0,
			&startCorners[0], &startCorners[1], &startCorners[2], &startCorners[3]);

		unsigned long displayDuration = m_NoteDesign.GetLiveMonitorDisplayDuration();
		m_NoteDesign.GetGridBoxVirtexPosLive(
			displayDuration, 0,
			&endCorners[0], &endCorners[1], &endCorners[2], &endCorners[3]);

		//     +   1+----+3   +
		//    /|   / top /    /|         y x
		//   + | 0+----+2   + |right    |/
		// lt| +   7+----+5 | +      z--+0
		//   |/    / bot /   |/
		//   +   6+----+4   +

		unsigned long c = m_NoteDesign.GetGridLineColor().BGRA();

		auto setVtx = [&](unsigned long i, const Vector3& pos) {
			pVertex[i].pos[0] = pos.x;
			pVertex[i].pos[1] = pos.y;
			pVertex[i].pos[2] = pos.z;
			pVertex[i].normal[0] = 0.0f;
			pVertex[i].normal[1] = 0.0f;
			pVertex[i].normal[2] = -1.0f;
			pVertex[i].color = c;
			pVertex[i].uv[0] = 0.0f;
			pVertex[i].uv[1] = 0.0f;
		};

		setVtx(0, startCorners[0]);
		setVtx(1, endCorners[0]);
		setVtx(2, startCorners[1]);
		setVtx(3, endCorners[1]);
		setVtx(4, startCorners[3]);
		setVtx(5, endCorners[3]);
		setVtx(6, startCorners[2]);
		setVtx(7, endCorners[2]);

		unsigned long edges[24] = {
			0, 1,  2, 3,  0, 2,  1, 3,   // top face
			4, 5,  6, 7,  4, 6,  5, 7,   // bottom face
			0, 6,  1, 7,  2, 4,  3, 5    // vertical
		};
		for (int i = 0; i < 24; i++) {
			pIndex[i] = edges[i];
		}
	}

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

EXIT:;
	return result;
}
