//******************************************************************************
//
// MIDITrail / DXPrimitive11
//
// Direct3D 11 primitive renderer (migration target)
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "DXPrimitive11.h"
#include <d3dcompiler.h>

using namespace YNBaseLib;
using namespace DirectX;

// static pipeline objects
ID3D11VertexShader*      DXPrimitive11::s_pVS = NULL;
ID3D11PixelShader*       DXPrimitive11::s_pPS = NULL;
ID3D11InputLayout*       DXPrimitive11::s_pLayout = NULL;
ID3D11Buffer*            DXPrimitive11::s_pConstBuf = NULL;
ID3D11SamplerState*      DXPrimitive11::s_pSampler = NULL;
ID3D11RasterizerState*   DXPrimitive11::s_pRasterNoCull = NULL;
ID3D11BlendState*        DXPrimitive11::s_pBlend = NULL;
ID3D11BlendState*        DXPrimitive11::s_pBlendAdd = NULL;
ID3D11DepthStencilState* DXPrimitive11::s_pDepth = NULL;
ID3D11DepthStencilState* DXPrimitive11::s_pDepthNoWrite = NULL;

static const char* DXP11_SHADER =
	"cbuffer Constants : register(b0) {\n"
	"  row_major float4x4 g_WVP;\n"
	"  row_major float4x4 g_World;\n"
	"  float4 g_LightDir;\n"
	"  float4 g_Ambient;\n"
	"  float4 g_Options;\n"
	"};\n"
	"Texture2D g_Tex : register(t0);\n"
	"SamplerState g_Samp : register(s0);\n"
	"struct VSIN { float3 pos:POSITION; float3 normal:NORMAL; float4 color:COLOR0; float2 uv:TEXCOORD0; };\n"
	"struct VSOUT { float4 pos:SV_POSITION; float4 color:COLOR0; float2 uv:TEXCOORD0; float3 normal:TEXCOORD1; };\n"
	"VSOUT VSMain(VSIN i) {\n"
	"  VSOUT o;\n"
	"  o.pos = mul(float4(i.pos,1.0), g_WVP);\n"
	"  o.color = i.color;\n"
	"  o.uv = i.uv;\n"
	"  o.normal = mul(float4(i.normal,0.0), g_World).xyz;\n"
	"  return o;\n"
	"}\n"
	"float4 PSMain(VSOUT i) : SV_TARGET {\n"
	"  float4 base0 = i.color;\n"
	"  if (g_Options.x > 0.5) { base0 = g_Tex.Sample(g_Samp, i.uv) * i.color; }\n"
	// ced 20260713: DX9's D3DRS_LIGHTING is a per-scene switch - the 2D and ring scenes
	// turn it off, so their primitives draw at their plain vertex colour. g_Options.y = 0
	// reproduces that; without it the 2D live notes came out shaded, unlike DX9.
	"  if (g_Options.y < 0.5) { return base0; }\n"
	"  float3 n = normalize(i.normal);\n"
	"  float3 L = normalize(g_LightDir.xyz);\n"
	// DX9's 3D scene (PianoRoll3DMod) lit with TWO opposing directional lights
	// (main + back, diffuse 1.2). A single light left every face turned away from
	// the key light on ambient only, so the keyboard looked darker/greyer than DX9.
	// Add the opposing fill light + DX9's 1.2 diffuse so those faces are lit again.
	"  float ndl = saturate(dot(n, -L)) + saturate(dot(n, L));\n"
	"  float3 light = saturate(g_Ambient.rgb + 1.2 * ndl);\n"
	"  return float4(base0.rgb * light, base0.a);\n"
	"}\n";

//******************************************************************************
// Constructor / destructor
//******************************************************************************
DXPrimitive11::DXPrimitive11()
{
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_VertexNum = 0;
	m_IndexNum = 0;
	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
	m_Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_LightEnable = true;   // DX9 D3DRS_LIGHTING; the 2D / ring scenes switch it off
	m_pSRV = NULL;
	m_Additive = false;
	m_LineTopology = false;
	m_PointTopology = false;
	m_DepthWrite = true;
}

DXPrimitive11::~DXPrimitive11()
{
	Release();
}

void DXPrimitive11::Release()
{
	if (m_pIndexBuffer != NULL)  { m_pIndexBuffer->Release();  m_pIndexBuffer = NULL; }
	if (m_pVertexBuffer != NULL) { m_pVertexBuffer->Release(); m_pVertexBuffer = NULL; }
	m_VertexNum = 0;
	m_IndexNum = 0;
	m_pSRV = NULL;  // not owned
}

//******************************************************************************
// Shared pipeline
//******************************************************************************
int DXPrimitive11::InitPipeline(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pPSBlob = NULL;
	ID3DBlob* pErr = NULL;

	if (s_pVS != NULL) return 0;  // already built
	if (pDevice == NULL) return YN_SET_ERR("Program error.", 0, 0);

	UINT flags = 0;
	hr = D3DCompile(DXP11_SHADER, strlen(DXP11_SHADER), NULL, NULL, NULL, "VSMain", "vs_4_0", flags, 0, &pVSBlob, &pErr);
	if (FAILED(hr) || (pVSBlob == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }
	hr = D3DCompile(DXP11_SHADER, strlen(DXP11_SHADER), NULL, NULL, NULL, "PSMain", "ps_4_0", flags, 0, &pPSBlob, &pErr);
	if (FAILED(hr) || (pPSBlob == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &s_pVS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &s_pPS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	{
		D3D11_INPUT_ELEMENT_DESC il[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		hr = pDevice->CreateInputLayout(il, 4, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &s_pLayout);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_BUFFER_DESC cb;
		ZeroMemory(&cb, sizeof(cb));
		cb.ByteWidth = sizeof(DXP11_CONSTANTS);
		cb.Usage = D3D11_USAGE_DYNAMIC;
		cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr = pDevice->CreateBuffer(&cb, NULL, &s_pConstBuf);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_SAMPLER_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.MaxLOD = D3D11_FLOAT32_MAX;
		hr = pDevice->CreateSamplerState(&sd, &s_pSampler);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_RASTERIZER_DESC rd;
		ZeroMemory(&rd, sizeof(rd));
		rd.FillMode = D3D11_FILL_SOLID;
		rd.CullMode = D3D11_CULL_NONE;     // scene used D3DCULL_NONE
		rd.DepthClipEnable = TRUE;
		rd.MultisampleEnable = TRUE;       // MSAA edge antialiasing
		hr = pDevice->CreateRasterizerState(&rd, &s_pRasterNoCull);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_BLEND_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.RenderTarget[0].BlendEnable = TRUE;
		bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		// alpha: standard "over" (src.a + dst.a*(1-src.a)) so an overlay's transparent
		// pixels keep the alpha of what's behind them instead of punching a hole in it
		// (matters for the transparent-video export: dashboard text, ripple glow, grid).
		bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&bd, &s_pBlend);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

		// additive (SRCALPHA / ONE) for glow effects like the note ripple
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		hr = pDevice->CreateBlendState(&bd, &s_pBlendAdd);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_DEPTH_STENCIL_DESC dd;
		ZeroMemory(&dd, sizeof(dd));
		dd.DepthEnable = TRUE;
		dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		hr = pDevice->CreateDepthStencilState(&dd, &s_pDepth);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
		// backdrop variant: still test depth, but do NOT write it, so a fully transparent
		// primitive (e.g. the grid at alpha 0) can never occlude the notes drawn after it.
		dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		hr = pDevice->CreateDepthStencilState(&dd, &s_pDepthNoWrite);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

EXIT:;
	if (pVSBlob != NULL) pVSBlob->Release();
	if (pPSBlob != NULL) pPSBlob->Release();
	if (pErr != NULL) pErr->Release();
	if (result != 0) ReleasePipeline();
	return result;
}

void DXPrimitive11::ReleasePipeline()
{
	if (s_pDepthNoWrite != NULL){ s_pDepthNoWrite->Release();s_pDepthNoWrite = NULL; }
	if (s_pDepth != NULL)       { s_pDepth->Release();       s_pDepth = NULL; }
	if (s_pBlendAdd != NULL)    { s_pBlendAdd->Release();    s_pBlendAdd = NULL; }
	if (s_pBlend != NULL)       { s_pBlend->Release();       s_pBlend = NULL; }
	if (s_pRasterNoCull != NULL){ s_pRasterNoCull->Release();s_pRasterNoCull = NULL; }
	if (s_pSampler != NULL)     { s_pSampler->Release();     s_pSampler = NULL; }
	if (s_pConstBuf != NULL)    { s_pConstBuf->Release();    s_pConstBuf = NULL; }
	if (s_pLayout != NULL)      { s_pLayout->Release();      s_pLayout = NULL; }
	if (s_pPS != NULL)          { s_pPS->Release();          s_pPS = NULL; }
	if (s_pVS != NULL)          { s_pVS->Release();          s_pVS = NULL; }
}

//******************************************************************************
// Geometry buffers (dynamic)
//******************************************************************************
int DXPrimitive11::CreateVertexBuffer(ID3D11Device* pDevice, unsigned long vertexNum)
{
	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC bd;
	if (m_pVertexBuffer != NULL) { m_pVertexBuffer->Release(); m_pVertexBuffer = NULL; }
	if (vertexNum == 0) { m_VertexNum = 0; return 0; }
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(DXP11_VERTEX) * vertexNum;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer);
	if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, vertexNum);
	m_VertexNum = vertexNum;
	return 0;
}

int DXPrimitive11::CreateIndexBuffer(ID3D11Device* pDevice, unsigned long indexNum)
{
	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC bd;
	if (m_pIndexBuffer != NULL) { m_pIndexBuffer->Release(); m_pIndexBuffer = NULL; }
	if (indexNum == 0) { m_IndexNum = 0; return 0; }
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(unsigned long) * indexNum;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = pDevice->CreateBuffer(&bd, NULL, &m_pIndexBuffer);
	if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, indexNum);
	m_IndexNum = indexNum;
	return 0;
}

int DXPrimitive11::LockVertex(ID3D11DeviceContext* pContext, DXP11_VERTEX** ppVertex)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = pContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, 0);
	*ppVertex = (DXP11_VERTEX*)ms.pData;
	return 0;
}
void DXPrimitive11::UnlockVertex(ID3D11DeviceContext* pContext) { pContext->Unmap(m_pVertexBuffer, 0); }

int DXPrimitive11::LockIndex(ID3D11DeviceContext* pContext, unsigned long** ppIndex)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = pContext->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, 0);
	*ppIndex = (unsigned long*)ms.pData;
	return 0;
}
void DXPrimitive11::UnlockIndex(ID3D11DeviceContext* pContext) { pContext->Unmap(m_pIndexBuffer, 0); }

//******************************************************************************
// State setters
//******************************************************************************
void DXPrimitive11::SetWorldMatrix(const XMMATRIX& world) { XMStoreFloat4x4(&m_World, world); }
void DXPrimitive11::SetMaterialAmbient(float r, float g, float b) { m_Ambient = XMFLOAT4(r, g, b, 1.0f); }
void DXPrimitive11::SetLightEnable(bool enable) { m_LightEnable = enable; }
void DXPrimitive11::SetTexture(ID3D11ShaderResourceView* pSRV) { m_pSRV = pSRV; }

//******************************************************************************
// Draw
//******************************************************************************
int DXPrimitive11::Draw(ID3D11DeviceContext* pContext, const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir, int drawPrimitiveNum, int startPrimitiveNum)
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE ms;
	unsigned long primNum = 0;
	UINT stride = sizeof(DXP11_VERTEX);
	UINT offset = 0;
	float blendFactor[4] = { 0, 0, 0, 0 };

	if ((m_pVertexBuffer == NULL) || (m_pIndexBuffer == NULL)) return 0;
	if (s_pVS == NULL) return YN_SET_ERR("Program error.", 0, 0);

	if (m_LineTopology || m_PointTopology) {
		if (m_IndexNum == 0) return 0;
	}
	else {
		primNum = m_IndexNum / 3;
		if (drawPrimitiveNum >= 0) primNum = (unsigned long)drawPrimitiveNum;
		if (primNum == 0) return 0;
	}

	// update constants
	{
		XMMATRIX world = XMLoadFloat4x4(&m_World);
		XMMATRIX wvp = world * viewProj;
		DXP11_CONSTANTS c;
		XMStoreFloat4x4(&c.wvp, wvp);
		c.world = m_World;
		c.lightDir = lightDir;
		c.ambient = m_Ambient;
		c.options = XMFLOAT4((m_pSRV != NULL) ? 1.0f : 0.0f, m_LightEnable ? 1.0f : 0.0f, 0, 0);
		hr = pContext->Map(s_pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, 0);
		memcpy(ms.pData, &c, sizeof(c));
		pContext->Unmap(s_pConstBuf, 0);
	}

	pContext->IASetInputLayout(s_pLayout);
	pContext->IASetPrimitiveTopology(
		m_PointTopology ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST :
		m_LineTopology ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST :
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->VSSetShader(s_pVS, NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf);
	pContext->PSSetShader(s_pPS, NULL, 0);
	pContext->PSSetConstantBuffers(0, 1, &s_pConstBuf);
	pContext->PSSetSamplers(0, 1, &s_pSampler);
	pContext->PSSetShaderResources(0, 1, &m_pSRV);
	pContext->RSSetState(s_pRasterNoCull);
	pContext->OMSetBlendState(m_Additive ? s_pBlendAdd : s_pBlend, blendFactor, 0xFFFFFFFF);
	pContext->OMSetDepthStencilState(m_DepthWrite ? s_pDepth : s_pDepthNoWrite, 0);

	if (m_LineTopology || m_PointTopology) {
		pContext->DrawIndexed(m_IndexNum, 0, 0);
	}
	else {
		pContext->DrawIndexed(primNum * 3, (UINT)(startPrimitiveNum * 3), 0);
	}
	return 0;
}
