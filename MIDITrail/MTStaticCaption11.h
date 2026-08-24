//******************************************************************************
//
// MIDITrail / MTStaticCaption11
//
// Static caption renderer.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTCaptionBase11.h"


//******************************************************************************
// DX11 static caption renderer
//******************************************************************************
class MTStaticCaption11 : public MTCaptionBase11
{
public:

	MTStaticCaption11();
	virtual ~MTStaticCaption11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const WCHAR* pFontName,
			unsigned long fontSize,
			const WCHAR* pText
		);

	void Release() override;

	void GetDisplaySize(float magRate, float* pWidth, float* pHeight);

	int Draw(
			ID3D11DeviceContext* pContext,
			float x, float y,
			float magRate,
			unsigned int screenWidth,
			unsigned int screenHeight
		);
};
