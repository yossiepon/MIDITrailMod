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

#pragma once

#include <directxtk/SimpleMath.h>
#include <tchar.h>


//******************************************************************************
// Color utility class
//******************************************************************************
class DXColorUtil
{
public:

	// RGBA hex string "RRGGBBAA" -> Color(r, g, b, a) in [0,1]
	static DirectX::SimpleMath::Color MakeColorFromHexRGBA(const TCHAR* pHexRGBA);

	// RGB hex string "RRGGBB" -> D3DCOLOR-style 0xFFRRGGBB (unsigned long)
	static unsigned long MakeColorFromHexRGB(const TCHAR* pHexRGB);

	// Color(r, g, b, a) -> RGBA hex string "RRGGBBAA"
	static void MakeHexRGBAFromColor(
					DirectX::SimpleMath::Color color,
					TCHAR* pHexRGBA,
					unsigned long bufSize
				);

private:

	DXColorUtil();
	virtual ~DXColorUtil();

	void operator=(const DXColorUtil&);
	DXColorUtil(const DXColorUtil&);
};
