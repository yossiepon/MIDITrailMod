//******************************************************************************
//
// MIDITrail / DXRenderer11
//
// Direct3D 11 renderer.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXRenderer11.h"
#include "MTFirstPersonCam.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
DXRenderer11::DXRenderer11()
{
	m_hWnd       = NULL;
	m_pDevice    = nullptr;
	m_pContext   = nullptr;
	m_pSwapChain = nullptr;
	m_pRTV       = nullptr;
	m_pDSV       = nullptr;
	m_pDepthTex  = nullptr;
	m_Width      = 0;
	m_Height     = 0;
	m_SampleCount = 1;
}

//******************************************************************************
// Destructor
//******************************************************************************
DXRenderer11::~DXRenderer11()
{
	Terminate();
}

//******************************************************************************
// Initialize
//******************************************************************************
int DXRenderer11::Initialize(
		HWND hWnd,
		unsigned long multiSampleCount
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	RECT rect;

	m_hWnd = hWnd;

	if (!GetClientRect(m_hWnd, &rect)) {
		return YN_SET_ERR("GetClientRect failed.", GetLastError(), 0);
	}
	m_Width  = rect.right - rect.left;
	m_Height = rect.bottom - rect.top;
	if (m_Width == 0)  m_Width = 1;
	if (m_Height == 0) m_Height = 1;

	// Determine MSAA sample count
	m_SampleCount = 1;
	if (multiSampleCount >= DX_MULTI_SAMPLE_TYPE_MIN &&
	    multiSampleCount <= DX_MULTI_SAMPLE_TYPE_MAX) {
		m_SampleCount = multiSampleCount;
	}

	// Create device and swap chain
	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount       = 1;
	scd.BufferDesc.Width  = m_Width;
	scd.BufferDesc.Height = m_Height;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator   = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = m_hWnd;
	scd.SampleDesc.Count   = m_SampleCount;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D11CreateDeviceAndSwapChain(
				nullptr,                   // default adapter
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,                   // no software rasterizer
				flags,
				featureLevels, _countof(featureLevels),
				D3D11_SDK_VERSION,
				&scd,
				&m_pSwapChain,
				&m_pDevice,
				&featureLevel,
				&m_pContext);

	if (FAILED(hr)) {
		// Fallback: try without MSAA
		if (m_SampleCount > 1) {
			m_SampleCount = 1;
			scd.SampleDesc.Count = 1;
			hr = D3D11CreateDeviceAndSwapChain(
						nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
						featureLevels, _countof(featureLevels),
						D3D11_SDK_VERSION, &scd, &m_pSwapChain,
						&m_pDevice, &featureLevel, &m_pContext);
		}
		if (FAILED(hr)) {
			return YN_SET_ERR("D3D11CreateDeviceAndSwapChain failed.", hr, 0);
		}
	}

	// Validate MSAA support (GPU may not support requested count)
	if (m_SampleCount > 1) {
		UINT qualityLevels = 0;
		hr = m_pDevice->CheckMultisampleQualityLevels(
					DXGI_FORMAT_R8G8B8A8_UNORM, m_SampleCount, &qualityLevels);
		if (FAILED(hr) || qualityLevels == 0) {
			// MSAA not supported at this count; recreate without it
			Terminate();
			m_hWnd = hWnd;
			m_SampleCount = 1;
			scd.SampleDesc.Count = 1;
			hr = D3D11CreateDeviceAndSwapChain(
						nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
						featureLevels, _countof(featureLevels),
						D3D11_SDK_VERSION, &scd, &m_pSwapChain,
						&m_pDevice, &featureLevel, &m_pContext);
			if (FAILED(hr)) {
				return YN_SET_ERR("D3D11CreateDeviceAndSwapChain failed.", hr, 0);
			}
		}
	}

	// Create render target and depth buffer
	result = _CreateRenderTarget();
	if (result != 0) return result;

	return 0;
}

//******************************************************************************
// Create render target view and depth-stencil view
//******************************************************************************
int DXRenderer11::_CreateRenderTarget()
{
	HRESULT hr;

	// Render target view from swap chain back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
	                              reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr)) {
		return YN_SET_ERR("GetBuffer failed.", hr, 0);
	}
	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRTV);
	pBackBuffer->Release();
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateRenderTargetView failed.", hr, 0);
	}

	// Depth-stencil texture (D24_UNORM_S8_UINT, matching MSAA sample count)
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width      = m_Width;
	depthDesc.Height     = m_Height;
	depthDesc.MipLevels  = 1;
	depthDesc.ArraySize  = 1;
	depthDesc.Format     = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count   = m_SampleCount;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage      = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags  = D3D11_BIND_DEPTH_STENCIL;

	hr = m_pDevice->CreateTexture2D(&depthDesc, nullptr, &m_pDepthTex);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateTexture2D (depth) failed.", hr, 0);
	}

	hr = m_pDevice->CreateDepthStencilView(m_pDepthTex, nullptr, &m_pDSV);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateDepthStencilView failed.", hr, 0);
	}

	// Bind render target
	m_pContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);

	// Viewport
	D3D11_VIEWPORT vp = {};
	vp.Width    = static_cast<float>(m_Width);
	vp.Height   = static_cast<float>(m_Height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &vp);

	return 0;
}

//******************************************************************************
// Release render target resources
//******************************************************************************
void DXRenderer11::_ReleaseRenderTarget()
{
	if (m_pContext) {
		m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
	if (m_pDSV)      { m_pDSV->Release();      m_pDSV = nullptr; }
	if (m_pDepthTex) { m_pDepthTex->Release(); m_pDepthTex = nullptr; }
	if (m_pRTV)      { m_pRTV->Release();      m_pRTV = nullptr; }
}

//******************************************************************************
// Resize (call when window size changes)
//******************************************************************************
int DXRenderer11::OnResize()
{
	if (m_pSwapChain == nullptr) return 0;

	RECT rect;
	if (!GetClientRect(m_hWnd, &rect)) {
		return YN_SET_ERR("GetClientRect failed.", GetLastError(), 0);
	}

	unsigned int newW = rect.right - rect.left;
	unsigned int newH = rect.bottom - rect.top;
	if (newW == 0) newW = 1;
	if (newH == 0) newH = 1;
	if (newW == m_Width && newH == m_Height) return 0;

	m_Width  = newW;
	m_Height = newH;

	_ReleaseRenderTarget();

	HRESULT hr = m_pSwapChain->ResizeBuffers(
				1, m_Width, m_Height,
				DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr)) {
		return YN_SET_ERR("ResizeBuffers failed.", hr, 0);
	}

	return _CreateRenderTarget();
}

//******************************************************************************
// Render one frame
//******************************************************************************
int DXRenderer11::RenderScene(
		IMTScene11* pScene,
		MTFirstPersonCam* pCamera
	)
{
	int result = 0;
	HRESULT hr = S_OK;

	if (m_pContext == nullptr || m_pRTV == nullptr || pScene == nullptr) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	// Clear render target and depth buffer
	m_pContext->ClearRenderTargetView(m_pRTV, pScene->GetBGColor());
	m_pContext->ClearDepthStencilView(m_pDSV,
	                                  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
	                                  1.0f, 0);

	// Ensure render target is bound
	m_pContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);

	// Draw scene
	{
		float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
		Matrix viewProj;
		float rollAngle = 0.0f;
		Vector3 camPos(0.0f, 0.0f, 0.0f);

		if (pCamera != nullptr) {
			Matrix view, proj;
			pCamera->GetViewProjection(aspect, &view, &proj);
			viewProj = view * proj;
			pCamera->GetPosition(&camPos);
			rollAngle = pCamera->GetRollAngle();
		}

		result = pScene->Draw(m_pContext, viewProj, rollAngle, camPos);
		if (result != 0) goto EXIT;
	}

	// Present
	hr = m_pSwapChain->Present(1, 0);
	if (FAILED(hr)) {
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
			result = DXRENDERER11_ERR_DEVICE_LOST;
			goto EXIT;
		}
		result = YN_SET_ERR("Present failed.", hr, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MSAA support check
//******************************************************************************
int DXRenderer11::IsSupportAntialias(
		unsigned long multiSampleCount,
		bool* pIsSupport
	)
{
	if (m_pDevice == nullptr || pIsSupport == nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}

	*pIsSupport = false;

	if (multiSampleCount < DX_MULTI_SAMPLE_TYPE_MIN ||
	    multiSampleCount > DX_MULTI_SAMPLE_TYPE_MAX) {
		return 0;
	}

	UINT qualityLevels = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(
				DXGI_FORMAT_R8G8B8A8_UNORM,
				multiSampleCount,
				&qualityLevels);

	if (SUCCEEDED(hr) && qualityLevels > 0) {
		*pIsSupport = true;
	}

	return 0;
}

//******************************************************************************
// Terminate
//******************************************************************************
void DXRenderer11::Terminate()
{
	_ReleaseRenderTarget();

	if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
	if (m_pContext)    { m_pContext->Release();    m_pContext = nullptr; }
	if (m_pDevice)     { m_pDevice->Release();     m_pDevice = nullptr; }

	m_hWnd  = NULL;
	m_Width = 0;
	m_Height = 0;
}
