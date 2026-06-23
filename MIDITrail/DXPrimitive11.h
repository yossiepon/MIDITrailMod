//******************************************************************************
//
// MIDITrail / DXPrimitive11
//
// Direct3D 11 primitive renderer (migration target; replaces DXPrimitive)
//
//******************************************************************************

// MEMO:
// Unified vertex (position + normal + diffuse color + uv) covers both the
// keyboard (textured) and the note boxes (colored). One VS/PS pair handles
// WVP transform, a single directional light, and texture x diffuse.

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>


//******************************************************************************
// Unified vertex
//******************************************************************************
struct DXP11_VERTEX {
	float    pos[3];
	float    normal[3];
	unsigned long color;   // D3DCOLOR-style 0xAARRGGBB
	float    uv[2];
};

//******************************************************************************
// Per-draw constants (matches the HLSL cbuffer)
//******************************************************************************
struct DXP11_CONSTANTS {
	DirectX::XMFLOAT4X4 wvp;        // world*view*proj
	DirectX::XMFLOAT4X4 world;      // world (for lighting normal)
	DirectX::XMFLOAT4   lightDir;   // xyz = directional light dir (towards surface)
	DirectX::XMFLOAT4   ambient;    // rgb = ambient, a = unused
	DirectX::XMFLOAT4   options;    // x = useTexture (0/1)
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

	// One-time shared pipeline (shaders / input layout / sampler / constant buffer).
	// Safe to call multiple times; created once per device.
	static int InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

	// Geometry buffers (dynamic so Lock/Unlock can update them per frame)
	int CreateVertexBuffer(ID3D11Device* pDevice, unsigned long vertexNum);
	int CreateIndexBuffer(ID3D11Device* pDevice, unsigned long indexNum);

	// Map/unmap for filling the buffers (whole-buffer, DISCARD)
	int LockVertex(ID3D11DeviceContext* pContext, DXP11_VERTEX** ppVertex);
	void UnlockVertex(ID3D11DeviceContext* pContext);
	int LockIndex(ID3D11DeviceContext* pContext, unsigned long** ppIndex);
	void UnlockIndex(ID3D11DeviceContext* pContext);

	void SetWorldMatrix(const DirectX::XMMATRIX& world);
	void SetMaterialAmbient(float r, float g, float b);
	// DX9's per-scene D3DRS_LIGHTING: off = draw at the plain vertex colour (2D / ring scenes)
	void SetLightEnable(bool enable);
	void SetTexture(ID3D11ShaderResourceView* pSRV);   // NULL = untextured (use vertex color)
	void SetAdditiveBlend(bool isAdditive) { m_Additive = isAdditive; }  // glow (SRCALPHA/ONE)
	void SetLineTopology(bool isLines) { m_LineTopology = isLines; }     // LINELIST instead of TRIANGLELIST
	void SetPointTopology(bool isPoints) { m_PointTopology = isPoints; } // POINTLIST instead of TRIANGLELIST (stars)
	void SetDepthWrite(bool write) { m_DepthWrite = write; }             // false = test but don't write depth (backdrop grid)

	// Draw. viewProj is the camera's view*proj. drawPrimitiveNum<0 => all.
	int Draw(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, int drawPrimitiveNum = -1, int startPrimitiveNum = 0);

private:

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	unsigned long m_VertexNum;
	unsigned long m_IndexNum;
	DirectX::XMFLOAT4X4 m_World;
	DirectX::XMFLOAT4 m_Ambient;
	bool m_LightEnable;
	ID3D11ShaderResourceView* m_pSRV;
	bool m_Additive;
	bool m_LineTopology;
	bool m_PointTopology;
	bool m_DepthWrite;   // false = don't write depth (so an invisible/backdrop primitive can't occlude notes)

	// shared pipeline objects (one set per process/device)
	static ID3D11VertexShader* s_pVS;
	static ID3D11PixelShader* s_pPS;
	static ID3D11InputLayout* s_pLayout;
	static ID3D11Buffer* s_pConstBuf;
	static ID3D11SamplerState* s_pSampler;
	static ID3D11RasterizerState* s_pRasterNoCull;
	static ID3D11BlendState* s_pBlend;
	static ID3D11BlendState* s_pBlendAdd;   // additive (SRCALPHA/ONE) for glow effects
	static ID3D11DepthStencilState* s_pDepth;
	static ID3D11DepthStencilState* s_pDepthNoWrite;   // depth test on, write off

	void operator=(const DXPrimitive11&);
	DXPrimitive11(const DXPrimitive11&);
};
