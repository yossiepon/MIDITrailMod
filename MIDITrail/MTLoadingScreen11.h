//******************************************************************************
//
// MIDITrail / MTLoadingScreen11
//
// Loading screen renderer.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include "DXRenderer11.h"
#include "DXPrimitive11.h"
#include "MTStaticCaption11.h"


//******************************************************************************
// Loading screen renderer
//******************************************************************************
class MTLoadingScreen11
{
public:

	MTLoadingScreen11();
	virtual ~MTLoadingScreen11();

	int Create(HWND hWnd, DXRenderer11* pRenderer);
	void Release();

	int Update(float progress, const char* message);

private:

	HWND m_hWnd;
	DXRenderer11* m_pRenderer;
	UINT m_Dpi;
	bool m_isReady;

	DXPrimitive11 m_PanelPrimitive;
	DXPrimitive11 m_SeparatorPrimitive;
	DXPrimitive11 m_BarTrackPrimitive;
	DXPrimitive11 m_BarFillPrimitive;

	MTStaticCaption11 m_TitleCaption;
	MTStaticCaption11 m_MessageCaption;
	MTStaticCaption11 m_PercentCaption;

	ID3D11ShaderResourceView* m_pWhiteTexSRV;

	int _CreateWhiteTexture(ID3D11Device* pDevice);
	int _DrawRect(
			ID3D11DeviceContext* pContext,
			DXPrimitive11* pPrimitive,
			float x, float y, float w, float h,
			DWORD color,
			unsigned int screenW, unsigned int screenH
		);
	void _PumpMessages();

	unsigned long _Scale(unsigned long base);

	void operator=(const MTLoadingScreen11&);
	MTLoadingScreen11(const MTLoadingScreen11&);
};
