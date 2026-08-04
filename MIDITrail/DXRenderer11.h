//******************************************************************************
//
// MIDITrail / DXRenderer11
//
// Direct3D 11 renderer.
// DX11 port of DXRenderer: owns the D3D11 device, swap chain, render target,
// and depth buffer. Drives the frame loop by calling Scene->Draw().
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <directxtk/SimpleMath.h>
#include "IMTScene11.h"

class MTFirstPersonCam;


//******************************************************************************
// Error codes
//******************************************************************************
#define DXRENDERER11_ERR_DEVICE_LOST  (100)

#define DX_MULTI_SAMPLE_TYPE_MIN    (2)
#define DX_MULTI_SAMPLE_TYPE_MAX    (16)


//******************************************************************************
// Direct3D 11 renderer
//******************************************************************************
class DXRenderer11
{
public:

	DXRenderer11();
	virtual ~DXRenderer11();

	// Initialize the D3D11 device, swap chain, and render targets.
	int Initialize(HWND hWnd, unsigned long multiSampleCount = 0);

	// Resize render targets when the window size changes.
	int OnResize();

	// Render one frame: clear, call pScene->Draw(), present.
	int RenderScene(IMTScene11* pScene, MTFirstPersonCam* pCamera);

	// Terminate and release all D3D resources.
	void Terminate();

	// Device accessors (for scene/component creation)
	ID3D11Device*        GetDevice()  { return m_pDevice; }
	ID3D11DeviceContext* GetContext() { return m_pContext; }

	// Backbuffer dimensions
	unsigned int GetWidth()  const { return m_Width; }
	unsigned int GetHeight() const { return m_Height; }

	// MSAA support check
	int IsSupportAntialias(unsigned long multiSampleCount, bool* pIsSupport);

	// Background color
	void SetBGColor(unsigned long argb);

private:

	HWND                    m_hWnd;
	ID3D11Device*           m_pDevice;
	ID3D11DeviceContext*    m_pContext;
	IDXGISwapChain*         m_pSwapChain;
	ID3D11RenderTargetView* m_pRTV;
	ID3D11DepthStencilView* m_pDSV;
	ID3D11Texture2D*        m_pDepthTex;
	unsigned int            m_Width;
	unsigned int            m_Height;
	unsigned int            m_SampleCount;
	float                   m_BGColor[4];

	int _CreateRenderTarget();
	void _ReleaseRenderTarget();

	void operator=(const DXRenderer11&);
	DXRenderer11(const DXRenderer11&);
};
