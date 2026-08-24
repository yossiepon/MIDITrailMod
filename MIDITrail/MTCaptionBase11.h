//******************************************************************************
//
// MIDITrail / MTCaptionBase11
//
// Common base class for DX11 caption renderers.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTFontTexture11.h"
#include <directxtk/SimpleMath.h>


//******************************************************************************
// DX11 caption base class
//******************************************************************************
class MTCaptionBase11
{
public:

	static const unsigned int SUPERSAMPLE_FACTOR = 2;

	MTCaptionBase11();
	virtual ~MTCaptionBase11();

	virtual void Release();

	void SetColor(DirectX::SimpleMath::Color color);
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);

protected:

	MTFontTexture11 m_FontTexture;
	DXPrimitive11 m_Primitive;
	DirectX::SimpleMath::Color m_Color;
	bool m_isReady;
};
