//******************************************************************************
//
// MIDITrail / DXColorUtil
//
// Color utility class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXColorUtil.h"
#include <stdio.h>

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
DXColorUtil::DXColorUtil() {}
DXColorUtil::~DXColorUtil() {}

//******************************************************************************
// RGBA hex string -> Color
//******************************************************************************
Color DXColorUtil::MakeColorFromHexRGBA(const TCHAR* pHexRGBA)
{
	float cr = 0.0f, cg = 0.0f, cb = 0.0f, ca = 0.0f;
	TCHAR* stopped = NULL;
	TCHAR buf[3] = { 0 };

	if (pHexRGBA == NULL || _tcslen(pHexRGBA) < 8) {
		return Color(0.0f, 0.0f, 0.0f, 0.0f);
	}

	buf[2] = _T('\0');

	buf[0] = pHexRGBA[0]; buf[1] = pHexRGBA[1];
	cr = _tcstol(buf, &stopped, 16) / 255.0f;

	buf[0] = pHexRGBA[2]; buf[1] = pHexRGBA[3];
	cg = _tcstol(buf, &stopped, 16) / 255.0f;

	buf[0] = pHexRGBA[4]; buf[1] = pHexRGBA[5];
	cb = _tcstol(buf, &stopped, 16) / 255.0f;

	buf[0] = pHexRGBA[6]; buf[1] = pHexRGBA[7];
	ca = _tcstol(buf, &stopped, 16) / 255.0f;

	return Color(cr, cg, cb, ca);
}

//******************************************************************************
// RGB hex string -> D3DCOLOR-style unsigned long (0xFFRRGGBB)
//******************************************************************************
unsigned long DXColorUtil::MakeColorFromHexRGB(const TCHAR* pHexRGB)
{
	long cr = 0, cg = 0, cb = 0;
	TCHAR* stopped = NULL;
	TCHAR buf[3] = { 0 };

	if (pHexRGB == NULL || _tcslen(pHexRGB) < 6) {
		return 0xFF000000;
	}

	buf[2] = _T('\0');

	buf[0] = pHexRGB[0]; buf[1] = pHexRGB[1];
	cr = _tcstol(buf, &stopped, 16);

	buf[0] = pHexRGB[2]; buf[1] = pHexRGB[3];
	cg = _tcstol(buf, &stopped, 16);

	buf[0] = pHexRGB[4]; buf[1] = pHexRGB[5];
	cb = _tcstol(buf, &stopped, 16);

	return (0xFF000000 | ((cr & 0xFF) << 16) | ((cg & 0xFF) << 8) | (cb & 0xFF));
}

//******************************************************************************
// Color -> RGBA hex string
//******************************************************************************
void DXColorUtil::MakeHexRGBAFromColor(
		Color color,
		TCHAR* pHexRGBA,
		unsigned long bufSize
	)
{
	_stprintf_s(pHexRGBA, bufSize, _T("%02X%02X%02X%02X"),
				(unsigned long)(color.R() * 255.0f),
				(unsigned long)(color.G() * 255.0f),
				(unsigned long)(color.B() * 255.0f),
				(unsigned long)(color.A() * 255.0f));
}
