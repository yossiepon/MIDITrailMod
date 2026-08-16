//******************************************************************************
//
// MIDITrail / MTFont2Bmp
//
// Font-to-bitmap rasterizer.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Creates a bitmap with the specified string rendered into it.
// Supports only a single line of text; multiple lines are not supported.
// The bitmap size is adjusted so its width is a multiple of 4.

#pragma once

#include <list>

//******************************************************************************
// Parameter definitions
//******************************************************************************
//Maximum bitmap size: D3D11 Feature Level 11 supports up to 16384
#define MTFONT2BMP_MAX_BMP_WIDTH  (4096)

//******************************************************************************
// Font-to-bitmap conversion class
//******************************************************************************
class MTFont2Bmp
{
public:

	//Constructor / Destructor
	MTFont2Bmp(void);
	virtual ~MTFont2Bmp(void);

	//Clear
	void Clear();
	
	//Set font
	//  Pass true for isForceFixedPitch to force a fixed pitch
	int SetFont(const WCHAR* pFontName, unsigned long fontSize, bool isForceFixedPitch = false);

	//Create bitmap
	int CreateBmp(const WCHAR* pStr);
	
	//Get bitmap size
	void GetBmpSize(unsigned long* pHeight, unsigned long* pWidth);
	
	//Get bitmap pixel
	//  Returns a pixel value with 17 gray levels (0x00-0x10)
	//  Returns 0xFF if out of range
	BYTE GetBmpPixcel(unsigned long x, unsigned long y);

private:

	typedef struct {
		GLYPHMETRICS glyphMetric;
		unsigned long bmpHeight;
		unsigned long bmpWidth;
		unsigned char* pBmp;
	} MTGlyphBmp;

	typedef std::list<MTGlyphBmp> MTGlyphBmpList;

private:

	WCHAR m_FontName[LF_FACESIZE];
	unsigned long m_FontSize;
	bool m_isForceFixedPitch;

	HFONT m_hFont;
	TEXTMETRIC m_TextMetric;
	MTGlyphBmpList m_GlyphBmpList;

	BYTE* m_pBmpBuf;
	unsigned long m_BmpHeight;
	unsigned long m_BmpWidth;
	
	int _CreateLogFont();
	int _CreateGlyphBmp(WCHAR char1, WCHAR char2, bool isSurrogatePair, MTGlyphBmp* pGB);
	int _CreateGlyphBmpList(const WCHAR* pStr);
	int _CreateBmpBuf();
	int _WriteGlyphToBmpBuf();

};

