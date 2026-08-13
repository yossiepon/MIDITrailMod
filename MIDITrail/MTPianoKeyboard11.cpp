//******************************************************************************
//
// MIDITrail / MTPianoKeyboard11
//
// Piano keyboard renderer base class (single channel, Rain/Roll shared).
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboard11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constants
//******************************************************************************
#define KEY_WHITE_1_VERTEX_NUM  (38)
#define KEY_WHITE_2_VERTEX_NUM  (44)
#define KEY_WHITE_3_VERTEX_NUM  (38)
#define KEY_BLACK_VERTEX_NUM    (30)

#define KEY_WHITE_1_INDEX_NUM   (60)
#define KEY_WHITE_2_INDEX_NUM   (66)
#define KEY_WHITE_3_INDEX_NUM   (60)
#define KEY_BLACK_INDEX_NUM     (48)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboard11::MTPianoKeyboard11()
{
	m_pSRV = NULL;
	m_pBaseVerts = NULL;
	m_pWorkVerts = NULL;
	m_VertexNum = 0;
	m_pKeyboardDesign = NULL;
	ZeroMemory(m_PrevRate, sizeof(m_PrevRate));
	ZeroMemory(m_PrevColor, sizeof(m_PrevColor));
	ZeroMemory(m_BufInfo, sizeof(m_BufInfo));
}

MTPianoKeyboard11::~MTPianoKeyboard11()
{
	Release();
}

//******************************************************************************
// Create (template method: derived sets m_pKeyboardDesign before calling)
//******************************************************************************
int MTPianoKeyboard11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		ID3D11ShaderResourceView* pSRV
	)
{
	int result = 0;

	Release();

	ZeroMemory(m_PrevRate, sizeof(m_PrevRate));
	ZeroMemory(m_PrevColor, sizeof(m_PrevColor));

	if (pDevice == NULL || pSRV == NULL || m_pKeyboardDesign == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_pSRV = pSRV;

	_CreateBufInfo();

	result = _CreateVertexOfKeyboard(pDevice, pContext);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTPianoKeyboard11::Release()
{
	m_Prim.Release();
	if (m_pBaseVerts != NULL) { free(m_pBaseVerts); m_pBaseVerts = NULL; }
	if (m_pWorkVerts != NULL) { free(m_pWorkVerts); m_pWorkVerts = NULL; }
	m_pSRV = NULL;
	m_VertexNum = 0;
}

//******************************************************************************
// BufInfo generation
//******************************************************************************
void MTPianoKeyboard11::_CreateBufInfo()
{
	unsigned long vertexPos = 0;
	unsigned long indexPos = 0;

	ZeroMemory(m_BufInfo, sizeof(m_BufInfo));

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		unsigned long vn = 0, in = 0;
		switch (m_pKeyboardDesign->GetKeyType(noteNo)) {
			case MTPianoKeyboardDesign11::KeyWhiteC:
			case MTPianoKeyboardDesign11::KeyWhiteF:
				vn = KEY_WHITE_1_VERTEX_NUM; in = KEY_WHITE_1_INDEX_NUM; break;
			case MTPianoKeyboardDesign11::KeyWhiteD:
			case MTPianoKeyboardDesign11::KeyWhiteG:
			case MTPianoKeyboardDesign11::KeyWhiteA:
				vn = KEY_WHITE_2_VERTEX_NUM; in = KEY_WHITE_2_INDEX_NUM; break;
			case MTPianoKeyboardDesign11::KeyWhiteE:
			case MTPianoKeyboardDesign11::KeyWhiteB:
				vn = KEY_WHITE_3_VERTEX_NUM; in = KEY_WHITE_3_INDEX_NUM; break;
			case MTPianoKeyboardDesign11::KeyBlack:
				vn = KEY_BLACK_VERTEX_NUM; in = KEY_BLACK_INDEX_NUM; break;
		}
		m_BufInfo[noteNo].vertexNum = vn;
		m_BufInfo[noteNo].indexNum = in;
		m_BufInfo[noteNo].vertexPos = vertexPos;
		m_BufInfo[noteNo].indexPos = indexPos;
		vertexPos += vn;
		indexPos += in;
	}
}

//******************************************************************************
// Update (non-virtual template: diff detection -> _BuildKeyCPU/_ResetKeyCPU -> flush)
//******************************************************************************
int MTPianoKeyboard11::Update(
		ID3D11DeviceContext* pContext,
		const MTKeyboardKeyState keyStates[SM_MAX_NOTE_NUM],
		const Matrix& world
	)
{
	int result = 0;
	bool changed = false;

	if (m_pBaseVerts == NULL || m_pWorkVerts == NULL) goto EXIT;

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		float rate = keyStates[noteNo].rate;
		unsigned long color = keyStates[noteNo].color;

		bool rateChanged = (rate != m_PrevRate[noteNo]);
		bool colorChanged = (rate > 0.0f) && (color != m_PrevColor[noteNo]);

		if (!rateChanged && !colorChanged) continue;

		if (rate == 0.0f) {
			result = _ResetKeyCPU(noteNo);
		}
		else if (rate >= 1.0f) {
			Color c((unsigned int)color);
			result = _BuildKeyCPU(noteNo, rate, &c);
		}
		else {
			result = _BuildKeyCPU(noteNo, rate, NULL);
		}
		if (result != 0) goto EXIT;

		m_PrevRate[noteNo] = rate;
		m_PrevColor[noteNo] = color;
		changed = true;
	}

	if (changed) {
		result = _FlushToGPU(pContext);
		if (result != 0) goto EXIT;
	}

	m_Prim.SetWorldMatrix(world);

EXIT:;
	return result;
}

//******************************************************************************
// ResetKeyCPU: restore key to unpressed state from base buffer
//******************************************************************************
int MTPianoKeyboard11::_ResetKeyCPU(
		unsigned char noteNo
	)
{
	if (noteNo >= SM_MAX_NOTE_NUM) return 0;

	unsigned long vpos = m_BufInfo[noteNo].vertexPos;
	unsigned long vnum = m_BufInfo[noteNo].vertexNum;
	memcpy(&m_pWorkVerts[vpos], &m_pBaseVerts[vpos],
		vnum * sizeof(DXPRIMITIVE11_VERTEX));

	return 0;
}

//******************************************************************************
// FlushToGPU: WRITE_DISCARD copy of work buffer to GPU
//******************************************************************************
int MTPianoKeyboard11::_FlushToGPU(
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;

	result = m_Prim.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;

	memcpy(pVertex, m_pWorkVerts, m_VertexNum * sizeof(DXPRIMITIVE11_VERTEX));

	m_Prim.UnlockVertex(pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTPianoKeyboard11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	m_Prim.SetTexture(m_pSRV);
	m_Prim.SetLightEnable(true);
	m_Prim.SetMaterialAmbient(0.5f, 0.5f, 0.5f);

	result = m_Prim.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Single key vertex dispatch (calls White1/2/3 or Black based on key type)
//******************************************************************************
int MTPianoKeyboard11::_CreateVertexOfKey(
		unsigned char noteNo,
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;

	KbdVertex* pKbdVertex = reinterpret_cast<KbdVertex*>(pVertex);

	switch (m_pKeyboardDesign->GetKeyType(noteNo)) {
		case MTPianoKeyboardDesign11::KeyWhiteC:
		case MTPianoKeyboardDesign11::KeyWhiteF:
			result = _CreateVertexOfKeyWhite1(noteNo, pKbdVertex, pIndex);
			break;
		case MTPianoKeyboardDesign11::KeyWhiteD:
		case MTPianoKeyboardDesign11::KeyWhiteG:
		case MTPianoKeyboardDesign11::KeyWhiteA:
			result = _CreateVertexOfKeyWhite2(noteNo, pKbdVertex, pIndex);
			break;
		case MTPianoKeyboardDesign11::KeyWhiteE:
		case MTPianoKeyboardDesign11::KeyWhiteB:
			result = _CreateVertexOfKeyWhite3(noteNo, pKbdVertex, pIndex);
			break;
		case MTPianoKeyboardDesign11::KeyBlack:
			result = _CreateVertexOfKeyBlack(noteNo, pKbdVertex, pIndex);
			break;
	}

	return result;
}
