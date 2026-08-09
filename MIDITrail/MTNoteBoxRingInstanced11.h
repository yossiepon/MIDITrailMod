//******************************************************************************
//
// MIDITrail / MTNoteBoxRingInstanced11
//
// GPU-instanced note renderer for PianoRoll Ring scenes.
// Uses cylindrical coordinate corner mask: instance data stores
// (xStart, xEnd, radius, angle0) and the vertex shader converts
// to Cartesian via cos/sin.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteInstancedBase11.h"
#include "MTNoteDesignRing11.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// Instance data per note (GPU layout, cylindrical coordinates)
//******************************************************************************
struct MTNOTEBOXRING_INST_INSTANCE {
	float xStart;           // time axis start position
	float xEnd;             // time axis end position
	float radius;           // cylinder radius (port/ch offset)
	float angle0;           // base angle in degrees (no PB)
	unsigned long color;    // D3DCOLOR 0xAARRGGBB
	float pbIndex;          // pitch-bend cbuffer index (port*16 + ch)
	float alpha;            // note opacity
	float startTimeMs;      // note start time in ms (for envelope)
	float endTimeMs;        // note end time in ms (for envelope)
};

//******************************************************************************
// Template vertex (shared quad geometry, corner[3] + normal[3])
//******************************************************************************
struct MTNOTEBOXRING_INST_TEMPLATE_VERTEX {
	float corner[3];    // (x_mask, r_mask, angle_mask): cylindrical corner
	float normal[3];    // dummy normal (actual normal computed in VS from angle)
};


//******************************************************************************
// GPU-instanced note ring renderer
//******************************************************************************
class MTNoteBoxRingInstanced11 : public MTNoteInstancedBase11
{
public:

	MTNoteBoxRingInstanced11();
	virtual ~MTNoteBoxRingInstanced11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				MTNoteDesignRing11* pNoteDesign = NULL
			);
	void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

	void Reset() override;
	void SetSkipStatus(bool /*isSkipping*/) {}
	void SetLightEnable(bool enable) { m_isLightEnable = enable; }
	unsigned long GetNoteCount() const;

	static int  InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

private:

	struct CBuffer {
		DirectX::XMFLOAT4X4 wvp;
		DirectX::XMFLOAT4   pb[32];        // per-(port,ch) pitch-bend angle shift (degrees)
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4   active;        // x=playTimeMSec, y=growFactor, z=whiteRate, w=pass
		DirectX::XMFLOAT4   opts;          // x=unused, yzw=emissiveRGB
		DirectX::XMFLOAT4   light;         // xyz=lightDir, w=diffuseLevel
		DirectX::XMFLOAT4   lambient;      // x=ambientLevel, yzw=unused, w=lightEnable
		DirectX::XMFLOAT4   envelope;      // x=decayDurMs, y=releaseDurMs, z=decayRatio, w=sustainRatio
		DirectX::XMFLOAT4   ringParams;    // x=halfNoteWidth, y=halfAngleStep (degrees), z/w=unused
	};

	MTNoteDesignRing11   m_NoteDesignLocal;
	MTNoteDesignRing11*  m_pNoteDesign;
	MTNoteTracker*       m_pNoteTracker;
	MTNotePitchBend*     m_pNotePitchBend;

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
