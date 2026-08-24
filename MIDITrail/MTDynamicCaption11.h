//******************************************************************************
//
// MIDITrail / MTDynamicCaption11
//
// Dynamic caption renderer (tile texture, UV update only).
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTCaptionBase11.h"

#define MTDYNAMICCAPTION11_MAX_CHARS  (256)

// ASCII printable characters (0x20-0x7E): shared by Dashboard and DiagOverlay
#define MT_ASCII_PRINTABLE_CHARS  L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"


//******************************************************************************
// DX11 dynamic caption renderer
//******************************************************************************
class MTDynamicCaption11 : public MTCaptionBase11
{
public:

	MTDynamicCaption11();
	virtual ~MTDynamicCaption11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const WCHAR* pFontName,
			unsigned long fontSize,
			const WCHAR* pCharacters,
			unsigned long captionSize
		);

	int CreateWithSharedTexture(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			ID3D11ShaderResourceView* pSharedSRV,
			unsigned long texWidth,
			unsigned long texHeight,
			const WCHAR* pCharacters,
			unsigned long captionSize
		);

	void Release() override;

	int SetString(const WCHAR* pStr);
	void GetDisplayCharSize(float magRate, float* pCharWidth, float* pCharHeight);

	int Draw(
			ID3D11DeviceContext* pContext,
			float x, float y,
			float magRate,
			unsigned int screenWidth,
			unsigned int screenHeight
		);

private:

	ID3D11Device* m_pDevice;
	WCHAR m_Chars[MTDYNAMICCAPTION11_MAX_CHARS];
	unsigned long m_CaptionSize;
	bool m_isDirty;

	struct CharUV {
		float u0, u1;
	};
	CharUV m_CharUV[MTDYNAMICCAPTION11_MAX_CHARS];
	unsigned long m_CharCount;
	WCHAR m_CurrentStr[MTDYNAMICCAPTION11_MAX_CHARS];
};
