//******************************************************************************
//
// MIDITrail / MTNoteRainInstanced11
//
// GPU-instanced note rain renderer for PianoRoll Rain scenes.
// All notes are stored in an IMMUTABLE instance buffer at load time.
// Per-frame CPU work is O(log N) binary search only.
// Alpha gradient and pitch-bend shift are computed in the vertex shader.
// No lighting, no envelope (rain notes have no active-note grow effect).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneInstanced11.h"
#include "MTNoteDesign.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"
#include "MTPianoKeyboardDesign.h"


//******************************************************************************
// Instance data per note (GPU layout)
//******************************************************************************
struct MTNOTERAIN_INST_INSTANCE {
	float vmin[3];          // quad minimum corner (left X, start Y, Z)
	float vmax[3];          // quad maximum corner (right X, end Y, Z)
	unsigned long color;    // D3DCOLOR 0xAARRGGBB
	float pbIndex;          // pitch-bend cbuffer index (port*16 + ch), stored as float
};

//******************************************************************************
// Template vertex (shared quad geometry)
//******************************************************************************
struct MTNOTERAIN_INST_TEMPLATE_VERTEX {
	float corner[3];    // (x_mask, y_mask, 0): 0=vmin, 1=vmax per axis
};


//******************************************************************************
// GPU-instanced note rain renderer
//******************************************************************************
class MTNoteRainInstanced11 : public MTSceneInstanced11
{
public:

	MTNoteRainInstanced11();
	virtual ~MTNoteRainInstanced11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNotePitchBend* pNotePitchBend
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
	unsigned long GetNoteCount() const;
	float GetPos() const { return m_CurPos; }

	static int  InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

private:

	struct CBuffer {
		DirectX::XMFLOAT4X4 wvp;
		DirectX::XMFLOAT4   pb[32];    // per-(port,ch) pitch-bend X shift
	};

	MTNoteDesign         m_NoteDesign;
	MTPianoKeyboardDesign m_KeyboardDesign;
	MTNotePitchBend*     m_pNotePitchBend;

	ID3D11Buffer* m_pTemplateVB;
	ID3D11Buffer* m_pInstanceVB;
	ID3D11Buffer* m_pIndexBuffer;

	unsigned long m_CurTickTime;
	float         m_CurPos;
	unsigned long m_NoteCount;
	float         m_YPerTick;

	DirectX::XMFLOAT4X4 m_World;

	static ID3D11VertexShader*      s_pVS;
	static ID3D11PixelShader*       s_pPS;
	static ID3D11InputLayout*       s_pLayout;
	static ID3D11Buffer*            s_pConstBuf;
	static ID3D11RasterizerState*   s_pRasterNoCull;
	static ID3D11BlendState*        s_pBlend;
	static ID3D11DepthStencilState* s_pDepth;

	int _CreateTemplateGeometry(ID3D11Device* pDevice);
	int _CreateInstanceBuffer(ID3D11Device* pDevice, SMSeqData* pSeqData);
};
