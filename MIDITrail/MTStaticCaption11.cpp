//******************************************************************************
//
// MIDITrail / MTStaticCaption11
//
// Static caption renderer.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTStaticCaption11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTStaticCaption11::MTStaticCaption11()
{
	m_Color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_isReady = false;
}

MTStaticCaption11::~MTStaticCaption11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTStaticCaption11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const WCHAR* pFontName,
		unsigned long fontSize,
		const WCHAR* pText
	)
{
	int result = 0;

	Release();

	unsigned long rgb = 0x00FFFFFF;
	result = m_FontTexture.SetFont(pFontName, fontSize, rgb, false);
	if (result != 0) goto EXIT;

	result = m_FontTexture.CreateTexture(pDevice, pText);
	if (result != 0) goto EXIT;

	// Quad: 4 vertices + 6 indices
	result = m_Primitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_Primitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;

	// Indices are fixed
	{
		unsigned long* pIndex = NULL;
		result = m_Primitive.LockIndex(pContext, &pIndex);
		if (result != 0) goto EXIT;

		pIndex[0] = 0; pIndex[1] = 1; pIndex[2] = 2;
		pIndex[3] = 2; pIndex[4] = 1; pIndex[5] = 3;

		m_Primitive.UnlockIndex(pContext);
	}

	m_Primitive.SetLightEnable(false);
	m_Primitive.SetDepthWrite(false);

	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTStaticCaption11::Release()
{
	m_FontTexture.Clear();
	m_Primitive.Release();
	m_isReady = false;
}

//******************************************************************************
// Set color
//******************************************************************************
void MTStaticCaption11::SetColor(Color color)
{
	m_Color = color;
}

//******************************************************************************
// Get texture size
//******************************************************************************
void MTStaticCaption11::GetTextureSize(unsigned long* pHeight, unsigned long* pWidth)
{
	m_FontTexture.GetTextureSize(pHeight, pWidth);
}

//******************************************************************************
// Get display size (supersample-adjusted)
//******************************************************************************
void MTStaticCaption11::GetDisplaySize(float magRate, float* pWidth, float* pHeight)
{
	static const float SS = 0.5f;
	unsigned long texH = 0, texW = 0;
	m_FontTexture.GetTextureSize(&texH, &texW);
	if (pWidth != NULL)  *pWidth  = (float)texW * magRate * SS;
	if (pHeight != NULL) *pHeight = (float)texH * magRate * SS;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTStaticCaption11::Draw(
		ID3D11DeviceContext* pContext,
		float x, float y,
		float magRate,
		unsigned int screenWidth,
		unsigned int screenHeight
	)
{
	if (!m_isReady) return 0;
	if (screenWidth == 0 || screenHeight == 0) return 0;

	int result = 0;

	unsigned long texH = 0, texW = 0;
	m_FontTexture.GetTextureSize(&texH, &texW);
	if (texW == 0 || texH == 0) return 0;

	static const float SS = 0.5f;
	float drawW = (float)texW * magRate * SS;
	float drawH = (float)texH * magRate * SS;
	float sw = (float)screenWidth;
	float sh = (float)screenHeight;

	float ndcX0 = (x / sw) * 2.0f - 1.0f;
	float ndcX1 = ((x + drawW) / sw) * 2.0f - 1.0f;
	float ndcY0 = 1.0f - (y / sh) * 2.0f;
	float ndcY1 = 1.0f - ((y + drawH) / sh) * 2.0f;

	unsigned char cr = (unsigned char)(m_Color.R() * 255.0f);
	unsigned char cg = (unsigned char)(m_Color.G() * 255.0f);
	unsigned char cb = (unsigned char)(m_Color.B() * 255.0f);
	unsigned char ca = (unsigned char)(m_Color.A() * 255.0f);
	DWORD color = (ca << 24) | (cr << 16) | (cg << 8) | cb;

	// Update vertices (Map/Unmap only, no buffer recreation)
	{
		DXPRIMITIVE11_VERTEX* pVertex = NULL;
		result = m_Primitive.LockVertex(pContext, &pVertex);
		if (result != 0) return result;

		auto setVtx = [&](unsigned long i, float px, float py, float u, float v) {
			pVertex[i].pos[0] = px;
			pVertex[i].pos[1] = py;
			pVertex[i].pos[2] = 0.0f;
			pVertex[i].normal[0] = 0.0f;
			pVertex[i].normal[1] = 0.0f;
			pVertex[i].normal[2] = -1.0f;
			pVertex[i].color = color;
			pVertex[i].uv[0] = u;
			pVertex[i].uv[1] = v;
		};

		setVtx(0, ndcX0, ndcY0, 0.0f, 0.0f);
		setVtx(1, ndcX1, ndcY0, 1.0f, 0.0f);
		setVtx(2, ndcX0, ndcY1, 0.0f, 1.0f);
		setVtx(3, ndcX1, ndcY1, 1.0f, 1.0f);

		m_Primitive.UnlockVertex(pContext);
	}

	m_Primitive.SetTexture(m_FontTexture.GetTexture());

	{
		Matrix identity;
		Vector4 lightDir(0.0f, 0.0f, 0.0f, 0.0f);
		result = m_Primitive.Draw(pContext, identity, lightDir);
	}

	return result;
}
