//******************************************************************************
//
// MIDITrail / DXRenderer11
//
// Direct3D 11 renderer.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <dxgi1_6.h>
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

	// Frame begin/end for non-scene rendering (e.g. loading screen)
	int BeginFrame(const float* pClearColor = NULL);
	int EndFrame();

	// Render target accessors (for loading screen)
	ID3D11RenderTargetView* GetRTV() { return m_pRTV; }

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

	int _CreateRenderTarget();
	void _ReleaseRenderTarget();

	void operator=(const DXRenderer11&);
	DXRenderer11(const DXRenderer11&);
};
