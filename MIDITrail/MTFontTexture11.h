//******************************************************************************
//
// MIDITrail / MTFontTexture11
//
// Font texture class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include "MTFont2Bmp.h"


//******************************************************************************
// DX11 font texture class
//******************************************************************************
class MTFontTexture11
{
public:

	MTFontTexture11();
	virtual ~MTFontTexture11();

	void Clear();

	int SetFont(
			const WCHAR* pFontName,
			unsigned long fontSize,
			unsigned long rgb,
			bool isForceFixedPitch = false
		);

	int CreateTexture(ID3D11Device* pDevice, const WCHAR* pStr);

	ID3D11ShaderResourceView* GetTexture();

	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);

private:

	ID3D11ShaderResourceView* m_pSRV;
	MTFont2Bmp m_Font2Bmp;
	unsigned long m_RGB;
	unsigned long m_TexHeight;
	unsigned long m_TexWidth;
};
