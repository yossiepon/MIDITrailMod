//******************************************************************************
//
// MIDITrail / MTLogo11
//
// MIDITrail logo renderer with gradation animation.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "MTLogo11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTLogo11::MTLogo11()
{
	m_pCpuVertex = NULL;
	m_VertexCount = 0;
	m_StartTime = 0;
}

MTLogo11::~MTLogo11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTLogo11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;

	Release();

	result = _CreateTexture(pDevice);
	if (result != 0) goto EXIT;

	result = _CreateVertex(pDevice, pContext);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Update (gradation animation)
//******************************************************************************
int MTLogo11::Update(
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;

	_SetGradationColor();

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;

	memcpy(pVertex, m_pCpuVertex, m_VertexCount * sizeof(DXPRIMITIVE11_VERTEX));

	m_Primitive.UnlockVertex(pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTLogo11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	m_Primitive.SetTexture(m_FontTexture.GetTexture());
	result = m_Primitive.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTLogo11::Release()
{
	m_Primitive.Release();
	m_FontTexture.Clear();

	delete[] m_pCpuVertex;
	m_pCpuVertex = NULL;
	m_VertexCount = 0;
}

//******************************************************************************
// Create texture
//******************************************************************************
int MTLogo11::_CreateTexture(
		ID3D11Device* pDevice
	)
{
	int result = 0;

	result = m_FontTexture.SetFont(L"Arial", 40, 0x00FFFFFF, false);
	if (result != 0) goto EXIT;

	result = m_FontTexture.CreateTexture(pDevice, MTLOGO_TITLE);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Create vertex
//******************************************************************************
int MTLogo11::_CreateVertex(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	unsigned long texHeight = 0;
	unsigned long texWidth = 0;
	float height = 0.0f;
	float width = 0.0f;

	m_VertexCount = 6 * MTLOGO_TILE_NUM;

	result = m_Primitive.CreateVertexBuffer(pDevice, m_VertexCount);
	if (result != 0) goto EXIT;

	m_Primitive.SetLightEnable(false);

	try {
		m_pCpuVertex = new DXPRIMITIVE11_VERTEX[m_VertexCount];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	m_FontTexture.GetTextureSize(&texHeight, &texWidth);
	height = texHeight * MTLOGO_MAG;
	width  = ((float)texWidth / (float)MTLOGO_TILE_NUM) * MTLOGO_MAG;

	for (unsigned long i = 0; i < MTLOGO_TILE_NUM; i++) {
		float x0 = MTLOGO_POS_X + width * i;
		float x1 = MTLOGO_POS_X + width * (i + 1.0f);
		float y0 = MTLOGO_POS_Y;
		float y1 = MTLOGO_POS_Y - height;
		float u0 = (float)i / (float)MTLOGO_TILE_NUM;
		float u1 = (float)(i + 1) / (float)MTLOGO_TILE_NUM;

		float nrm[3] = { 0.0f, 0.0f, -1.0f };
		unsigned long clr = 0xFF000000;

		float positions[6][3] = {
			{ x0, y0, 0.0f }, { x1, y0, 0.0f }, { x0, y1, 0.0f },
			{ x0, y1, 0.0f }, { x1, y0, 0.0f }, { x1, y1, 0.0f }
		};
		float uvs[6][2] = {
			{ u0, 0.0f }, { u1, 0.0f }, { u0, 1.0f },
			{ u0, 1.0f }, { u1, 0.0f }, { u1, 1.0f }
		};

		for (unsigned long v = 0; v < 6; v++) {
			memcpy(m_pCpuVertex[i*6+v].pos, positions[v], sizeof(float)*3);
			memcpy(m_pCpuVertex[i*6+v].normal, nrm, sizeof(float)*3);
			m_pCpuVertex[i*6+v].color = clr;
			memcpy(m_pCpuVertex[i*6+v].uv, uvs[v], sizeof(float)*2);
		}
	}

	m_Primitive.SetWorldMatrix(Matrix());

EXIT:;
	return result;
}

//******************************************************************************
// Gradation color
//******************************************************************************
void MTLogo11::_SetGradationColor()
{
	if (m_pCpuVertex == NULL) return;

	if (m_StartTime == 0) {
		m_StartTime = timeGetTime();
	}
	unsigned long sceneTime = timeGetTime() - m_StartTime;

	for (unsigned long i = 0; i < MTLOGO_TILE_NUM; i++) {
		unsigned long delay = i * (MTLOGO_GRADATION_TIME / MTLOGO_TILE_NUM);
		unsigned long tileTime = (sceneTime < delay) ? 0 : (sceneTime - delay);

		float c = 0.0f;
		if (tileTime < MTLOGO_GRADATION_TIME) {
			c = (float)tileTime / (float)MTLOGO_GRADATION_TIME;
		}
		else if (tileTime < (MTLOGO_GRADATION_TIME * 2)) {
			c = 1.0f - ((float)(tileTime - MTLOGO_GRADATION_TIME) / (float)MTLOGO_GRADATION_TIME);
		}

		unsigned char cb = (unsigned char)(c * 255.0f);
		unsigned long color = 0xFF000000 | (cb << 16) | (cb << 8) | cb;

		for (unsigned long v = 0; v < 6; v++) {
			m_pCpuVertex[i*6+v].color = color;
		}
	}
}
