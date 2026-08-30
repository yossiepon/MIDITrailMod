//******************************************************************************
//
// MIDITrail / MTFont2Bmp
//
// Font-to-bitmap rasterizer.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTFont2Bmp.h"
#include <new>

using namespace YNBaseLib;


//******************************************************************************
// Constructor
//******************************************************************************
MTFont2Bmp::MTFont2Bmp(void)
{
	m_FontName[0] = L'\0';
	m_FontSize = 0;
	m_isForceFixedPitch = false;
	m_hFont = NULL;
	m_pBmpBuf = NULL;
	ZeroMemory(&m_TextMetric, sizeof(TEXTMETRIC));
}

//******************************************************************************
// Destructor
//******************************************************************************
MTFont2Bmp::~MTFont2Bmp(void)
{
	Clear();
}

//******************************************************************************
// Clear
//******************************************************************************
void MTFont2Bmp::Clear()
{
	MTGlyphBmpList::iterator itr;

	if (m_hFont != NULL) {
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}

	if(!m_GlyphBmpList.empty()) {

		for (itr = m_GlyphBmpList.begin(); itr != m_GlyphBmpList.end(); itr++) {

			if(itr->pBmp != NULL) {

				delete [] (itr->pBmp);

			}

		}
		m_GlyphBmpList.clear();

	}

	if(m_pBmpBuf != NULL) {

		delete [] m_pBmpBuf;
		m_pBmpBuf = NULL;

	}
}

//******************************************************************************
// Set font
//******************************************************************************
int MTFont2Bmp::SetFont(
		const WCHAR* pFontName,
		unsigned long fontSize,
		bool isForceFixedPitch
	)
{
	int result = 0;
	errno_t eresult = 0;

	eresult = wcscpy_s(m_FontName, LF_FACESIZE, pFontName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_FontSize = fontSize;
	m_isForceFixedPitch = isForceFixedPitch;

EXIT:;
	return result;
}

//******************************************************************************
// Create bitmap
//******************************************************************************
int MTFont2Bmp::CreateBmp(
		const WCHAR* pStr
	)
{
	int result = 0;

	Clear();

	//Create logical font
	result = _CreateLogFont();
	if (result != 0) goto EXIT;

	//Create the glyph BMP list
	result = _CreateGlyphBmpList(pStr);
	if (result != 0) goto EXIT;

	//Create the buffer for the entire string
	result = _CreateBmpBuf();
	if (result != 0) goto EXIT;

	//Write the glyph BMP into the buffer
	result = _WriteGlyphToBmpBuf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get bitmap size
//******************************************************************************
void MTFont2Bmp::GetBmpSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	*pHeight = m_BmpHeight;
	*pWidth = m_BmpWidth;
}

//******************************************************************************
// Get bitmap pixel value
//******************************************************************************
BYTE MTFont2Bmp::GetBmpPixcel(
		unsigned long x,
		unsigned long y
	)
{
	BYTE pixcel = 0xFF;

	if (m_pBmpBuf == NULL) goto EXIT;

	if ((x >= m_BmpWidth) || (y >= m_BmpHeight)) goto EXIT;

	pixcel = m_pBmpBuf[y*m_BmpWidth + x];

EXIT:;
	return pixcel;
}

//******************************************************************************
// Create logical font
//******************************************************************************
int MTFont2Bmp::_CreateLogFont()
{
	int result = 0;
	LOGFONTW logfont;

	//Build the logical font info
	ZeroMemory(&logfont, sizeof(LOGFONTW));
	logfont.lfHeight         = m_FontSize;			//Height
	logfont.lfWidth          = 0;					//Width
	logfont.lfEscapement     = 0;					//Angle
	logfont.lfOrientation    = 0;					//Angle
	logfont.lfWeight         = 0;					//Weight
	logfont.lfItalic         = FALSE;				//Italic
	logfont.lfUnderline      = FALSE;				//Underline
	logfont.lfStrikeOut      = FALSE;				//Strikeout
	logfont.lfCharSet        = DEFAULT_CHARSET;		//Character set
	logfont.lfOutPrecision   = OUT_TT_ONLY_PRECIS;	//Output precision: use TrueType fonts (falls back to default behavior if unavailable)
	logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;	//Clipping precision: default
	logfont.lfQuality        = PROOF_QUALITY;		//Quality: prioritize rendering quality over font attributes
	logfont.lfPitchAndFamily = DEFAULT_PITCH		//Pitch: default
								| FF_DONTCARE;		//Family: generic family
	wcscpy_s(logfont.lfFaceName, LF_FACESIZE, m_FontName);

	if (m_isForceFixedPitch) {
		logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	}

	//Create the logical font
	m_hFont = CreateFontIndirectW(&logfont);
	if (m_hFont == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Create glyph bitmap
//******************************************************************************
int MTFont2Bmp::_CreateGlyphBmp(
		WCHAR char1,
		WCHAR char2,
		bool isSurrogatePair,
		MTGlyphBmp* pGlyphBmp
	)
{
	int result = 0;
	HDC hDC = NULL;
	BOOL bresult = FALSE;
	HFONT hOldFont = NULL;
	unsigned long size = 0;
	unsigned char* pBuf = NULL;
	GLYPHMETRICS glyphMetric;
	CONST MAT2 mat = {{0,1},{0,0},{0,0},{0,1}};
	GCP_RESULTSW gcp;
	WCHAR str[2];
	WCHAR buff[2];
	DWORD dwresult = 0;
	DWORD flag = 0;
	char32_t code = 0;

	//Get the device context
	hDC = GetDC(NULL);
	if (hDC == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Set the logical font into the device context
	hOldFont = (HFONT)SelectObject(hDC, m_hFont);
	if (hOldFont == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}

	//Get the text metrics
	bresult = GetTextMetrics(hDC, &m_TextMetric);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}

	//For a surrogate pair, get the glyph index
	if (isSurrogatePair) {
		str[0] = char1;
		str[1] = char2;
		memset(&gcp, 0, sizeof(GCP_RESULTSW));
		gcp.lStructSize = sizeof(GCP_RESULTSW);
		gcp.lpGlyphs = buff;
		gcp.nGlyphs = 2;
		dwresult = GetCharacterPlacementW(
							hDC,			//Device context
							str,			//The string to process (need not be null-terminated)
							gcp.nGlyphs,	//String length
							0,				//Maximum extent over which the string is processed (logical units)
							&gcp,			//Destination for the result
							GCP_GLYPHSHAPE	//Flags
						);
		if (dwresult == 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		code = gcp.lpGlyphs[0];
		flag = GGO_GLYPH_INDEX;

	}
	else {
		code = char1;
		flag = 0;
	}

	//Get the buffer size needed to create the bitmap
	size = GetGlyphOutlineW(
					hDC,				//Device context
					code,				//Character
					GGO_GRAY4_BITMAP | flag,	//Format: bitmap (17 gray levels)
					&glyphMetric,		//Glyph metrics: receives info about the created character cell
					0,					//Buffer size: pass zero to obtain the required size
					NULL,				//Buffer address
					&mat				//Transform matrix
				);
	if (size == GDI_ERROR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}
	
	//Do not create a bitmap for whitespace characters
	if (size == 0) {
		//Glyph bitmap info
		pGlyphBmp->glyphMetric = glyphMetric;
		pGlyphBmp->bmpHeight   = 0;
		pGlyphBmp->bmpWidth    = 0;
		pGlyphBmp->pBmp        = NULL;
	}
	//Create a bitmap for non-whitespace characters
	else {

		//Allocate the memory needed to create the bitmap
		try {
			pBuf = new BYTE[size];
		}
		catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", size, 0);
			goto EXIT;
		}

		//Create the TrueType font bitmap
		size = GetGlyphOutlineW(
						hDC,				//Device context
						code,				//Character
						GGO_GRAY4_BITMAP | flag,	//Format: bitmap (17 gray levels)
						&glyphMetric,		//Glyph metrics: receives info about the created character cell
						size,				//Buffer size
						pBuf,				//Buffer address
						&mat				//Transform matrix
					);
		if (size == GDI_ERROR) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		//Glyph bitmap info: keep in mind the bitmap width must be a multiple of 4
		pGlyphBmp->glyphMetric = glyphMetric;
		pGlyphBmp->bmpHeight   = glyphMetric.gmBlackBoxY;
		pGlyphBmp->bmpWidth    = glyphMetric.gmBlackBoxX + (4 - (glyphMetric.gmBlackBoxX % 4)) % 4;
		pGlyphBmp->pBmp        = pBuf;
	}
	pBuf = NULL;

EXIT:;
	delete [] pBuf;
	if (hDC != NULL) {
		if (hOldFont != NULL) {
			SelectObject(hDC, hOldFont);
		}
		ReleaseDC(NULL, hDC);
	}
	return result;
}

//******************************************************************************
// Create glyph bitmap list
//******************************************************************************
int MTFont2Bmp::_CreateGlyphBmpList(
		const WCHAR* pStr
	)
{
	int result = 0;
	WCHAR char1 = 0;
	WCHAR char2 = 0;
	bool isSurrogatePair = false;
	MTGlyphBmp glyphBmp;

	WCHAR* ptr = (WCHAR*)pStr;
	
	//Create a glyph bitmap for each character
	while (ptr[0] != L'\0') {
		char1 = ptr[0];
		char2 = ptr[1];
		if (IS_HIGH_SURROGATE(char1) && IS_LOW_SURROGATE(char2)) {
			//For a surrogate pair
			isSurrogatePair = true;
			ptr += 2;
		}
		else {
			isSurrogatePair = false;
			ptr += 1;
		}

		//Create glyph bitmap
		result = _CreateGlyphBmp(char1, char2, isSurrogatePair, &glyphBmp);
		if (result != 0) goto EXIT;

		//Register into the string list
		m_GlyphBmpList.push_back(glyphBmp);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Create string buffer
//******************************************************************************
int MTFont2Bmp::_CreateBmpBuf()
{
	int result = 0;
	MTGlyphBmpList::iterator itr;
	
	//            |<- gmCellIncX ->|
	//  ----------0----------------+----->x
	//   ^  ^     |  |<---bx--->|  |
	//   |  | ----|--@----------+--|----  @ = (gmptGlyphOrigin.x, ta-gy)
	//   |  |  ^  |  | *       *|  | ^
	//   |  |  |  |  |  *     * |  | |    th: tmHeight
	//   th ta gy |  |   *   *  |  | by   ta: tmAscent
	//   |  |  |  |  |    * *   |  | |    gy: gmptGlyphOrigin.y
	//   |  v  v  |  |     *    |  | |    bx: gmBlackBoxX
	//   | =======|==|=== * ====|==| |    by: gmBlackBoxY
	//   |        |  |   *      |  | v    ==: base line
	//   |        |--+----------+--|----
	//   v        |  |          |  |
	//  ----------+----------------+
	//            |
	//            v
	//            y

	//Height
	m_BmpHeight = m_TextMetric.tmHeight;

	//Width: account for glyph overhang beyond cell advance
	m_BmpWidth = 0;

	if(!m_GlyphBmpList.empty()) {
		unsigned long offset = 0;
		unsigned long maxExtent = 0;

		for (itr = m_GlyphBmpList.begin(); itr != m_GlyphBmpList.end(); itr++) {
			if (itr->pBmp != NULL) {
				unsigned long glyphEnd = offset
					+ (unsigned long)(itr->glyphMetric.gmptGlyphOrigin.x)
					+ itr->glyphMetric.gmBlackBoxX;
				if (glyphEnd > maxExtent) {
					maxExtent = glyphEnd;
				}
			}
			offset += (itr->glyphMetric.gmCellIncX);
		}

		m_BmpWidth = (maxExtent > offset) ? maxExtent : offset;
	}

	//Round the width up to a multiple of 4
	m_BmpWidth = m_BmpWidth + ((4 - (m_BmpWidth % 4)) % 4);

	//Clip if it exceeds the typical size allowed for a texture
	if (m_BmpWidth > MTFONT2BMP_MAX_BMP_WIDTH) {
		m_BmpWidth = MTFONT2BMP_MAX_BMP_WIDTH;
	}

	//Create the bitmap buffer
	try {
		m_pBmpBuf = new BYTE[(m_BmpHeight * m_BmpWidth)];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", m_BmpHeight, m_BmpWidth);
		goto EXIT;
	}
	ZeroMemory(m_pBmpBuf, (m_BmpHeight * m_BmpWidth));

EXIT:;
	return result;
}

//******************************************************************************
// Write glyph bitmaps into the bitmap buffer
//******************************************************************************
int MTFont2Bmp::_WriteGlyphToBmpBuf()
{
	int result = 0;
	MTGlyphBmpList::iterator itr;
	unsigned long offsetX = 0;
	unsigned long x = 0;
	unsigned long y = 0;
	unsigned long destX = 0;
	BYTE* pSrc = NULL;
	BYTE* pDest = NULL;

	if(!m_GlyphBmpList.empty()) {

		for (itr = m_GlyphBmpList.begin(); itr != m_GlyphBmpList.end(); itr++) {

			//Skip empty characters
			if (itr->pBmp == NULL) {
				offsetX += (itr->glyphMetric.gmCellIncX);
				continue;
			}

			//The source glyph bitmap coordinates disregard the 4-multiple size constraint of the bitmap
			//and scan within the actual data range
			for (y = 0; y < (itr->glyphMetric.gmBlackBoxY); y++) {
				//Skip if outside the destination area
				if (y >= m_BmpHeight) continue;

				for (x = 0; x < (itr->glyphMetric.gmBlackBoxX); x++) {

					//Skip if outside the destination area
					destX = offsetX + (itr->glyphMetric.gmptGlyphOrigin.x) + x;
					if (destX >= m_BmpWidth) continue;

					//Source pixel pointer: computed accounting for the bitmap's 4-multiple size constraint
					pSrc = itr->pBmp + (itr->bmpWidth * y) + x;

					//Destination pixel pointer
					pDest = m_pBmpBuf
								+ (m_TextMetric.tmAscent - (itr->glyphMetric.gmptGlyphOrigin.y) + y) * m_BmpWidth
								+ (offsetX + (itr->glyphMetric.gmptGlyphOrigin.x) + x);

					//Check whether the write would go past the allocated buffer
					if (pDest > (m_pBmpBuf + (m_BmpHeight * m_BmpWidth) - 1)) {
						//result = YN_SET_ERR("Program error.", itr->glyphMetric.gmBlackBoxY, itr->glyphMetric.gmBlackBoxX);
						//goto EXIT;
						//Skip instead of treating it as an error
						continue;
					}

					//Copy pixel
					*pDest = *pSrc;
				}
			}
			offsetX += (itr->glyphMetric.gmCellIncX);
		}

	}

//EXIT:;
	return result;
}

