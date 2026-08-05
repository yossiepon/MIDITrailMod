//******************************************************************************
//
// MIDITrail / MTPianoKeyboard11
//
// DX11 piano keyboard renderer (1ch).
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXH.h"
#include "MTPianoKeyboard11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

// DX9 互換の読みやすい頂点構造体（メモリレイアウトは DXPRIMITIVE11_VERTEX と同一）
struct KbdVertex {
	Vector3 p;
	Vector3 n;
	unsigned long c;
	Vector2 t;
};
static_assert(sizeof(KbdVertex) == sizeof(DXPRIMITIVE11_VERTEX), "KbdVertex layout mismatch");


//******************************************************************************
// Constants (per DX9 MTPianoKeyboard)
//******************************************************************************
#define KEY_WHITE_1_VERTEX_NUM  (38)
#define KEY_WHITE_2_VERTEX_NUM  (44)
#define KEY_WHITE_3_VERTEX_NUM  (38)
#define KEY_BLACK_VERTEX_NUM    (30)
#define KEY_VERTEX_NUM_MAX      KEY_WHITE_2_VERTEX_NUM

#define KEY_WHITE_1_INDEX_NUM   (60)
#define KEY_WHITE_2_INDEX_NUM   (66)
#define KEY_WHITE_3_INDEX_NUM   (60)
#define KEY_BLACK_INDEX_NUM     (48)
#define KEY_INDEX_NUM_MAX       KEY_WHITE_2_INDEX_NUM


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboard11::MTPianoKeyboard11()
{
	m_pSRV = NULL;
	m_pBaseVerts = NULL;
	m_pWorkVerts = NULL;
	m_VertexNum = 0;
	ZeroMemory(m_PrevRate, sizeof(m_PrevRate));
	ZeroMemory(m_PrevColor, sizeof(m_PrevColor));
	ZeroMemory(m_BufInfo, sizeof(m_BufInfo));
}

MTPianoKeyboard11::~MTPianoKeyboard11()
{
	Release();
}

//******************************************************************************
// Create
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

	if (pDevice == NULL || pSRV == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_pSRV = pSRV;

	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

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
		switch (m_KeyboardDesign.GetKeyType(noteNo)) {
			case MTPianoKeyboardDesign::KeyWhiteC:
			case MTPianoKeyboardDesign::KeyWhiteF:
				vn = KEY_WHITE_1_VERTEX_NUM; in = KEY_WHITE_1_INDEX_NUM; break;
			case MTPianoKeyboardDesign::KeyWhiteD:
			case MTPianoKeyboardDesign::KeyWhiteG:
			case MTPianoKeyboardDesign::KeyWhiteA:
				vn = KEY_WHITE_2_VERTEX_NUM; in = KEY_WHITE_2_INDEX_NUM; break;
			case MTPianoKeyboardDesign::KeyWhiteE:
			case MTPianoKeyboardDesign::KeyWhiteB:
				vn = KEY_WHITE_3_VERTEX_NUM; in = KEY_WHITE_3_INDEX_NUM; break;
			case MTPianoKeyboardDesign::KeyBlack:
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
// Vertex buffer creation
//******************************************************************************
int MTPianoKeyboard11::_CreateVertexOfKeyboard(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	// Total counts
	unsigned long totalVerts = 0, totalIndices = 0;
	for (unsigned char n = 0; n < SM_MAX_NOTE_NUM; n++) {
		totalVerts += m_BufInfo[n].vertexNum;
		totalIndices += m_BufInfo[n].indexNum;
	}
	m_VertexNum = totalVerts;

	// GPU buffers
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

	// Generate each key
	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		result = _CreateVertexOfKey(
			noteNo,
			&pVertex[m_BufInfo[noteNo].vertexPos],
			&pIndex[m_BufInfo[noteNo].indexPos]
		);
		if (result != 0) goto EXIT;

		// Hide keys outside display range
		if (!m_KeyboardDesign.IsKeyDisp(noteNo)) {
			unsigned long idxOff = m_BufInfo[noteNo].indexPos;
			for (unsigned long i = 0; i < m_BufInfo[noteNo].indexNum; i++) {
				pIndex[idxOff + i] = 0;
			}
		}
	}

	// CPU mirror (copy before Unlock invalidates pVertex)
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

	ZeroMemory(m_PrevRate, sizeof(m_PrevRate));
	ZeroMemory(m_PrevColor, sizeof(m_PrevColor));

EXIT:;
	return result;
}

//******************************************************************************
// Single key vertex generation (dispatches to White1/2/3 or Black)
//******************************************************************************
int MTPianoKeyboard11::_CreateVertexOfKey(
		unsigned char noteNo,
		DXPRIMITIVE11_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;

	KbdVertex* pKbdVertex = reinterpret_cast<KbdVertex*>(pVertex);

	switch (m_KeyboardDesign.GetKeyType(noteNo)) {
		case MTPianoKeyboardDesign::KeyWhiteC:
		case MTPianoKeyboardDesign::KeyWhiteF:
			result = _CreateVertexOfKeyWhite1(noteNo, pKbdVertex, pIndex);
			break;
		case MTPianoKeyboardDesign::KeyWhiteD:
		case MTPianoKeyboardDesign::KeyWhiteG:
		case MTPianoKeyboardDesign::KeyWhiteA:
			result = _CreateVertexOfKeyWhite2(noteNo, pKbdVertex, pIndex);
			break;
		case MTPianoKeyboardDesign::KeyWhiteE:
		case MTPianoKeyboardDesign::KeyWhiteB:
			result = _CreateVertexOfKeyWhite3(noteNo, pKbdVertex, pIndex);
			break;
		case MTPianoKeyboardDesign::KeyBlack:
			result = _CreateVertexOfKeyBlack(noteNo, pKbdVertex, pIndex);
			break;
	}

	return result;
}

//******************************************************************************
// White key type 1 (C, F) - stub: delegates to DX9 geometry via temp buffer
// TODO: Full vertex generation to be ported from DX9 MTPianoKeyboard.cpp
//******************************************************************************
// Vertex generation methods are implemented via KbdVertex adapter.
// KbdVertex has identical memory layout to DXPRIMITIVE11_VERTEX, so
// reinterpret_cast is safe (verified by static_assert above).
// The code preserves DX9's ASCII art comments for key geometry.
#include "MTPianoKeyboard11_vertex.inc"

//******************************************************************************
// Update (called by Ctrl with key states)
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
// BuildKeyCPU: generate rotated/colored key vertices in CPU work buffer
//******************************************************************************
int MTPianoKeyboard11::_BuildKeyCPU(
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

	if (!m_KeyboardDesign.IsKeyDisp(noteNo)) goto EXIT;

	{
		KbdVertex tempVertex[KEY_VERTEX_NUM_MAX];
		unsigned long tempIndex[KEY_INDEX_NUM_MAX];

		// Generate unrotated vertices with color
		switch (m_KeyboardDesign.GetKeyType(noteNo)) {
			case MTPianoKeyboardDesign::KeyWhiteC:
			case MTPianoKeyboardDesign::KeyWhiteF:
				result = _CreateVertexOfKeyWhite1(noteNo, tempVertex, tempIndex, pColor);
				break;
			case MTPianoKeyboardDesign::KeyWhiteD:
			case MTPianoKeyboardDesign::KeyWhiteG:
			case MTPianoKeyboardDesign::KeyWhiteA:
				result = _CreateVertexOfKeyWhite2(noteNo, tempVertex, tempIndex, pColor);
				break;
			case MTPianoKeyboardDesign::KeyWhiteE:
			case MTPianoKeyboardDesign::KeyWhiteB:
				result = _CreateVertexOfKeyWhite3(noteNo, tempVertex, tempIndex, pColor);
				break;
			case MTPianoKeyboardDesign::KeyBlack:
				result = _CreateVertexOfKeyBlack(noteNo, tempVertex, tempIndex, pColor);
				break;
		}
		if (result != 0) goto EXIT;

		// Rotate vertices around pivot
		float angle = _GetKeyRotateAngle() * rate;
		float centerY = 0.0f;
		float centerZ = m_KeyboardDesign.GetKeyRotateAxisXPos();

		for (unsigned long i = 0; i < m_BufInfo[noteNo].vertexNum; i++) {
			tempVertex[i].p = DXH::RotateYZ(centerY, centerZ, tempVertex[i].p, angle);
			tempVertex[i].n = DXH::RotateYZ(0.0f, 0.0f, tempVertex[i].n, angle);
		}

		// Copy to work buffer (KbdVertex and DXPRIMITIVE11_VERTEX have same layout)
		unsigned long vpos = m_BufInfo[noteNo].vertexPos;
		unsigned long vnum = m_BufInfo[noteNo].vertexNum;
		memcpy(&m_pWorkVerts[vpos], tempVertex, vnum * sizeof(DXPRIMITIVE11_VERTEX));
	}

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
// Get key rotate angle (virtual - overridden by Mod11)
//******************************************************************************
float MTPianoKeyboard11::_GetKeyRotateAngle()
{
	return m_KeyboardDesign.GetKeyRotateAngle();
}
