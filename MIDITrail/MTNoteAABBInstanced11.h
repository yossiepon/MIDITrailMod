//******************************************************************************
//
// MIDITrail / MTNoteAABBInstanced11
//
// Unified GPU-instanced note renderer for AABB-based scenes
// (PianoRoll 3D, PianoRoll 2D, Rain).
// Uses shader permutations (#define) for feature variation:
//   Roll3D: HAS_LIGHTING + HAS_ENVELOPE (24 vertices, 44B instance, 2-pass)
//   Roll2D: HAS_ENVELOPE               (4 vertices,  44B instance, 2-pass)
//   Rain:   HAS_ALPHA_GRADIENT          (4 vertices,  32B instance, 1-pass)
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteInstancedBase11.h"
#include "MTNoteDesign11.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"
#include "MTPianoKeyboardDesign11.h"


//******************************************************************************
// Rendering mode
//******************************************************************************
enum class MTAABBMode {
	Roll3D,     // HAS_LIGHTING + HAS_ENVELOPE, 24 vertices, 2-pass
	Roll2D,     // HAS_ENVELOPE, 4 vertices, 2-pass
	Rain,       // HAS_ALPHA_GRADIENT, 4 vertices, 1-pass
};


//******************************************************************************
// Instance data (AABB, 44B for Roll, 32B for Rain)
//******************************************************************************
struct MTNOTEAABB_INST_INSTANCE_FULL {
	float vmin[3];
	float vmax[3];
	unsigned long color;
	float pbIndex;
	float alpha;
	float startTimeMs;
	float endTimeMs;
};

struct MTNOTEAABB_INST_INSTANCE_RAIN {
	float vmin[3];
	float vmax[3];
	unsigned long color;
	float pbIndex;
};

//******************************************************************************
// Template vertex (shared: corner[3] + normal[3])
//******************************************************************************
struct MTNOTEAABB_INST_TEMPLATE_VERTEX {
	float corner[3];
	float normal[3];
};


//******************************************************************************
// Unified AABB-based GPU-instanced note renderer
//******************************************************************************
class MTNoteAABBInstanced11 : public MTNoteInstancedBase11
{
public:

	MTNoteAABBInstanced11();
	virtual ~MTNoteAABBInstanced11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				MTAABBMode mode,
				MTNoteDesign11* pNoteDesign = NULL
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
	float GetPos() const { return m_CurPos; }

	static int  InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

private:

	static const int MODE_COUNT = 3;

	// CBuffer for Roll3D (HAS_LIGHTING + HAS_ENVELOPE)
	struct CBufferRoll3D {
		DirectX::XMFLOAT4X4 wvp;
		DirectX::XMFLOAT4   pb[32];
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4   active;
		DirectX::XMFLOAT4   opts;
		DirectX::XMFLOAT4   light;
		DirectX::XMFLOAT4   lambient;
		DirectX::XMFLOAT4   envelope;
	};

	// CBuffer for Roll2D (HAS_ENVELOPE only, no lighting fields)
	struct CBufferRoll2D {
		DirectX::XMFLOAT4X4 wvp;
		DirectX::XMFLOAT4   pb[32];
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4   active;
		DirectX::XMFLOAT4   opts;
		DirectX::XMFLOAT4   envelope;
	};

	// CBuffer for Rain (minimal)
	struct CBufferRain {
		DirectX::XMFLOAT4X4 wvp;
		DirectX::XMFLOAT4   pb[32];
	};

	MTAABBMode           m_Mode;
	MTNoteDesign11      m_NoteDesignLocal;
	MTNoteDesign11*     m_pNoteDesign;
	MTPianoKeyboardDesign11 m_KeyboardDesign;
	MTNoteTracker*       m_pNoteTracker;
	MTNotePitchBend*     m_pNotePitchBend;

	ID3D11Buffer* m_pTemplateVB;
	ID3D11Buffer* m_pInstanceVB;
	ID3D11Buffer* m_pIndexBuffer;

	unsigned long m_CurTickTime;
	unsigned long m_PlayTimeMSec;
	float         m_CurPos;
	bool          m_isLightEnable;
	unsigned long m_NoteCount;
	unsigned long m_IndexCountPerInstance;
	float         m_TickToPos;

	DirectX::XMFLOAT4X4 m_World;

	// Per-mode pipeline (3 permutations)
	static ID3D11VertexShader*      s_pVS[MODE_COUNT];
	static ID3D11PixelShader*       s_pPS[MODE_COUNT];
	static ID3D11InputLayout*       s_pLayout[MODE_COUNT];
	static ID3D11Buffer*            s_pConstBuf[MODE_COUNT];

	int _CreateTemplateGeometry(ID3D11Device* pDevice);
	int _CreateInstanceBuffer(ID3D11Device* pDevice, SMSeqData* pSeqData);
	int _DrawRoll(ID3D11DeviceContext* pContext, const DirectX::SimpleMath::Matrix& viewProj,
	              const DirectX::SimpleMath::Vector4& lightDir);
	int _DrawRain(ID3D11DeviceContext* pContext, const DirectX::SimpleMath::Matrix& viewProj);
};
