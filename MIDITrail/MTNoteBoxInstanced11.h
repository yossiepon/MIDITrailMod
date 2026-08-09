//******************************************************************************
//
// MIDITrail / MTNoteBoxInstanced11
//
// GPU-instanced note box renderer for PianoRoll 3D/2D scenes.
// All notes are stored in an IMMUTABLE instance buffer at load time.
// Per-frame CPU work is O(log N) binary search only.
// Active note detection, envelope, pitch-bend, and lighting are computed
// entirely in the vertex shader.
//
// Ring scenes continue to use MTNoteBox11 (legacy DXPrimitive11 path).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteInstancedBase11.h"
#include "MTNoteDesignMod.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// Instance data per note (GPU layout)
//******************************************************************************
struct MTNOTEBOX_INST_INSTANCE {
	float vmin[3];          // box minimum corner (startX, bottomY, rightZ)
	float vmax[3];          // box maximum corner (endX, topY, leftZ)
	unsigned long color;    // D3DCOLOR 0xAARRGGBB
	float pbIndex;          // pitch-bend cbuffer index (port*16 + ch), stored as float
	float alpha;            // note opacity 0..1
	float startTimeMs;      // note start time in ms (for envelope calculation)
	float endTimeMs;        // note end time in ms (for envelope calculation)
};

//******************************************************************************
// Template vertex (shared box geometry)
//******************************************************************************
struct MTNOTEBOX_INST_TEMPLATE_VERTEX {
	float corner[3];    // 0 or 1 mask: selects vmin/vmax per axis
	float normal[3];    // face normal
};


//******************************************************************************
// GPU-instanced note box renderer
//******************************************************************************
class MTNoteBoxInstanced11 : public MTNoteInstancedBase11
{
public:

	MTNoteBoxInstanced11();
	virtual ~MTNoteBoxInstanced11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				MTNoteDesignMod* pNoteDesign = NULL
			);
	void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

	void Reset() override;
	// GPU instancing has no per-frame CPU work to skip
	void SetSkipStatus(bool /*isSkipping*/) {}
	void SetLightEnable(bool enable) { m_isLightEnable = enable; }
	unsigned long GetNoteCount() const;

	static int  InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

private:

	// Per-draw constant buffer (HLSL layout)
	struct CBuffer {
		DirectX::XMFLOAT4X4 wvp;
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4   active;    // x=playTimeMSec, y=growFactor, z=whiteRate, w=passIndex
		DirectX::XMFLOAT4   opts;      // x=unused, y/z/w=emissiveRGB
		DirectX::XMFLOAT4   light;     // xyz=lightDir, w=diffuseLevel
		DirectX::XMFLOAT4   lambient;  // x=ambientLevel, y=unused, z=unused, w=lightEnable
		DirectX::XMFLOAT4   envelope;  // x=decayDurMs, y=releaseDurMs, z=decayRatio, w=sustainRatio
		DirectX::XMFLOAT4   pb[32];    // per-(port,ch) pitch-bend Y shift
	};

	MTNoteDesignMod  m_NoteDesignLocal;
	MTNoteDesignMod* m_pNoteDesign;
	MTNoteTracker*   m_pNoteTracker;
	MTNotePitchBend* m_pNotePitchBend;

	ID3D11Buffer* m_pTemplateVB;
	ID3D11Buffer* m_pInstanceVB;
	ID3D11Buffer* m_pIndexBuffer;

	unsigned long m_CurTickTime;
	unsigned long m_PlayTimeMSec;
	bool          m_isLightEnable;
	unsigned long m_NoteCount;
	float         m_XPerTick;

	DirectX::XMFLOAT4X4 m_World;

	static ID3D11VertexShader*      s_pVS;
	static ID3D11PixelShader*       s_pPS;
	static ID3D11InputLayout*       s_pLayout;
	static ID3D11Buffer*            s_pConstBuf;

	int _CreateTemplateGeometry(ID3D11Device* pDevice);
	int _CreateInstanceBuffer(ID3D11Device* pDevice);
};
