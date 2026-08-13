//******************************************************************************
//
// MIDITrail / MTNoteInstancedBase11
//
// GPU-instanced note renderer base class.
//
// Copyright (C) 2026 Ced (MIDITrail Mod Mod). All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteInstancedBase11.h"
#include "DXPrimitive11.h"

using namespace YNBaseLib;


//******************************************************************************
// Shared pipeline state storage
//******************************************************************************
ID3D11RasterizerState*   MTNoteInstancedBase11::s_pRasterNoCull = nullptr;
ID3D11BlendState*        MTNoteInstancedBase11::s_pBlend        = nullptr;
ID3D11DepthStencilState* MTNoteInstancedBase11::s_pDepth        = nullptr;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteInstancedBase11::MTNoteInstancedBase11()
{
	m_CullNoteCount = 0;
}

MTNoteInstancedBase11::~MTNoteInstancedBase11()
{
}

//******************************************************************************
// Create an IMMUTABLE buffer (vertex or index)
//******************************************************************************
int MTNoteInstancedBase11::CreateImmutableBuffer(
		ID3D11Device* pDevice,
		D3D11_BIND_FLAG bindFlag,
		const void* pData,
		unsigned long byteSize,
		ID3D11Buffer** ppBuffer
	)
{
	int result = 0;
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = byteSize;
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = bindFlag;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pData;

	hr = pDevice->CreateBuffer(&bd, &initData, ppBuffer);
	if (FAILED(hr)) {
		result = YN_SET_ERR("DirectX API error.", hr, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Build culling arrays (sorted startTick + prefix-max endTick)
//******************************************************************************
void MTNoteInstancedBase11::BuildCullingArrays(
		const unsigned long* pStartTicks,
		const unsigned long* pEndTicks,
		unsigned long noteCount
	)
{
	m_CullNoteCount = noteCount;
	m_StartTicks.resize(noteCount);
	m_MaxEndTicks.resize(noteCount);

	if (noteCount == 0) return;

	unsigned long runningMax = 0;
	for (unsigned long i = 0; i < noteCount; i++) {
		m_StartTicks[i] = pStartTicks[i];
		if (pEndTicks[i] > runningMax) {
			runningMax = pEndTicks[i];
		}
		m_MaxEndTicks[i] = runningMax;
	}
}

//******************************************************************************
// Binary search: find [lo, hi) range of potentially visible notes
//******************************************************************************
void MTNoteInstancedBase11::_RangeForTicks(
		unsigned long tickLow,
		unsigned long tickHigh,
		unsigned long* pLo,
		unsigned long* pHi
	) const
{
	unsigned long n = m_CullNoteCount;

	// Lower bound: find first index where maxEndTick >= tickLow
	// All notes before this index have ended before the visible window.
	unsigned long lo = 0;
	{
		unsigned long left = 0, right = n;
		while (left < right) {
			unsigned long mid = left + (right - left) / 2;
			if (m_MaxEndTicks[mid] < tickLow) {
				left = mid + 1;
			} else {
				right = mid;
			}
		}
		lo = left;
	}

	// Upper bound: find first index where startTick > tickHigh
	// All notes from this index onward haven't started yet.
	unsigned long hi = n;
	{
		unsigned long left = lo, right = n;
		while (left < right) {
			unsigned long mid = left + (right - left) / 2;
			if (m_StartTicks[mid] <= tickHigh) {
				left = mid + 1;
			} else {
				right = mid;
			}
		}
		hi = left;
	}

	*pLo = lo;
	*pHi = hi;
}

//******************************************************************************
// Get visible note range for a tick window
//******************************************************************************
void MTNoteInstancedBase11::GetVisibleRange(
		unsigned long tickLow,
		unsigned long tickHigh,
		unsigned long* pLo,
		unsigned long* pHi
	) const
{
	if (m_CullNoteCount == 0) {
		*pLo = 0;
		*pHi = 0;
		return;
	}
	_RangeForTicks(tickLow, tickHigh, pLo, pHi);
}


//******************************************************************************
// Initialize shared pipeline states
//******************************************************************************
int MTNoteInstancedBase11::InitCommonStates(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;

	if (s_pRasterNoCull != nullptr) return 0;

	// Rasterizer (no cull)
	{
		D3D11_RASTERIZER_DESC rd = {};
		rd.FillMode = D3D11_FILL_SOLID;
		rd.CullMode = D3D11_CULL_NONE;
		rd.DepthClipEnable = TRUE;
		hr = pDevice->CreateRasterizerState(&rd, &s_pRasterNoCull);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	// Blend (alpha blending)
	{
		D3D11_BLEND_DESC bd = {};
		bd.RenderTarget[0].BlendEnable = TRUE;
		bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&bd, &s_pBlend);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	// Depth stencil
	{
		D3D11_DEPTH_STENCIL_DESC dd = {};
		dd.DepthEnable = TRUE;
		dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		hr = pDevice->CreateDepthStencilState(&dd, &s_pDepth);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	DXPrimitive11::RegisterDeviceCleanup([]{ MTNoteInstancedBase11::ReleaseCommonStates(); });

EXIT:;
	return result;
}


//******************************************************************************
// Release shared pipeline states
//******************************************************************************
void MTNoteInstancedBase11::ReleaseCommonStates()
{
	if (s_pRasterNoCull) { s_pRasterNoCull->Release(); s_pRasterNoCull = nullptr; }
	if (s_pBlend)        { s_pBlend->Release();        s_pBlend = nullptr; }
	if (s_pDepth)        { s_pDepth->Release();        s_pDepth = nullptr; }
}


//******************************************************************************
// Bind shared pipeline states to context
//******************************************************************************
void MTNoteInstancedBase11::BindCommonStates(ID3D11DeviceContext* pContext)
{
	pContext->RSSetState(s_pRasterNoCull);
	pContext->OMSetBlendState(s_pBlend, nullptr, 0xFFFFFFFF);
	pContext->OMSetDepthStencilState(s_pDepth, 0);
}


//******************************************************************************
// Fill pitch-bend array for cbuffer
//******************************************************************************
void MTNoteInstancedBase11::FillPitchBendArray(
		float* pbOut,
		MTNotePitchBend* pPB,
		std::function<float(short value, unsigned char sens)> shiftFunc
	)
{
	ZeroMemory(pbOut, sizeof(float) * 128);
	if (pPB == nullptr) return;

	for (unsigned char port = 0; port < 8; port++) {
		for (unsigned char ch = 0; ch < 16; ch++) {
			short pbValue = pPB->GetValue(port, ch);
			unsigned char pbSens = pPB->GetSensitivity(port, ch);
			unsigned int idx = port * 16 + ch;
			pbOut[idx] = shiftFunc(pbValue, pbSens);
		}
	}
}
