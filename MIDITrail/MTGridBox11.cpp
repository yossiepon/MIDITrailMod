//******************************************************************************
//
// MIDITrail / MTGridBox11
//
// Grid box renderer (Playback).
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridBox11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Create
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

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	{
		Color lineColor = m_NoteDesign.GetGridLineColor();
		if (lineColor.A() < 0.01f) {
			m_isVisible = false;
		}
	}

	result = _CreateVertices(pDevice, pContext, pSeqData);
	if (result != 0) goto EXIT;

	_SetupPrimitive();
	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// Vertex generation
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

	// Vertex count: box 8 vertices + bar lines (2*barNum) + port divider lines (4*(portNum-1))
	// Index count: box 12 edges*2 + bar lines (2*barNum) + port divider lines (4*(portNum-1))
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

		// Grid color
		Color gridColor = m_NoteDesign.GetGridLineColor();
		unsigned char cr = (unsigned char)(gridColor.R() * 255.0f);
		unsigned char cg = (unsigned char)(gridColor.G() * 255.0f);
		unsigned char cb = (unsigned char)(gridColor.B() * 255.0f);
		unsigned char ca = (unsigned char)(gridColor.A() * 255.0f);
		DWORD color = (ca << 24) | (cr << 16) | (cg << 8) | cb;

		// 8 vertices of the box
		unsigned char lastPortNo = 0;
		portList.GetPort(portList.GetSize() - 1, &lastPortNo);

		Vector3 startFirst[4], endFirst[4];
		Vector3 startFinal[4], endFinal[4];

		m_NoteDesign.GetGridBoxVirtexPos(0, 0, &startFirst[0], &startFirst[1], &startFirst[2], &startFirst[3]);
		m_NoteDesign.GetGridBoxVirtexPos(totalTickTime, 0, &endFirst[0], &endFirst[1], &endFirst[2], &endFirst[3]);
		m_NoteDesign.GetGridBoxVirtexPos(0, lastPortNo, &startFinal[0], &startFinal[1], &startFinal[2], &startFinal[3]);
		m_NoteDesign.GetGridBoxVirtexPos(totalTickTime, lastPortNo, &endFinal[0], &endFinal[1], &endFinal[2], &endFinal[3]);

		// Box vertices
		//     1+----+3        y x
		//    / top /          |/
		//  0+----+2         z--+0
		//    7+----+5
		//    / bot /
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

		//     +   1+----+3   +
		//    /|   / top /   /|      y x
		//   + | 0+----+2   + |      |/
		// left| + 7+----+5 | +   z--+0
		//   |/    / bot /  |/
		//   +   6+----+4   +
		// Top face
		setVtx(0, startFinal[0]);   // FinalPort, tickTime=0, top-left
		setVtx(1, endFinal[0]);     // FinalPort, tickTime=end, top-left
		setVtx(2, startFirst[1]);   // FirstPort, tickTime=0, top-right
		setVtx(3, endFirst[1]);     // FirstPort, tickTime=end, top-right
		// Bottom face
		setVtx(4, startFirst[3]);   // FirstPort, tickTime=0, bottom-right
		setVtx(5, endFirst[3]);     // FirstPort, tickTime=end, bottom-right
		setVtx(6, startFinal[2]);   // FinalPort, tickTime=0, bottom-left
		setVtx(7, endFinal[2]);     // FinalPort, tickTime=end, bottom-left

		// Indices for the 12 edges
		unsigned long edges[] = {
			0,1, 1,3, 3,2, 2,0,  // top face, 4 edges
			6,7, 7,5, 5,4, 4,6,  // bottom face, 4 edges
			0,6, 1,7, 3,5, 2,4   // vertical, 4 edges
		};
		for (unsigned long i = 0; i < 24; i++) {
			pIndex[i] = edges[i];
		}

		// Bar lines (lines connecting vertices 0 and 2 of lastPortNo = left face, Y-axis direction)
		unsigned long vi = 8;
		unsigned long ii = 24;
		for (unsigned long bar = 0; bar < barNum; bar++) {
			unsigned long barTickTime = 0;
			barList.GetBar(bar, &barTickTime);

			Vector3 barVtx[4];
			m_NoteDesign.GetGridBoxVirtexPos(barTickTime, lastPortNo,
				&barVtx[0], &barVtx[1], &barVtx[2], &barVtx[3]);

			setVtx(vi,     barVtx[0]);  // top-left
			setVtx(vi + 1, barVtx[2]);  // bottom-left
			pIndex[ii]     = vi;
			pIndex[ii + 1] = vi + 1;

			vi += 2;
			ii += 2;
		}

		// Port divider lines (2 lines connecting vertices 1 and 3 of each port at time 0 and the end)
		for (unsigned long p = 1; p < portNum; p++) {
			unsigned char portNo = 0;
			portList.GetPort(p, &portNo);

			Vector3 ps[4], pe[4];
			m_NoteDesign.GetGridBoxVirtexPos(0, portNo, &ps[0], &ps[1], &ps[2], &ps[3]);
			m_NoteDesign.GetGridBoxVirtexPos(totalTickTime, portNo, &pe[0], &pe[1], &pe[2], &pe[3]);

			setVtx(vi,     ps[1]);  // time 0, top-right
			setVtx(vi + 1, pe[1]);  // end time, top-right
			setVtx(vi + 2, ps[3]);  // time 0, bottom-right
			setVtx(vi + 3, pe[3]);  // end time, bottom-right
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
