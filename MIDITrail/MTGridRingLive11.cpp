//******************************************************************************
//
// MIDITrail / MTGridRingLive11
//
// Live monitor grid ring renderer (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridRingLive11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Create
//******************************************************************************
int MTGridRingLive11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName
	)
{
	int result = 0;

	Release();

	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;

	result = _CreateVertices(pDevice, pContext);
	if (result != 0) goto EXIT;

	_SetupPrimitive();

	{
		Color lineColor = m_NoteDesign.GetGridLineColor();
		if (lineColor.A() < 0.01f) {
			m_isVisible = false;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Create vertices (2 rings: start + end of display window)
//******************************************************************************
int MTGridRingLive11::_CreateVertices(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	unsigned long vertexNum = GRID_RING_SEGMENTS * 2;
	unsigned long indexNum  = GRID_RING_SEGMENTS * 2 * 2;

	result = m_Primitive.CreateVertexBuffer(pDevice, vertexNum);
	if (result != 0) goto EXIT;
	result = m_Primitive.CreateIndexBuffer(pDevice, indexNum);
	if (result != 0) goto EXIT;

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	{
		Vector3 basePosStart, basePosEnd;
		m_NoteDesign.GetGridRingBasePosLive(&basePosStart, &basePosEnd);

		unsigned long color = m_NoteDesign.GetGridLineColor().BGRA();
		unsigned long vertexIndex = 0;

		_CreateVertexOfRing(pVertex, &vertexIndex, pIndex, basePosStart, color);
		_CreateVertexOfRing(pVertex, &vertexIndex, pIndex, basePosEnd, color);
	}

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

EXIT:;
	return result;
}
