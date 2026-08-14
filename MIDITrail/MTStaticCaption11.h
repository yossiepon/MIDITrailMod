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

#include "DXPrimitive11.h"
#include "MTFontTexture11.h"
#include <directxtk/SimpleMath.h>


//******************************************************************************
// DX11 static caption renderer
//******************************************************************************
class MTStaticCaption11
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

	void Release();

	void SetColor(DirectX::SimpleMath::Color color);
	void GetTextureSize(unsigned long* pHeight, unsigned long* pWidth);

	int Draw(
			ID3D11DeviceContext* pContext,
			float x, float y,
			float magRate,
			unsigned int screenWidth,
			unsigned int screenHeight
		);

private:

	MTFontTexture11 m_FontTexture;
	DXPrimitive11 m_Primitive;
	DirectX::SimpleMath::Color m_Color;
	bool m_isReady;

	int _BuildQuad(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			float x, float y, float magRate,
			unsigned int screenW, unsigned int screenH
		);
};
