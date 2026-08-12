//******************************************************************************
//
// MIDITrail / DXPrimitive11
//
// Direct3D 11 primitive renderer.
// DX11 port of DXPrimitive: replaces fixed-function pipeline with a unified
// vertex format and a VS/PS pair that handles WVP transform, one directional
// light, and texture * vertex color.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>


//******************************************************************************
// Unified vertex format
//******************************************************************************
// DX9 used FVF with caller-defined vertex size. DX11 has no fixed-function
// pipeline, so we define a single vertex layout that covers all use cases:
// textured keyboard (needs uv), colored note boxes (needs color + normal),
// and backdrop elements (needs pos + color).
struct DXPRIMITIVE11_VERTEX {
	float         pos[3];      // position
	float         normal[3];   // normal (for directional lighting)
	unsigned long color;       // 0xAARRGGBB (D3DCOLOR layout)
	float         uv[2];      // texture coordinate
};

//******************************************************************************
// Per-draw constant buffer (maps to HLSL cbuffer)
//******************************************************************************
struct DXPRIMITIVE11_CBUFFER {
	DirectX::XMFLOAT4X4  wvp;       // world * view * projection
	DirectX::XMFLOAT4X4  world;     // world matrix (for normal transform)
	DirectX::XMFLOAT4    lightDir;  // xyz = light direction (toward surface)
	DirectX::XMFLOAT4    ambient;   // rgb = ambient color, a = unused
	DirectX::XMFLOAT4    options;   // x = useTexture (0 or 1)
};


//******************************************************************************
// Direct3D 11 primitive renderer
//******************************************************************************
class DXPrimitive11
{
public:

	DXPrimitive11();
	virtual ~DXPrimitive11();

	void Release();

	// Shared pipeline state: shaders, input layout, sampler, rasterizer,
	// blend, and depth-stencil states. Created once per device lifetime.
	static int  InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

	// Geometry buffers (DYNAMIC + DISCARD for per-frame updates)
	int CreateVertexBuffer(ID3D11Device* pDevice, unsigned long vertexNum);
	int CreateIndexBuffer(ID3D11Device* pDevice, unsigned long indexNum);

	// Map/Unmap (replaces DX9 Lock/Unlock)
	int  LockVertex(ID3D11DeviceContext* pContext, DXPRIMITIVE11_VERTEX** ppVertex);
	void UnlockVertex(ID3D11DeviceContext* pContext);
	int  LockIndex(ID3D11DeviceContext* pContext, unsigned long** ppIndex);
	void UnlockIndex(ID3D11DeviceContext* pContext);

	// State setters (replaces DX9 SetMaterial/Transform/SetRenderState)
	void SetWorldMatrix(const DirectX::SimpleMath::Matrix& world);
	void SetMaterialAmbient(float r, float g, float b);
	void SetLightEnable(bool enable);
	void SetBilateralLighting(bool enable, float diffuseLevel = 1.2f);
	void SetTexture(ID3D11ShaderResourceView* pSRV);
	void SetAdditiveBlend(bool additive);
	void SetCustomBlendState(ID3D11BlendState* pBlend);
	void SetDepthWrite(bool write);
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

	// Draw all (or a subset of) primitives.
	int Draw(
			ID3D11DeviceContext* pContext,
			const DirectX::SimpleMath::Matrix& viewProj,
			const DirectX::SimpleMath::Vector4& lightDir,
			int drawPrimitiveNum = -1,
			int startPrimitiveNum = 0
		);

private:

	// Per-instance geometry
	ID3D11Buffer*  m_pVertexBuffer;
	ID3D11Buffer*  m_pIndexBuffer;
	unsigned long  m_VertexNum;
	unsigned long  m_IndexNum;

	// Per-instance draw state
	DirectX::XMFLOAT4X4       m_World;
	DirectX::XMFLOAT4         m_Ambient;
	bool                      m_LightEnable;
	bool                      m_BilateralLighting;
	float                     m_DiffuseLevel;
	ID3D11ShaderResourceView* m_pSRV;
	bool                      m_Additive;
	ID3D11BlendState*         m_pCustomBlend;
	bool                      m_DepthWrite;
	D3D11_PRIMITIVE_TOPOLOGY  m_Topology;

	// Shared pipeline objects (static — one set for all instances)
	static ID3D11VertexShader*     s_pVS;
	static ID3D11PixelShader*      s_pPS;
	static ID3D11InputLayout*      s_pLayout;
	static ID3D11Buffer*           s_pConstBuf;
	static ID3D11SamplerState*     s_pSampler;
	static ID3D11RasterizerState*  s_pRasterNoCull;
	static ID3D11BlendState*       s_pBlend;
	static ID3D11BlendState*       s_pBlendAdd;
	static ID3D11DepthStencilState* s_pDepth;
	static ID3D11DepthStencilState* s_pDepthNoWrite;

	void operator=(const DXPrimitive11&);
	DXPrimitive11(const DXPrimitive11&);
};
