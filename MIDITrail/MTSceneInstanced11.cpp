//******************************************************************************
//
// MIDITrail / MTSceneInstanced11
//
// Base class for GPU-instanced scene components.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTSceneInstanced11.h"

using namespace YNBaseLib;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTSceneInstanced11::MTSceneInstanced11()
{
	m_CullNoteCount = 0;
}

MTSceneInstanced11::~MTSceneInstanced11()
{
}

//******************************************************************************
// Create an IMMUTABLE buffer (vertex or index)
//******************************************************************************
int MTSceneInstanced11::CreateImmutableBuffer(
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
void MTSceneInstanced11::BuildCullingArrays(
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
void MTSceneInstanced11::_RangeForTicks(
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
void MTSceneInstanced11::GetVisibleRange(
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
