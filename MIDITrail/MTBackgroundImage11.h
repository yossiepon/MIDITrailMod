//******************************************************************************
//
// MIDITrail / MTBackgroundImage11
//
// DX11 background image renderer.
// Draws a user-specified image filling the screen behind all other content.
//
// Copyright (C) 2016 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "DXPrimitive11.h"
#include "DXTexture11.h"
#include "YNBaseLib.h"
#include <directxtk/SimpleMath.h>

using namespace YNBaseLib;


//******************************************************************************
// DX11 background image renderer
//******************************************************************************
class MTBackgroundImage11 : public MTSceneComponent11
{
public:

	MTBackgroundImage11();
	virtual ~MTBackgroundImage11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	void Release();

	int DrawDX11(ID3D11DeviceContext* pContext);

	void Reset();
	bool IsReady() const { return m_isReady; }

private:

	HWND m_hWnd;
	DXPrimitive11 m_Primitive;
	ID3D11ShaderResourceView* m_pSRV;
	unsigned int m_ImgWidth;
	unsigned int m_ImgHeight;
	bool m_isReady;

	YNConfFile m_ConfFile;

	int _CreateVertices(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	int _InitConfFile();
	int _LoadTexture(ID3D11Device* pDevice);
};
