//******************************************************************************
//
// MIDITrail / DXPrimitive11
//
// Direct3D 11 primitive renderer.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXPrimitive11.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Static pipeline objects
//******************************************************************************
ID3D11VertexShader*      DXPrimitive11::s_pVS           = nullptr;
ID3D11PixelShader*       DXPrimitive11::s_pPS           = nullptr;
ID3D11InputLayout*       DXPrimitive11::s_pLayout        = nullptr;
ID3D11Buffer*            DXPrimitive11::s_pConstBuf      = nullptr;
ID3D11SamplerState*      DXPrimitive11::s_pSampler       = nullptr;
ID3D11RasterizerState*   DXPrimitive11::s_pRasterNoCull  = nullptr;
ID3D11BlendState*        DXPrimitive11::s_pBlend         = nullptr;
ID3D11BlendState*        DXPrimitive11::s_pBlendAdd      = nullptr;
ID3D11DepthStencilState* DXPrimitive11::s_pDepth         = nullptr;
ID3D11DepthStencilState* DXPrimitive11::s_pDepthNoWrite  = nullptr;
std::vector<std::function<void()>> DXPrimitive11::s_DeviceCleanups;


//******************************************************************************
// HLSL shader source
//******************************************************************************
// Replaces DX9 fixed-function: WVP transform + one directional light +
// texture * vertex color. When lighting is disabled (ambient = 1,1,1),
// the output is simply vertex color (optionally modulated by texture).
static const char s_ShaderSrc[] = R"(
cbuffer cb0 : register(b0) {
    float4x4 wvp;
    float4x4 world;
    float4   lightDir;    // xyz = direction toward surface
    float4   ambient;     // rgb = ambient color
    float4   options;     // x = useTexture (0/1)
};

struct VS_IN {
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float4 color  : COLOR;
    float2 uv     : TEXCOORD;
};

struct VS_OUT {
    float4 pos    : SV_POSITION;
    float4 color  : COLOR;
    float2 uv     : TEXCOORD;
    float3 normal : NORMAL;
};

VS_OUT VS(VS_IN v) {
    VS_OUT o;
    o.pos    = mul(float4(v.pos, 1.0), wvp);
    o.color  = v.color;
    o.uv     = v.uv;
    o.normal = mul(float4(v.normal, 0.0), world).xyz;
    return o;
}

Texture2D    tex0 : register(t0);
SamplerState sam0 : register(s0);

float4 PS(VS_OUT p) : SV_TARGET {
    float3 n = normalize(p.normal);
    float ndl = saturate(dot(n, -lightDir.xyz));

    float4 c = p.color;

    if (options.y > 0.5) {
        // Bilateral (2-light): DX9 opposing directional lights
        ndl += saturate(dot(n, lightDir.xyz));
        c.rgb = saturate(c.rgb * (ambient.x + options.z * ndl));
    } else {
        // Standard (1-light)
        float3 lighting = ambient.rgb + ndl;
        c.rgb *= lighting;
    }

    if (options.x > 0.5) {
        c *= tex0.Sample(sam0, p.uv);
    }

    return c;
}
)";


//******************************************************************************
// Constructor
//******************************************************************************
DXPrimitive11::DXPrimitive11()
{
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer  = nullptr;
	m_VertexNum     = 0;
	m_IndexNum      = 0;
	m_LightEnable   = true;
	m_BilateralLighting = false;
	m_DiffuseLevel  = 1.0f;
	m_pSRV          = nullptr;
	m_Additive      = false;
	m_pCustomBlend  = nullptr;
	m_DepthWrite    = true;
	m_Topology      = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
	m_Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
}

//******************************************************************************
// Destructor
//******************************************************************************
DXPrimitive11::~DXPrimitive11()
{
	Release();
}

//******************************************************************************
// Release per-instance resources
//******************************************************************************
void DXPrimitive11::Release()
{
	if (m_pVertexBuffer) { m_pVertexBuffer->Release(); m_pVertexBuffer = nullptr; }
	if (m_pIndexBuffer)  { m_pIndexBuffer->Release();  m_pIndexBuffer  = nullptr; }
	m_VertexNum = 0;
	m_IndexNum  = 0;
	m_pSRV      = nullptr;
}

//******************************************************************************
// Initialize shared pipeline (once per device)
//******************************************************************************
int DXPrimitive11::InitPipeline(ID3D11Device* pDevice)
{
	HRESULT hr;

	if (s_pVS != nullptr) return 0;

	// Compile vertex shader
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pErrBlob = nullptr;
	hr = D3DCompile(s_ShaderSrc, sizeof(s_ShaderSrc), "DXPrimitive11",
	                nullptr, nullptr, "VS", "vs_4_0", 0, 0, &pVSBlob, &pErrBlob);
	if (FAILED(hr)) {
		if (pErrBlob) pErrBlob->Release();
		return YN_SET_ERR("Shader compile error (VS).", hr, 0);
	}

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
	                                  pVSBlob->GetBufferSize(), nullptr, &s_pVS);
	if (FAILED(hr)) {
		pVSBlob->Release();
		return YN_SET_ERR("CreateVertexShader failed.", hr, 0);
	}

	// Input layout matching DXPRIMITIVE11_VERTEX
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,     0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = pDevice->CreateInputLayout(layout, _countof(layout),
	                                 pVSBlob->GetBufferPointer(),
	                                 pVSBlob->GetBufferSize(), &s_pLayout);
	pVSBlob->Release();
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateInputLayout failed.", hr, 0);
	}

	// Compile pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = D3DCompile(s_ShaderSrc, sizeof(s_ShaderSrc), "DXPrimitive11",
	                nullptr, nullptr, "PS", "ps_4_0", 0, 0, &pPSBlob, &pErrBlob);
	if (FAILED(hr)) {
		if (pErrBlob) pErrBlob->Release();
		return YN_SET_ERR("Shader compile error (PS).", hr, 0);
	}

	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
	                                 pPSBlob->GetBufferSize(), nullptr, &s_pPS);
	pPSBlob->Release();
	if (FAILED(hr)) {
		return YN_SET_ERR("CreatePixelShader failed.", hr, 0);
	}

	// Constant buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth      = sizeof(DXPRIMITIVE11_CBUFFER);
	cbDesc.Usage           = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags       = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;
	hr = pDevice->CreateBuffer(&cbDesc, nullptr, &s_pConstBuf);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateBuffer (CB) failed.", hr, 0);
	}

	// Sampler (linear filter, wrap)
	D3D11_SAMPLER_DESC samDesc = {};
	samDesc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = pDevice->CreateSamplerState(&samDesc, &s_pSampler);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateSamplerState failed.", hr, 0);
	}

	// Rasterizer: no backface culling (DX9 used D3DCULL_NONE for most scenes)
	D3D11_RASTERIZER_DESC rasDesc = {};
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_NONE;
	rasDesc.DepthClipEnable = TRUE;
	hr = pDevice->CreateRasterizerState(&rasDesc, &s_pRasterNoCull);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateRasterizerState failed.", hr, 0);
	}

	// Blend state: standard alpha blending (SrcAlpha / InvSrcAlpha)
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable    = TRUE;
	blendDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = pDevice->CreateBlendState(&blendDesc, &s_pBlend);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateBlendState failed.", hr, 0);
	}

	// Additive blend (SrcAlpha / One) for glow effects like ripple
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	hr = pDevice->CreateBlendState(&blendDesc, &s_pBlendAdd);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateBlendState (additive) failed.", hr, 0);
	}

	// Depth-stencil: normal (test + write)
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable    = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;
	hr = pDevice->CreateDepthStencilState(&dsDesc, &s_pDepth);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateDepthStencilState failed.", hr, 0);
	}

	// Depth-stencil: test only, no write (for backdrop grid that should not
	// occlude notes drawn later)
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = pDevice->CreateDepthStencilState(&dsDesc, &s_pDepthNoWrite);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateDepthStencilState (no write) failed.", hr, 0);
	}

	RegisterDeviceCleanup([]{ DXPrimitive11::ReleasePipeline(); });

	return 0;
}

//******************************************************************************
// Release shared pipeline
//******************************************************************************
void DXPrimitive11::ReleasePipeline()
{
	if (s_pVS)           { s_pVS->Release();           s_pVS = nullptr; }
	if (s_pPS)           { s_pPS->Release();           s_pPS = nullptr; }
	if (s_pLayout)       { s_pLayout->Release();       s_pLayout = nullptr; }
	if (s_pConstBuf)     { s_pConstBuf->Release();     s_pConstBuf = nullptr; }
	if (s_pSampler)      { s_pSampler->Release();      s_pSampler = nullptr; }
	if (s_pRasterNoCull) { s_pRasterNoCull->Release(); s_pRasterNoCull = nullptr; }
	if (s_pBlend)        { s_pBlend->Release();        s_pBlend = nullptr; }
	if (s_pBlendAdd)     { s_pBlendAdd->Release();     s_pBlendAdd = nullptr; }
	if (s_pDepth)        { s_pDepth->Release();        s_pDepth = nullptr; }
	if (s_pDepthNoWrite) { s_pDepthNoWrite->Release(); s_pDepthNoWrite = nullptr; }
}

//******************************************************************************
// Register pipeline cleanup (called from other Pipeline owners' InitPipeline)
//******************************************************************************
void DXPrimitive11::RegisterDeviceCleanup(std::function<void()> cleanup)
{
	s_DeviceCleanups.push_back(cleanup);
}

//******************************************************************************
// Release all registered pipelines
//******************************************************************************
void DXPrimitive11::ReleaseAllDeviceResources()
{
	for (auto& cleanup : s_DeviceCleanups) {
		cleanup();
	}
	s_DeviceCleanups.clear();
}

//******************************************************************************
// Create vertex buffer
//******************************************************************************
int DXPrimitive11::CreateVertexBuffer(
		ID3D11Device* pDevice,
		unsigned long vertexNum
	)
{
	if (pDevice == nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}
	if (m_pVertexBuffer != nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}
	if (vertexNum == 0) {
		m_VertexNum = 0;
		return 0;
	}

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth      = sizeof(DXPRIMITIVE11_VERTEX) * vertexNum;
	desc.Usage           = D3D11_USAGE_DYNAMIC;
	desc.BindFlags       = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, &m_pVertexBuffer);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateBuffer (VB) failed.", hr, vertexNum);
	}

	m_VertexNum = vertexNum;
	return 0;
}

//******************************************************************************
// Create index buffer
//******************************************************************************
int DXPrimitive11::CreateIndexBuffer(
		ID3D11Device* pDevice,
		unsigned long indexNum
	)
{
	if (pDevice == nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}
	if (m_pIndexBuffer != nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}
	if (indexNum == 0) {
		m_IndexNum = 0;
		return 0;
	}

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth      = sizeof(unsigned long) * indexNum;
	desc.Usage           = D3D11_USAGE_DYNAMIC;
	desc.BindFlags       = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, &m_pIndexBuffer);
	if (FAILED(hr)) {
		return YN_SET_ERR("CreateBuffer (IB) failed.", hr, indexNum);
	}

	m_IndexNum = indexNum;
	return 0;
}

//******************************************************************************
// Lock / Unlock vertex buffer (Map/Unmap with DISCARD)
//******************************************************************************
int DXPrimitive11::LockVertex(
		ID3D11DeviceContext* pContext,
		DXPRIMITIVE11_VERTEX** ppVertex
	)
{
	if (m_pVertexBuffer == nullptr || ppVertex == nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT hr = pContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr)) {
		return YN_SET_ERR("Map (VB) failed.", hr, 0);
	}

	*ppVertex = static_cast<DXPRIMITIVE11_VERTEX*>(mapped.pData);
	return 0;
}

void DXPrimitive11::UnlockVertex(ID3D11DeviceContext* pContext)
{
	if (m_pVertexBuffer != nullptr) {
		pContext->Unmap(m_pVertexBuffer, 0);
	}
}

//******************************************************************************
// Lock / Unlock index buffer
//******************************************************************************
int DXPrimitive11::LockIndex(
		ID3D11DeviceContext* pContext,
		unsigned long** ppIndex
	)
{
	if (m_pIndexBuffer == nullptr || ppIndex == nullptr) {
		return YN_SET_ERR("Program error.", 0, 0);
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT hr = pContext->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr)) {
		return YN_SET_ERR("Map (IB) failed.", hr, 0);
	}

	*ppIndex = static_cast<unsigned long*>(mapped.pData);
	return 0;
}

void DXPrimitive11::UnlockIndex(ID3D11DeviceContext* pContext)
{
	if (m_pIndexBuffer != nullptr) {
		pContext->Unmap(m_pIndexBuffer, 0);
	}
}

//******************************************************************************
// State setters
//******************************************************************************
void DXPrimitive11::SetWorldMatrix(const SimpleMath::Matrix& world)
{
	XMStoreFloat4x4(&m_World, world);
}

void DXPrimitive11::SetMaterialAmbient(float r, float g, float b)
{
	m_Ambient = XMFLOAT4(r, g, b, 1.0f);
}

void DXPrimitive11::SetLightEnable(bool enable)
{
	m_LightEnable = enable;
}

void DXPrimitive11::SetBilateralLighting(bool enable, float diffuseLevel)
{
	m_BilateralLighting = enable;
	m_DiffuseLevel = diffuseLevel;
}

void DXPrimitive11::SetTexture(ID3D11ShaderResourceView* pSRV)
{
	m_pSRV = pSRV;
}

void DXPrimitive11::SetAdditiveBlend(bool additive)
{
	m_Additive = additive;
	m_pCustomBlend = nullptr;
}

void DXPrimitive11::SetCustomBlendState(ID3D11BlendState* pBlend)
{
	m_pCustomBlend = pBlend;
}

void DXPrimitive11::SetDepthWrite(bool write)
{
	m_DepthWrite = write;
}

void DXPrimitive11::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_Topology = topology;
}

//******************************************************************************
// Draw
//******************************************************************************
int DXPrimitive11::Draw(
		ID3D11DeviceContext* pContext,
		const SimpleMath::Matrix& viewProj,
		const SimpleMath::Vector4& lightDir,
		int drawPrimitiveNum,
		int startPrimitiveNum
	)
{
	if (m_pVertexBuffer == nullptr) return 0;
	if (s_pVS == nullptr) {
		return YN_SET_ERR("Pipeline not initialized.", 0, 0);
	}

	// Build constant buffer
	XMMATRIX worldMat = XMLoadFloat4x4(&m_World);
	XMMATRIX vpMat    = static_cast<XMMATRIX>(viewProj);
	XMMATRIX wvpMat   = worldMat * vpMat;

	DXPRIMITIVE11_CBUFFER cb;
	XMStoreFloat4x4(&cb.wvp,   XMMatrixTranspose(wvpMat));
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMat));
	cb.lightDir = static_cast<XMFLOAT4>(lightDir);
	cb.options  = XMFLOAT4(m_pSRV ? 1.0f : 0.0f,
	                       (m_BilateralLighting && m_LightEnable) ? 1.0f : 0.0f,
	                       m_DiffuseLevel, 0.0f);

	if (m_LightEnable) {
		cb.ambient = m_Ambient;
	}
	else {
		// No lighting: ambient = 1 makes lit = color * (1 + 0) = color
		cb.ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.lightDir = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// Upload constant buffer
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = pContext->Map(s_pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr)) {
		return YN_SET_ERR("Map (CB) failed.", hr, 0);
	}
	memcpy(mapped.pData, &cb, sizeof(cb));
	pContext->Unmap(s_pConstBuf, 0);

	// Set pipeline state
	pContext->IASetInputLayout(s_pLayout);
	pContext->IASetPrimitiveTopology(m_Topology);

	UINT stride = sizeof(DXPRIMITIVE11_VERTEX);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	if (m_pIndexBuffer) {
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}

	pContext->VSSetShader(s_pVS, nullptr, 0);
	pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf);
	pContext->PSSetShader(s_pPS, nullptr, 0);
	pContext->PSSetConstantBuffers(0, 1, &s_pConstBuf);
	pContext->PSSetSamplers(0, 1, &s_pSampler);

	if (m_pSRV) {
		pContext->PSSetShaderResources(0, 1, &m_pSRV);
	}

	pContext->RSSetState(s_pRasterNoCull);

	float blendFactor[4] = { 0, 0, 0, 0 };
	ID3D11BlendState* pBlend = m_pCustomBlend ? m_pCustomBlend
	                         : (m_Additive ? s_pBlendAdd : s_pBlend);
	pContext->OMSetBlendState(pBlend, blendFactor, 0xFFFFFFFF);
	pContext->OMSetDepthStencilState(m_DepthWrite ? s_pDepth : s_pDepthNoWrite, 0);

	// Determine draw count
	unsigned long totalIndices = (m_pIndexBuffer != nullptr) ? m_IndexNum : m_VertexNum;
	unsigned long startIndex = 0;
	unsigned long drawCount  = totalIndices;

	if (m_Topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
		startIndex = (unsigned long)startPrimitiveNum * 3;
		if (drawPrimitiveNum >= 0) {
			drawCount = (unsigned long)drawPrimitiveNum * 3;
		}
	}
	else if (m_Topology == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {
		startIndex = (unsigned long)startPrimitiveNum * 2;
		if (drawPrimitiveNum >= 0) {
			drawCount = (unsigned long)drawPrimitiveNum * 2;
		}
	}
	else if (m_Topology == D3D11_PRIMITIVE_TOPOLOGY_POINTLIST) {
		startIndex = (unsigned long)startPrimitiveNum;
		if (drawPrimitiveNum >= 0) {
			drawCount = (unsigned long)drawPrimitiveNum;
		}
	}

	// Issue draw call
	if (m_pIndexBuffer != nullptr) {
		pContext->DrawIndexed(drawCount, startIndex, 0);
	}
	else {
		pContext->Draw(drawCount, startIndex);
	}

	return 0;
}
