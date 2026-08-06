//******************************************************************************
//
// MIDITrail / MTDynamicCaption11
//
// DX11 dynamic caption renderer.
// Pre-renders all possible characters to a tile texture. SetString updates
// UV coordinates only (no texture regeneration). Each character is a quad
// (6 vertices in TRIANGLELIST).
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTDynamicCaption11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ / デストラクタ
//******************************************************************************
MTDynamicCaption11::MTDynamicCaption11()
{
	m_pDevice = NULL;
	m_Chars[0] = L'\0';
	m_CaptionSize = 0;
	m_Color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_isReady = false;
	m_isDirty = true;
	m_CharCount = 0;
	m_CurrentStr[0] = L'\0';
}

MTDynamicCaption11::~MTDynamicCaption11()
{
	Release();
}

//******************************************************************************
// 生成
//******************************************************************************
int MTDynamicCaption11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const WCHAR* pFontName,
		unsigned long fontSize,
		const WCHAR* pCharacters,
		unsigned long captionSize
	)
{
	int result = 0;

	Release();

	m_pDevice = pDevice;
	m_CaptionSize = captionSize;

	wcscpy_s(m_Chars, MTDYNAMICCAPTION11_MAX_CHARS, pCharacters);
	m_CharCount = (unsigned long)wcslen(m_Chars);

	// 各文字の UV 範囲を事前計算
	for (unsigned long i = 0; i < m_CharCount; i++) {
		m_CharUV[i].u0 = (float)i / (float)m_CharCount;
		m_CharUV[i].u1 = (float)(i + 1) / (float)m_CharCount;
	}

	// フォントテクスチャ生成（固定ピッチ強制）
	unsigned long rgb = 0x00FFFFFF;
	result = m_FontTexture.SetFont(pFontName, fontSize, rgb, true);
	if (result != 0) goto EXIT;

	result = m_FontTexture.CreateTexture(pDevice, pCharacters);
	if (result != 0) goto EXIT;

	// 頂点バッファ生成（1 文字 = 4 頂点）
	result = m_Primitive.CreateVertexBuffer(pDevice, 4 * m_CaptionSize);
	if (result != 0) goto EXIT;

	// インデックスバッファ生成（1 文字 = 6 インデックス）
	result = m_Primitive.CreateIndexBuffer(pDevice, 6 * m_CaptionSize);
	if (result != 0) goto EXIT;

	// インデックスは固定なので Create 時に書き込む
	{
		unsigned long* pIndex = NULL;
		ID3D11DeviceContext* pCtx = NULL;
		pDevice->GetImmediateContext(&pCtx);
		result = m_Primitive.LockIndex(pCtx, &pIndex);
		if (result != 0) {
			pCtx->Release();
			goto EXIT;
		}
		for (unsigned long i = 0; i < m_CaptionSize; i++) {
			unsigned long base = i * 4;
			unsigned long idx = i * 6;
			pIndex[idx + 0] = base + 0;
			pIndex[idx + 1] = base + 1;
			pIndex[idx + 2] = base + 2;
			pIndex[idx + 3] = base + 2;
			pIndex[idx + 4] = base + 1;
			pIndex[idx + 5] = base + 3;
		}
		m_Primitive.UnlockIndex(pCtx);
		pCtx->Release();
	}

	m_Primitive.SetLightEnable(false);
	m_Primitive.SetDepthWrite(false);

	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTDynamicCaption11::Release()
{
	m_FontTexture.Clear();
	m_Primitive.Release();
	m_isReady = false;
}

//******************************************************************************
// 文字列設定（UV 座標のみ更新）
//******************************************************************************
int MTDynamicCaption11::SetString(const WCHAR* pStr)
{
	if (pStr == NULL) return YN_SET_ERR("Program error.", 0, 0);

	wcsncpy_s(m_CurrentStr, MTDYNAMICCAPTION11_MAX_CHARS, pStr, _TRUNCATE);
	m_isDirty = true;

	return 0;
}

//******************************************************************************
// 色設定
//******************************************************************************
void MTDynamicCaption11::SetColor(Color color)
{
	m_Color = color;
	m_isDirty = true;
}

//******************************************************************************
// テクスチャサイズ取得
//******************************************************************************
void MTDynamicCaption11::GetTextureSize(unsigned long* pHeight, unsigned long* pWidth)
{
	m_FontTexture.GetTextureSize(pHeight, pWidth);
}

//******************************************************************************
// 文字の UV 座標を検索
//******************************************************************************
static bool _FindCharUV(
		WCHAR target,
		const WCHAR* chars,
		unsigned long charCount,
		float* pU0, float* pU1
	)
{
	for (unsigned long i = 0; i < charCount; i++) {
		if (chars[i] == target) {
			*pU0 = (float)i / (float)charCount;
			*pU1 = (float)(i + 1) / (float)charCount;
			return true;
		}
	}
	*pU0 = 0.0f;
	*pU1 = 0.0f;
	return false;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTDynamicCaption11::Draw(
		ID3D11DeviceContext* pContext,
		float x, float y,
		float magRate,
		unsigned int screenWidth,
		unsigned int screenHeight
	)
{
	int result = 0;

	if (!m_isReady) return 0;
	if (screenWidth == 0 || screenHeight == 0) return 0;

	// 頂点データ更新
	{
		DXPRIMITIVE11_VERTEX* pVertex = NULL;
		result = m_Primitive.LockVertex(pContext, &pVertex);
		if (result != 0) return result;

		unsigned long texH = 0, texW = 0;
		m_FontTexture.GetTextureSize(&texH, &texW);

		float charPixelW = ((float)texW / (float)m_CharCount) * magRate;
		float charPixelH = (float)texH * magRate;
		float sw = (float)screenWidth;
		float sh = (float)screenHeight;

		unsigned char cr = (unsigned char)(m_Color.R() * 255.0f);
		unsigned char cg = (unsigned char)(m_Color.G() * 255.0f);
		unsigned char cb = (unsigned char)(m_Color.B() * 255.0f);
		unsigned char ca = (unsigned char)(m_Color.A() * 255.0f);
		DWORD color = (ca << 24) | (cr << 16) | (cg << 8) | cb;

		auto setVtx = [&](unsigned long vi, float px, float py, float u, float v) {
			pVertex[vi].pos[0] = px;
			pVertex[vi].pos[1] = py;
			pVertex[vi].pos[2] = 0.0f;
			pVertex[vi].normal[0] = 0.0f;
			pVertex[vi].normal[1] = 0.0f;
			pVertex[vi].normal[2] = -1.0f;
			pVertex[vi].color = color;
			pVertex[vi].uv[0] = u;
			pVertex[vi].uv[1] = v;
		};

		for (unsigned long i = 0; i < m_CaptionSize; i++) {
			float px0 = x + charPixelW * (float)i;
			float px1 = px0 + charPixelW;
			float py0 = y;
			float py1 = y + charPixelH;

			float nx0 = (px0 / sw) * 2.0f - 1.0f;
			float nx1 = (px1 / sw) * 2.0f - 1.0f;
			float ny0 = 1.0f - (py0 / sh) * 2.0f;
			float ny1 = 1.0f - (py1 / sh) * 2.0f;

			float u0 = 0.0f, u1 = 0.0f;
			if (i < (unsigned long)wcslen(m_CurrentStr)) {
				_FindCharUV(m_CurrentStr[i], m_Chars, m_CharCount, &u0, &u1);
			}

			// 4 頂点（インデックスバッファで 2 三角形に展開）
			unsigned long base = i * 4;
			setVtx(base + 0, nx0, ny0, u0, 0.0f);
			setVtx(base + 1, nx1, ny0, u1, 0.0f);
			setVtx(base + 2, nx0, ny1, u0, 1.0f);
			setVtx(base + 3, nx1, ny1, u1, 1.0f);
		}

		m_Primitive.UnlockVertex(pContext);
	}

	// 描画
	m_Primitive.SetTexture(m_FontTexture.GetTexture());

	{
		Matrix identity;
		Vector4 lightDir(0.0f, 0.0f, 0.0f, 0.0f);
		result = m_Primitive.Draw(pContext, identity, lightDir);
	}

	return result;
}
