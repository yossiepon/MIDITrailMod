//******************************************************************************
//
// MIDITrail / MTFontTexture11
//
// Font texture class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXTexture11.h"
#include "MTFontTexture11.h"

using namespace YNBaseLib;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTFontTexture11::MTFontTexture11()
{
	m_RGB = 0x00FFFFFF;
	m_TexHeight = 0;
	m_TexWidth = 0;
	m_pSRV = NULL;
}

MTFontTexture11::~MTFontTexture11()
{
	Clear();
}

//******************************************************************************
// Clear
//******************************************************************************
void MTFontTexture11::Clear()
{
	if (m_pSRV != NULL) {
		m_pSRV->Release();
		m_pSRV = NULL;
	}
	m_Font2Bmp.Clear();
	m_TexHeight = 0;
	m_TexWidth = 0;
}

//******************************************************************************
// Set font
//******************************************************************************
int MTFontTexture11::SetFont(
		const WCHAR* pFontName,
		unsigned long fontSize,
		unsigned long rgb,
		bool isForceFixedPitch
	)
{
	int result = 0;

	result = m_Font2Bmp.SetFont(pFontName, fontSize, isForceFixedPitch);
	if (result != 0) goto EXIT;

	m_RGB = 0x00FFFFFF & rgb;

EXIT:;
	return result;
}

//******************************************************************************
// Create texture
//******************************************************************************
int MTFontTexture11::CreateTexture(
		ID3D11Device* pDevice,
		const WCHAR* pStr
	)
{
	int result = 0;
	DWORD bmpHeight = 0;
	DWORD bmpWidth = 0;
	unsigned char* pPixels = NULL;
	int grayLevelNum = 17;  // GGO_GRAY4_BITMAP: 17 levels

	if (m_pSRV != NULL) {
		m_pSRV->Release();
		m_pSRV = NULL;
	}

	// Create font bitmap
	result = m_Font2Bmp.CreateBmp(pStr);
	if (result != 0) goto EXIT;

	m_Font2Bmp.GetBmpSize(&bmpHeight, &bmpWidth);

	if (bmpWidth == 0 || bmpHeight == 0) goto EXIT;

	// Build RGBA8 pixel buffer
	try {
		pPixels = new unsigned char[(size_t)bmpWidth * bmpHeight * 4];
	}
	catch (std::bad_alloc&) {
		result = YN_SET_ERR("Could not allocate memory.", bmpWidth, bmpHeight);
		goto EXIT;
	}

	{
		unsigned char colorR = (unsigned char)((m_RGB >> 16) & 0xFF);
		unsigned char colorG = (unsigned char)((m_RGB >>  8) & 0xFF);
		unsigned char colorB = (unsigned char)((m_RGB >>  0) & 0xFF);

		for (DWORD y = 0; y < bmpHeight; y++) {
			for (DWORD x = 0; x < bmpWidth; x++) {
				BYTE bmpPixel = m_Font2Bmp.GetBmpPixcel(x, y);
				unsigned char alpha = (unsigned char)((0xFF * bmpPixel) / (grayLevelNum - 1));

				size_t offset = ((size_t)y * bmpWidth + x) * 4;

				if (bmpPixel == 0) {
					pPixels[offset + 0] = 0;       // R
					pPixels[offset + 1] = 0;       // G
					pPixels[offset + 2] = 0;       // B
					pPixels[offset + 3] = 0;       // A
				}
				else {
					pPixels[offset + 0] = colorR;  // R
					pPixels[offset + 1] = colorG;  // G
					pPixels[offset + 2] = colorB;  // B
					pPixels[offset + 3] = alpha;   // A
				}
			}
		}
	}

	// Build SRV 
	result = DXTexture11::CreateFromRGBA(pDevice, pPixels, bmpWidth, bmpHeight, &m_pSRV);
	if (result != 0) goto EXIT;

	m_TexHeight = bmpHeight;
	m_TexWidth = bmpWidth;

EXIT:;
	delete[] pPixels;
	m_Font2Bmp.Clear();
	return result;
}

//******************************************************************************
// Get texture
//******************************************************************************
ID3D11ShaderResourceView* MTFontTexture11::GetTexture()
{
	return m_pSRV;
}

//******************************************************************************
// Get texture size
//******************************************************************************
void MTFontTexture11::GetTextureSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	*pHeight = m_TexHeight;
	*pWidth = m_TexWidth;
}
