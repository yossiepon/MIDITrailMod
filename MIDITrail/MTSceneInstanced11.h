//******************************************************************************
//
// MIDITrail / MTSceneInstanced11
//
// Base class for GPU-instanced scene components.
// Provides: IMMUTABLE buffer management, binary-search culling with prefix-max,
// and DrawIndexedInstanced helpers.
// Subclasses define their own shaders, InputLayout, and instance data format.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <vector>
#include "MTSceneComponent11.h"


//******************************************************************************
// GPU-instanced scene component base
//******************************************************************************
class MTSceneInstanced11 : public MTSceneComponent11
{
public:

	MTSceneInstanced11();
	virtual ~MTSceneInstanced11();

protected:

	// ---- IMMUTABLE buffer creation helpers ----

	static int CreateImmutableBuffer(
				ID3D11Device* pDevice,
				D3D11_BIND_FLAG bindFlag,
				const void* pData,
				unsigned long byteSize,
				ID3D11Buffer** ppBuffer
			);

	// ---- Binary-search culling (prefix-max) ----

	void BuildCullingArrays(
				const unsigned long* pStartTicks,
				const unsigned long* pEndTicks,
				unsigned long noteCount
			);

	void GetVisibleRange(
				unsigned long tickLow,
				unsigned long tickHigh,
				unsigned long* pLo,
				unsigned long* pHi
			) const;

	unsigned long GetCulledNoteCount() const { return m_CullNoteCount; }

private:

	std::vector<unsigned long> m_StartTicks;
	std::vector<unsigned long> m_MaxEndTicks;
	unsigned long m_CullNoteCount;

	void _RangeForTicks(
				unsigned long tickLow,
				unsigned long tickHigh,
				unsigned long* pLo,
				unsigned long* pHi
			) const;
};
