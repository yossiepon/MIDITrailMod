//******************************************************************************
//
// MIDITrail / MTPianoKeyboardFlat11
//
// Piano keyboard renderer for flat scenes (single channel).
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXH.h"
#include "MTPianoKeyboardFlat11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


#define KEY_VERTEX_NUM_MAX  (44)  // KEY_WHITE_2_VERTEX_NUM
#define KEY_INDEX_NUM_MAX   (66)  // KEY_WHITE_2_INDEX_NUM


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
//******************************************************************************
// Create
//******************************************************************************
int MTPianoKeyboardFlat11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		ID3D11ShaderResourceView* pSRV
	)
{
	int result = 0;

	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	m_pKeyboardDesign = &m_KeyboardDesign;

	result = MTPianoKeyboard11::Create(pDevice, pContext, pSceneName, pSeqData, pSRV);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

MTPianoKeyboardFlat11::MTPianoKeyboardFlat11()
{
}

MTPianoKeyboardFlat11::~MTPianoKeyboardFlat11()
{
}

//******************************************************************************
// Vertex buffer creation (flat/linear key layout in Rain coordinates)
//******************************************************************************
int MTPianoKeyboardFlat11::_CreateVertexOfKeyboard(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	unsigned long totalVerts = 0, totalIndices = 0;
	for (unsigned char n = 0; n < SM_MAX_NOTE_NUM; n++) {
		totalVerts += m_BufInfo[n].vertexNum;
		totalIndices += m_BufInfo[n].indexNum;
	}
	m_VertexNum = totalVerts;

	result = m_Prim.CreateVertexBuffer(pDevice, totalVerts);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, totalIndices);
	if (result != 0) goto EXIT;

	result = m_Prim.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Prim.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	ZeroMemory(pVertex, totalVerts * sizeof(DXPRIMITIVE11_VERTEX));
	ZeroMemory(pIndex, totalIndices * sizeof(unsigned long));

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		result = _CreateVertexOfKey(
			noteNo,
			&pVertex[m_BufInfo[noteNo].vertexPos],
			&pIndex[m_BufInfo[noteNo].indexPos]
		);
		if (result != 0) goto EXIT;

		if (!m_pKeyboardDesign->IsKeyDisp(noteNo)) {
			unsigned long idxOff = m_BufInfo[noteNo].indexPos;
			for (unsigned long i = 0; i < m_BufInfo[noteNo].indexNum; i++) {
				pIndex[idxOff + i] = 0;
			}
		}
	}

	m_pBaseVerts = (DXPRIMITIVE11_VERTEX*)malloc(totalVerts * sizeof(DXPRIMITIVE11_VERTEX));
	m_pWorkVerts = (DXPRIMITIVE11_VERTEX*)malloc(totalVerts * sizeof(DXPRIMITIVE11_VERTEX));
	if (m_pBaseVerts == NULL || m_pWorkVerts == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	memcpy(m_pBaseVerts, pVertex, totalVerts * sizeof(DXPRIMITIVE11_VERTEX));
	memcpy(m_pWorkVerts, pVertex, totalVerts * sizeof(DXPRIMITIVE11_VERTEX));

	m_Prim.UnlockVertex(pContext);
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

EXIT:;
	return result;
}

//******************************************************************************
// BuildKeyCPU (flat: rotate in YZ plane, no orientation transform)
//******************************************************************************
int MTPianoKeyboardFlat11::_BuildKeyCPU(
		unsigned char noteNo,
		float rate,
		Color* pColor
	)
{
	int result = 0;

	if (noteNo >= SM_MAX_NOTE_NUM) {
		result = YN_SET_ERR("Program error.", noteNo, 0);
		goto EXIT;
	}

	if (!m_pKeyboardDesign->IsKeyDisp(noteNo)) goto EXIT;

	{
		KbdVertex tempVertex[KEY_VERTEX_NUM_MAX];
		unsigned long tempIndex[KEY_INDEX_NUM_MAX];

		switch (m_pKeyboardDesign->GetKeyType(noteNo)) {
			case MTPianoKeyboardDesign11::KeyWhiteC:
			case MTPianoKeyboardDesign11::KeyWhiteF:
				result = _CreateVertexOfKeyWhite1(noteNo, tempVertex, tempIndex, pColor);
				break;
			case MTPianoKeyboardDesign11::KeyWhiteD:
			case MTPianoKeyboardDesign11::KeyWhiteG:
			case MTPianoKeyboardDesign11::KeyWhiteA:
				result = _CreateVertexOfKeyWhite2(noteNo, tempVertex, tempIndex, pColor);
				break;
			case MTPianoKeyboardDesign11::KeyWhiteE:
			case MTPianoKeyboardDesign11::KeyWhiteB:
				result = _CreateVertexOfKeyWhite3(noteNo, tempVertex, tempIndex, pColor);
				break;
			case MTPianoKeyboardDesign11::KeyBlack:
				result = _CreateVertexOfKeyBlack(noteNo, tempVertex, tempIndex, pColor);
				break;
		}
		if (result != 0) goto EXIT;

		float angle = m_pKeyboardDesign->GetKeyRotateAngle() * rate;
		float centerY = 0.0f;
		float centerZ = m_pKeyboardDesign->GetKeyRotateAxisXPos();

		for (unsigned long i = 0; i < m_BufInfo[noteNo].vertexNum; i++) {
			tempVertex[i].p = DXH::RotateYZ(centerY, centerZ, tempVertex[i].p, angle);
			tempVertex[i].n = DXH::RotateYZ(0.0f, 0.0f, tempVertex[i].n, angle);
		}

		unsigned long vpos = m_BufInfo[noteNo].vertexPos;
		unsigned long vnum = m_BufInfo[noteNo].vertexNum;
		memcpy(&m_pWorkVerts[vpos], tempVertex, vnum * sizeof(DXPRIMITIVE11_VERTEX));
	}

EXIT:;
	return result;
}
