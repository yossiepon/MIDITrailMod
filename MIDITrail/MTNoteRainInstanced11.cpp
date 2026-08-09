//******************************************************************************
//
// MIDITrail / MTNoteRainInstanced11
//
// GPU-instanced note rain renderer for PianoRoll Rain scenes.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include <d3dcompiler.h>
#include "YNBaseLib.h"
#include "MTNoteRainInstanced11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#pragma comment(lib, "d3dcompiler.lib")


//******************************************************************************
// Culling distance
//******************************************************************************
#define MTNOTERAININST_CULL_DISTANCE  (2200.0f)


//******************************************************************************
// Inline HLSL shader
//******************************************************************************
static const char* MTNOTERAININST_SHADER =
	"cbuffer Constants : register(b0) {\n"
	"  float4x4 g_WVP;\n"
	"  float4 g_PB[32];\n"
	"};\n"

	"struct VSIN {\n"
	"  float3 corner : POSITION;\n"
	"  float3 vmin   : TEXCOORD0;\n"
	"  float3 vmax   : TEXCOORD1;\n"
	"  float4 color  : COLOR0;\n"
	"  float  pbIdxF : TEXCOORD2;\n"
	"};\n"

	"struct VSOUT {\n"
	"  float4 pos : SV_POSITION;\n"
	"  float4 col : COLOR0;\n"
	"};\n"

	"VSOUT VSMain(VSIN i) {\n"
	"  VSOUT o;\n"
	"  float3 wp;\n"
	"  wp.x = lerp(i.vmin.x, i.vmax.x, i.corner.x);\n"
	"  wp.y = lerp(i.vmin.y, i.vmax.y, i.corner.y);\n"
	"  wp.z = i.vmin.z;\n"

	// Pitch bend X shift
	"  uint pbIdx = (uint)(i.pbIdxF + 0.5);\n"
	"  float pbShift = g_PB[pbIdx >> 2][pbIdx & 3];\n"
	"  wp.x += pbShift;\n"

	"  o.pos = mul(float4(wp, 1.0), g_WVP);\n"

	// Alpha gradient: note-ON end (corner.y=0) = 1.0, note-OFF end (corner.y=1) = 0.5
	"  float alpha = lerp(1.0, 0.5, i.corner.y);\n"
	"  o.col = float4(i.color.rgb, alpha);\n"
	"  return o;\n"
	"}\n"

	"float4 PSMain(VSOUT i) : SV_TARGET {\n"
	"  return i.col;\n"
	"}\n";


//******************************************************************************
// Static pipeline members
//******************************************************************************
ID3D11VertexShader*      MTNoteRainInstanced11::s_pVS         = nullptr;
ID3D11PixelShader*       MTNoteRainInstanced11::s_pPS         = nullptr;
ID3D11InputLayout*       MTNoteRainInstanced11::s_pLayout     = nullptr;
ID3D11Buffer*            MTNoteRainInstanced11::s_pConstBuf   = nullptr;
ID3D11RasterizerState*   MTNoteRainInstanced11::s_pRasterNoCull  = nullptr;
ID3D11BlendState*        MTNoteRainInstanced11::s_pBlend      = nullptr;
ID3D11DepthStencilState* MTNoteRainInstanced11::s_pDepth      = nullptr;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteRainInstanced11::MTNoteRainInstanced11()
{
	m_pNotePitchBend = nullptr;
	m_pTemplateVB = nullptr;
	m_pInstanceVB = nullptr;
	m_pIndexBuffer = nullptr;
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
	m_NoteCount = 0;
	m_YPerTick = 1.0f;
	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
}

MTNoteRainInstanced11::~MTNoteRainInstanced11()
{
	Release();
}


//******************************************************************************
// Initialize static pipeline
//******************************************************************************
int MTNoteRainInstanced11::InitPipeline(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;
	ID3DBlob* pErrors = nullptr;

	if (s_pVS != nullptr) return 0;

	hr = D3DCompile(MTNOTERAININST_SHADER, strlen(MTNOTERAININST_SHADER), "MTNoteRainInstanced11",
	                nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &pVSBlob, &pErrors);
	if (FAILED(hr)) {
		result = YN_SET_ERR("Shader compile error (VS).", hr, 0);
		goto EXIT;
	}
	hr = D3DCompile(MTNOTERAININST_SHADER, strlen(MTNOTERAININST_SHADER), "MTNoteRainInstanced11",
	                nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pPSBlob, &pErrors);
	if (FAILED(hr)) {
		result = YN_SET_ERR("Shader compile error (PS).", hr, 0);
		goto EXIT;
	}

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &s_pVS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &s_pPS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	// Input Layout
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT,       1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		hr = pDevice->CreateInputLayout(layout, _countof(layout),
		                                pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &s_pLayout);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	// Constant buffer
	{
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(CBuffer);
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr = pDevice->CreateBuffer(&bd, nullptr, &s_pConstBuf);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	// Rasterizer (no cull for flat quads)
	{
		D3D11_RASTERIZER_DESC rd = {};
		rd.FillMode = D3D11_FILL_SOLID;
		rd.CullMode = D3D11_CULL_NONE;
		rd.DepthClipEnable = TRUE;
		hr = pDevice->CreateRasterizerState(&rd, &s_pRasterNoCull);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	// Blend (alpha blending for gradient)
	{
		D3D11_BLEND_DESC bd = {};
		bd.RenderTarget[0].BlendEnable = TRUE;
		bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&bd, &s_pBlend);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	// Depth stencil
	{
		D3D11_DEPTH_STENCIL_DESC dd = {};
		dd.DepthEnable = TRUE;
		dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		hr = pDevice->CreateDepthStencilState(&dd, &s_pDepth);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

EXIT:;
	if (pVSBlob) pVSBlob->Release();
	if (pPSBlob) pPSBlob->Release();
	if (pErrors) pErrors->Release();
	return result;
}


//******************************************************************************
// Release static pipeline
//******************************************************************************
void MTNoteRainInstanced11::ReleasePipeline()
{
	if (s_pVS)          { s_pVS->Release();          s_pVS = nullptr; }
	if (s_pPS)          { s_pPS->Release();          s_pPS = nullptr; }
	if (s_pLayout)      { s_pLayout->Release();      s_pLayout = nullptr; }
	if (s_pConstBuf)    { s_pConstBuf->Release();    s_pConstBuf = nullptr; }
	if (s_pRasterNoCull){ s_pRasterNoCull->Release(); s_pRasterNoCull = nullptr; }
	if (s_pBlend)       { s_pBlend->Release();       s_pBlend = nullptr; }
	if (s_pDepth)       { s_pDepth->Release();       s_pDepth = nullptr; }
}


//******************************************************************************
// Create
//******************************************************************************
int MTNoteRainInstanced11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_pNotePitchBend = pNotePitchBend;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	m_YPerTick = m_NoteDesign.GetPlayPosX(1);

	result = InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = _CreateTemplateGeometry(pDevice);
	if (result != 0) goto EXIT;

	result = _CreateInstanceBuffer(pDevice, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// Create template geometry (4-vertex quad)
//******************************************************************************
int MTNoteRainInstanced11::_CreateTemplateGeometry(ID3D11Device* pDevice)
{
	int result = 0;

	// corner.x selects vmin.x(0) or vmax.x(1), corner.y selects vmin.y(0) or vmax.y(1)
	MTNOTERAIN_INST_TEMPLATE_VERTEX verts[4] = {
		{{0, 0, 0}},   // bottom-left  (note-ON end, alpha=1.0)
		{{1, 0, 0}},   // bottom-right (note-ON end, alpha=1.0)
		{{1, 1, 0}},   // top-right    (note-OFF end, alpha=0.5)
		{{0, 1, 0}},   // top-left     (note-OFF end, alpha=0.5)
	};

	result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER,
	                               verts, sizeof(verts), &m_pTemplateVB);
	if (result != 0) goto EXIT;

	unsigned long indices[6] = {
		0, 2, 1,
		0, 3, 2,
	};

	result = CreateImmutableBuffer(pDevice, D3D11_BIND_INDEX_BUFFER,
	                               indices, sizeof(indices), &m_pIndexBuffer);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// Create instance buffer
//******************************************************************************
int MTNoteRainInstanced11::_CreateInstanceBuffer(ID3D11Device* pDevice, SMSeqData* pSeqData)
{
	int result = 0;
	SMTrack track;
	SMNoteList noteList;

	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	result = track.GetNoteList(&noteList);
	if (result != 0) goto EXIT;

	m_NoteCount = noteList.GetSize();
	if (m_NoteCount == 0) goto EXIT;

	{
		std::vector<MTNOTERAIN_INST_INSTANCE> instances(m_NoteCount);
		std::vector<unsigned long> startTicks(m_NoteCount);
		std::vector<unsigned long> endTicks(m_NoteCount);

		for (unsigned long i = 0; i < m_NoteCount; i++) {
			SMNote note;
			result = noteList.GetNote(i, &note);
			if (result != 0) goto EXIT;

			float startY = m_NoteDesign.GetPlayPosX(note.startTime);
			float endY   = m_NoteDesign.GetPlayPosX(note.endTime);

			Vector3 moveVec = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
			float posX = moveVec.x + m_KeyboardDesign.GetKeyCenterPosX(note.noteNo);
			float posY = moveVec.y + m_KeyboardDesign.GetWhiteKeyHeight() / 2.0f;
			float posZ = moveVec.z + m_KeyboardDesign.GetNoteDropPosZ(note.noteNo);
			float rainWidth = m_KeyboardDesign.GetBlackKeyWidth();

			instances[i].vmin[0] = posX - rainWidth / 2.0f;
			instances[i].vmin[1] = startY + posY;
			instances[i].vmin[2] = posZ;
			instances[i].vmax[0] = posX + rainWidth / 2.0f;
			instances[i].vmax[1] = endY + posY;
			instances[i].vmax[2] = posZ;

			Color c = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
			instances[i].color =
				((unsigned long)(c.A() * 255.0f + 0.5f) << 24) |
				((unsigned long)(c.R() * 255.0f + 0.5f) << 16) |
				((unsigned long)(c.G() * 255.0f + 0.5f) <<  8) |
				((unsigned long)(c.B() * 255.0f + 0.5f));

			instances[i].pbIndex = (float)((note.portNo & 0x7) * 16 + note.chNo);

			startTicks[i] = note.startTime;
			endTicks[i] = note.endTime;
		}

		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER,
		                               instances.data(),
		                               (unsigned long)(m_NoteCount * sizeof(MTNOTERAIN_INST_INSTANCE)),
		                               &m_pInstanceVB);
		if (result != 0) goto EXIT;

		BuildCullingArrays(startTicks.data(), endTicks.data(), m_NoteCount);
	}

EXIT:;
	return result;
}


//******************************************************************************
// Update
//******************************************************************************
int MTNoteRainInstanced11::Update(const MTSceneUpdateContext& ctx)
{
	m_CurTickTime = ctx.curTickTime;
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	Matrix world = Matrix::CreateTranslation(0.0f, -m_CurPos, 0.0f)
	             * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
	XMStoreFloat4x4(&m_World, world);

	return 0;
}


//******************************************************************************
// Draw
//******************************************************************************
int MTNoteRainInstanced11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& /*lightDir*/
	)
{
	int result = 0;

	if (!IsEnable() || m_NoteCount == 0) goto EXIT;

	{
		unsigned long tickWindow = (unsigned long)(MTNOTERAININST_CULL_DISTANCE / m_YPerTick);
		unsigned long tickLow = (m_CurTickTime > tickWindow) ? (m_CurTickTime - tickWindow) : 0;
		unsigned long tickHigh = m_CurTickTime + tickWindow;

		unsigned long loNote = 0, hiNote = 0;
		GetVisibleRange(tickLow, tickHigh, &loNote, &hiNote);
		if (loNote >= hiNote) goto EXIT;

		pContext->IASetInputLayout(s_pLayout);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->VSSetShader(s_pVS, nullptr, 0);
		pContext->PSSetShader(s_pPS, nullptr, 0);
		pContext->RSSetState(s_pRasterNoCull);
		pContext->OMSetBlendState(s_pBlend, nullptr, 0xFFFFFFFF);
		pContext->OMSetDepthStencilState(s_pDepth, 0);

		UINT strides[2] = { sizeof(MTNOTERAIN_INST_TEMPLATE_VERTEX), sizeof(MTNOTERAIN_INST_INSTANCE) };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* buffers[2] = { m_pTemplateVB, m_pInstanceVB };
		pContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Update constant buffer
		{
			Matrix world = XMLoadFloat4x4(&m_World);
			Matrix wvp = world * viewProj;

			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = pContext->Map(s_pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

			CBuffer* cb = (CBuffer*)mapped.pData;
			XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(wvp));

			ZeroMemory(cb->pb, sizeof(cb->pb));
			if (m_pNotePitchBend != nullptr) {
				for (unsigned char port = 0; port < 8; port++) {
					for (unsigned char ch = 0; ch < 16; ch++) {
						short pbValue = m_pNotePitchBend->GetValue(port, ch);
						unsigned char pbSens = m_pNotePitchBend->GetSensitivity(port, ch);
						unsigned int idx = port * 16 + ch;
						((float*)cb->pb)[idx] = m_KeyboardDesign.GetPitchBendShift(pbValue, pbSens);
					}
				}
			}

			pContext->Unmap(s_pConstBuf, 0);
			pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf);
		}

		pContext->DrawIndexedInstanced(6, hiNote - loNote, 0, 0, loNote);
	}

EXIT:;
	return result;
}


//******************************************************************************
// Release
//******************************************************************************
void MTNoteRainInstanced11::Release()
{
	if (m_pTemplateVB)  { m_pTemplateVB->Release();  m_pTemplateVB = nullptr; }
	if (m_pInstanceVB)  { m_pInstanceVB->Release();  m_pInstanceVB = nullptr; }
	if (m_pIndexBuffer) { m_pIndexBuffer->Release(); m_pIndexBuffer = nullptr; }

	m_pNotePitchBend = nullptr;
	m_NoteCount = 0;
}


//******************************************************************************
// Reset
//******************************************************************************
void MTNoteRainInstanced11::Reset()
{
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
}


//******************************************************************************
// Note count
//******************************************************************************
unsigned long MTNoteRainInstanced11::GetNoteCount() const
{
	return m_NoteCount;
}
