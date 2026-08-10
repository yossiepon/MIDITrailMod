//******************************************************************************
//
// MIDITrail / MTGridRing11
//
// DX11 grid ring renderer (Playback mode).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridRing11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Create
//******************************************************************************
int MTGridRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMBarList barList;
	unsigned long barNum = 0;
	unsigned long totalTickTime = 0;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	totalTickTime = pSeqData->GetTotalTickTime();
	result = pSeqData->GetBarList(&barList);
	if (result != 0) goto EXIT;
	barNum = barList.GetSize();

	{
		unsigned long vertexNum = GRID_RING_SEGMENTS * (barNum + 1);
		unsigned long indexNum  = GRID_RING_SEGMENTS * 2 * (barNum + 1);

		result = m_Primitive.CreateVertexBuffer(pDevice, vertexNum);
		if (result != 0) goto EXIT;
		result = m_Primitive.CreateIndexBuffer(pDevice, indexNum);
		if (result != 0) goto EXIT;

		_SetupPrimitive();

		DXPRIMITIVE11_VERTEX* pVertex = NULL;
		unsigned long* pIndex = NULL;

		result = m_Primitive.LockVertex(pContext, &pVertex);
		if (result != 0) goto EXIT;
		result = m_Primitive.LockIndex(pContext, &pIndex);
		if (result != 0) goto EXIT;

		result = _CreateVertexOfGrid(pVertex, pIndex, totalTickTime, &barList);
		if (result != 0) goto EXIT;

		m_Primitive.UnlockVertex(pContext);
		m_Primitive.UnlockIndex(pContext);
	}

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
// Grid vertex creation
//******************************************************************************
int MTGridRing11::_CreateVertexOfGrid(
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long totalTickTime,
		SMBarList* pBarList
	)
{
	int result = 0;
	unsigned long vertexIndex = 0;
	unsigned long tickTime = 0;
	Vector3 basePos;

	unsigned long color = m_NoteDesign.GetGridLineColor().BGRA();

	for (unsigned long i = 0; i < pBarList->GetSize(); i++) {
		result = pBarList->GetBar(i, &tickTime);
		if (result != 0) goto EXIT;

		m_NoteDesign.GetGridRingBasePos(tickTime, &basePos);
		_CreateVertexOfRing(pVertex, &vertexIndex, pIndex, basePos, color);
	}

	m_NoteDesign.GetGridRingBasePos(totalTickTime, &basePos);
	_CreateVertexOfRing(pVertex, &vertexIndex, pIndex, basePos, color);

EXIT:;
	return result;
}
