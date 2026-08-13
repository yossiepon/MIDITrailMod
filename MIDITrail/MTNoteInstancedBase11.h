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

#pragma once

#include <d3d11.h>
#include <vector>
#include <functional>
#include "MTSceneComponent11.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// GPU-instanced note renderer base
//******************************************************************************
class MTNoteInstancedBase11 : public MTSceneComponent11
{
public:

	MTNoteInstancedBase11();
	virtual ~MTNoteInstancedBase11();

	// ---- Shared pipeline states (all subclasses use identical settings) ----

	static int  InitCommonStates(ID3D11Device* pDevice);
	static void ReleaseCommonStates();

protected:

	// ---- Shared pipeline state accessors ----

	static ID3D11RasterizerState*   GetRasterizerNoCull()  { return s_pRasterNoCull; }
	static ID3D11BlendState*        GetBlendAlpha()        { return s_pBlend; }
	static ID3D11DepthStencilState* GetDepthLessEqual()    { return s_pDepth; }

	void BindCommonStates(ID3D11DeviceContext* pContext);

	// ---- Pitch-bend helpers ----

	static void FillPitchBendArray(
				float* pbOut,
				MTNotePitchBend* pPB,
				std::function<float(short value, unsigned char sens)> shiftFunc
			);

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

	// ---- Shared pipeline state storage ----

	static ID3D11RasterizerState*   s_pRasterNoCull;
	static ID3D11BlendState*        s_pBlend;
	static ID3D11DepthStencilState* s_pDepth;
};
