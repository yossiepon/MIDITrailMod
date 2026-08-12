//******************************************************************************
//
// MIDITrail / MTNoteAABBInstanced11
//
// Unified GPU-instanced note renderer for AABB-based scenes.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include <d3dcompiler.h>
#include "YNBaseLib.h"
#include "MTNoteAABBInstanced11.h"
#include "MTNoteInstancedShaderSnippets.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#pragma comment(lib, "d3dcompiler.lib")


//******************************************************************************
// Culling distance
//******************************************************************************
#define MTNOTEAABB_CULL_DISTANCE  (2200.0f)


//******************************************************************************
// Unified HLSL shader source (compiled with different #defines per mode)
//******************************************************************************
static const char* MTNOTEAABB_SHADER_SOURCE =
	"cbuffer Constants : register(b0) {\n"
	"  float4x4 g_WVP;\n"
	"  float4 g_PB[32];\n"
	"#if defined(HAS_ENVELOPE) || defined(HAS_LIGHTING_BILATERAL) || defined(HAS_LIGHTING_UNILATERAL)\n"
	"  float4x4 g_World;\n"
	"#endif\n"
	"#if defined(HAS_LIGHTING_BILATERAL) || defined(HAS_LIGHTING_UNILATERAL)\n"
	"  float4 g_Light;\n"
	"  float4 g_LAmb;\n"
	"#endif\n"
	"#ifdef HAS_ENVELOPE\n"
	"  float4 g_Active;\n"
	"  float4 g_Opts;\n"
	"  float4 g_Envelope;\n"
	"#endif\n"
	"};\n"

	"struct VSIN {\n"
	"  float3 corner : POSITION;\n"
	"  float3 nrm    : NORMAL;\n"
	"  float3 vmin   : TEXCOORD0;\n"
	"  float3 vmax   : TEXCOORD1;\n"
	"  float4 color  : COLOR0;\n"
	"  float  pbIdxF : TEXCOORD2;\n"
	"#ifdef HAS_ENVELOPE\n"
	"  float  alpha  : TEXCOORD3;\n"
	"  float  startMs: TEXCOORD4;\n"
	"  float  endMs  : TEXCOORD5;\n"
	"#endif\n"
	"};\n"

	"struct VSOUT {\n"
	"  float4 pos   : SV_POSITION;\n"
	"  float4 col   : COLOR0;\n"
	"#ifdef HAS_ENVELOPE\n"
	"  float  emph  : TEXCOORD0;\n"
	"  float  aflag : TEXCOORD1;\n"
	"#endif\n"
	"};\n"

	"#ifdef HAS_ENVELOPE\n"
	MTNOTEINST_HLSL_SHARED_FUNCTIONS
	"#endif\n"

	"VSOUT VSMain(VSIN i) {\n"
	"  VSOUT o;\n"

	// Envelope / active note detection
	"  float hide = 0.0;\n"
	"  float decayCoeff = 0.0;\n"
	"#ifdef HAS_ENVELOPE\n"
	"  float playMs = g_Active.x;\n"
	"  float apass = g_Active.w;\n"
	"  float active = ((i.startMs <= playMs) && (playMs <= i.endMs)) ? 1.0 : 0.0;\n"
	"  hide = abs(apass - active);\n"
	"  float keyDownRate = 0.0;\n"
	"  if (active > 0.5 && apass > 0.5) {\n"
	"    keyDownRate = CalcEnvelope(playMs, i.startMs, i.endMs,\n"
	"                              g_Envelope.x, g_Envelope.y, g_Envelope.z, g_Envelope.w);\n"
	"    decayCoeff = GetDecayCoeff(keyDownRate);\n"
	"  }\n"
	"#endif\n"

	// Position with AABB corner mask
	"#ifdef HAS_ENVELOPE\n"
	"  float growFactor = 1.0 + decayCoeff * g_Active.y;\n"
	"  float3 c   = (i.vmin + i.vmax) * 0.5;\n"
	"  float3 ext = (i.vmax - i.vmin) * 0.5;\n"
	"  float3 lo = float3(i.vmin.x, c.y - ext.y * growFactor, c.z - ext.z * growFactor);\n"
	"  float3 hi = float3(i.vmax.x, c.y + ext.y * growFactor, c.z + ext.z * growFactor);\n"
	"  float3 wp = lo + (hi - lo) * i.corner * (1.0 - hide);\n"
	"#else\n"
	"  float3 wp;\n"
	"  wp.x = lerp(i.vmin.x, i.vmax.x, i.corner.x);\n"
	"  wp.y = lerp(i.vmin.y, i.vmax.y, i.corner.y);\n"
	"  wp.z = i.vmin.z;\n"
	"#endif\n"

	// Pitch bend
	"  uint pbIdx = (uint)(i.pbIdxF + 0.5);\n"
	"  float pbShift = g_PB[pbIdx >> 2][pbIdx & 3];\n"
	"#ifdef HAS_ENVELOPE\n"
	"  wp.y += active * apass * pbShift;\n"
	"#else\n"
	"  wp.x += pbShift;\n"
	"#endif\n"

	"  o.pos = mul(float4(wp, 1.0), g_WVP);\n"

	// Color output
	"  float3 rgb = i.color.rgb;\n"

	// Lighting (bilateral: 2-light for Roll3D)
	"#ifdef HAS_LIGHTING_BILATERAL\n"
	"  float3 n = normalize(mul(float4(i.nrm, 0.0), g_World).xyz);\n"
	"  float3 L = normalize(g_Light.xyz);\n"
	"  float ndl = saturate(dot(n, -L)) + saturate(dot(n, L));\n"
	"  rgb = lerp(rgb, saturate(rgb * (g_LAmb.x + g_Light.w * ndl)), g_LAmb.w);\n"
	"#endif\n"

	// Lighting (unilateral: 1-light for Rain)
	"#ifdef HAS_LIGHTING_UNILATERAL\n"
	"  float3 n = normalize(mul(float4(i.nrm, 0.0), g_World).xyz);\n"
	"  float3 L = normalize(g_Light.xyz);\n"
	"  float ndl = saturate(dot(n, -L));\n"
	"  rgb = lerp(rgb, saturate(rgb * (g_LAmb.x + g_Light.w * ndl)), g_LAmb.w);\n"
	"#endif\n"

	// Alpha
	"#ifdef HAS_ALPHA_GRADIENT\n"
	"  float outAlpha = lerp(1.0, 0.5, i.corner.y);\n"
	"#elif defined(HAS_ENVELOPE)\n"
	"  float outAlpha = i.alpha;\n"
	"#else\n"
	"  float outAlpha = i.color.a;\n"
	"#endif\n"

	"  o.col = float4(rgb, outAlpha);\n"

	"#ifdef HAS_ENVELOPE\n"
	"  o.emph = decayCoeff * g_Active.z;\n"
	"  o.aflag = (apass > 0.5) ? active : 0.0;\n"
	"#endif\n"

	"  return o;\n"
	"}\n"

	"float4 PSMain(VSOUT i) : SV_TARGET {\n"
	"#ifdef HAS_ENVELOPE\n"
	"  float3 base = lerp(i.col.rgb, float3(1,1,1), saturate(i.emph));\n"
	"  base += i.aflag * g_Opts.yzw;\n"
	"  return float4(saturate(base), i.col.a);\n"
	"#else\n"
	"  return i.col;\n"
	"#endif\n"
	"}\n";


//******************************************************************************
// Static pipeline members (per-mode arrays)
//******************************************************************************
ID3D11VertexShader*  MTNoteAABBInstanced11::s_pVS[MODE_COUNT]       = {};
ID3D11PixelShader*   MTNoteAABBInstanced11::s_pPS[MODE_COUNT]       = {};
ID3D11InputLayout*   MTNoteAABBInstanced11::s_pLayout[MODE_COUNT]   = {};
ID3D11Buffer*        MTNoteAABBInstanced11::s_pConstBuf[MODE_COUNT] = {};


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteAABBInstanced11::MTNoteAABBInstanced11()
{
	m_Mode = MTAABBMode::Roll3D;
	m_pNoteDesign = nullptr;
	m_pNoteTracker = nullptr;
	m_pNotePitchBend = nullptr;
	m_pTemplateVB = nullptr;
	m_pInstanceVB = nullptr;
	m_pIndexBuffer = nullptr;
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	m_CurPos = 0.0f;
	m_isLightEnable = true;
	m_NoteCount = 0;
	m_IndexCountPerInstance = 36;
	m_TickToPos = 1.0f;
	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
}

MTNoteAABBInstanced11::~MTNoteAABBInstanced11()
{
	Release();
}


//******************************************************************************
// Compile one shader permutation
//******************************************************************************
static int CompilePermutation(
		ID3D11Device* pDevice,
		const D3D_SHADER_MACRO* pDefines,
		unsigned long cbufferSize,
		bool hasEnvelope,
		ID3D11VertexShader** ppVS,
		ID3D11PixelShader** ppPS,
		ID3D11InputLayout** ppLayout,
		ID3D11Buffer** ppConstBuf
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;
	ID3DBlob* pErrors = nullptr;

	hr = D3DCompile(MTNOTEAABB_SHADER_SOURCE, strlen(MTNOTEAABB_SHADER_SOURCE), "MTNoteAABBInstanced11",
	                pDefines, nullptr, "VSMain", "vs_5_0", 0, 0, &pVSBlob, &pErrors);
	if (FAILED(hr)) {
		result = YN_SET_ERR("Shader compile error (VS).", hr, 0);
		goto EXIT;
	}
	hr = D3DCompile(MTNOTEAABB_SHADER_SOURCE, strlen(MTNOTEAABB_SHADER_SOURCE), "MTNoteAABBInstanced11",
	                pDefines, nullptr, "PSMain", "ps_5_0", 0, 0, &pPSBlob, &pErrors);
	if (FAILED(hr)) {
		result = YN_SET_ERR("Shader compile error (PS).", hr, 0);
		goto EXIT;
	}

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, ppVS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, ppPS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	// Input Layout (slot 0 = template, slot 1 = instance)
	{
		D3D11_INPUT_ELEMENT_DESC layoutFull[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT,       1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32_FLOAT,       1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, DXGI_FORMAT_R32_FLOAT,       1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 5, DXGI_FORMAT_R32_FLOAT,       1, 40, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		D3D11_INPUT_ELEMENT_DESC layoutRain[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT,       1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};

		D3D11_INPUT_ELEMENT_DESC* pLayout = hasEnvelope ? layoutFull : layoutRain;
		UINT layoutCount = hasEnvelope ? _countof(layoutFull) : _countof(layoutRain);

		hr = pDevice->CreateInputLayout(pLayout, layoutCount,
		                                pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), ppLayout);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	// Constant buffer
	{
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = cbufferSize;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr = pDevice->CreateBuffer(&bd, nullptr, ppConstBuf);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

EXIT:;
	if (pVSBlob) pVSBlob->Release();
	if (pPSBlob) pPSBlob->Release();
	if (pErrors) pErrors->Release();
	return result;
}


//******************************************************************************
// Initialize static pipeline (all 3 permutations)
//******************************************************************************
int MTNoteAABBInstanced11::InitPipeline(ID3D11Device* pDevice)
{
	int result = 0;

	if (s_pVS[0] != nullptr) return 0;

	// Roll3D: HAS_LIGHTING_BILATERAL + HAS_ENVELOPE
	{
		D3D_SHADER_MACRO defines[] = {
			{ "HAS_LIGHTING_BILATERAL", "1" },
			{ "HAS_ENVELOPE", "1" },
			{ nullptr, nullptr }
		};
		result = CompilePermutation(pDevice, defines, sizeof(CBufferRoll3D), true,
		                            &s_pVS[0], &s_pPS[0], &s_pLayout[0], &s_pConstBuf[0]);
		if (result != 0) goto EXIT;
	}

	// Roll2D: HAS_ENVELOPE only
	{
		D3D_SHADER_MACRO defines[] = {
			{ "HAS_ENVELOPE", "1" },
			{ nullptr, nullptr }
		};
		result = CompilePermutation(pDevice, defines, sizeof(CBufferRoll2D), true,
		                            &s_pVS[1], &s_pPS[1], &s_pLayout[1], &s_pConstBuf[1]);
		if (result != 0) goto EXIT;
	}

	// Rain: HAS_LIGHTING_UNILATERAL + HAS_ALPHA_GRADIENT
	{
		D3D_SHADER_MACRO defines[] = {
			{ "HAS_LIGHTING_UNILATERAL", "1" },
			{ "HAS_ALPHA_GRADIENT", "1" },
			{ nullptr, nullptr }
		};
		result = CompilePermutation(pDevice, defines, sizeof(CBufferRain), false,
		                            &s_pVS[2], &s_pPS[2], &s_pLayout[2], &s_pConstBuf[2]);
		if (result != 0) goto EXIT;
	}

	result = InitCommonStates(pDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// Release static pipeline
//******************************************************************************
void MTNoteAABBInstanced11::ReleasePipeline()
{
	for (int i = 0; i < MODE_COUNT; i++) {
		if (s_pVS[i])       { s_pVS[i]->Release();       s_pVS[i] = nullptr; }
		if (s_pPS[i])       { s_pPS[i]->Release();       s_pPS[i] = nullptr; }
		if (s_pLayout[i])   { s_pLayout[i]->Release();   s_pLayout[i] = nullptr; }
		if (s_pConstBuf[i]) { s_pConstBuf[i]->Release(); s_pConstBuf[i] = nullptr; }
	}
}


//******************************************************************************
// Create
//******************************************************************************
int MTNoteAABBInstanced11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNoteTracker* pNoteTracker,
		MTNotePitchBend* pNotePitchBend,
		MTAABBMode mode,
		MTNoteDesign11* pNoteDesign
	)
{
	int result = 0;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_Mode = mode;
	m_pNoteTracker = pNoteTracker;
	m_pNotePitchBend = pNotePitchBend;

	if (pNoteDesign != NULL) {
		m_pNoteDesign = pNoteDesign;
	}
	else {
		result = m_NoteDesignLocal.Initialize(pSceneName, pSeqData);
		if (result != 0) goto EXIT;
		m_pNoteDesign = &m_NoteDesignLocal;
	}

	if (mode == MTAABBMode::Rain) {
		result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
		if (result != 0) goto EXIT;
	}

	m_NoteCount = (m_pNoteTracker != nullptr) ? m_pNoteTracker->GetNoteCount() : 0;
	m_TickToPos = m_pNoteDesign->GetPlayPosX(1);

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
// Create template geometry
//******************************************************************************
int MTNoteAABBInstanced11::_CreateTemplateGeometry(ID3D11Device* pDevice)
{
	int result = 0;

	if (m_Mode == MTAABBMode::Roll3D) {
		// 3D: full box (24 vertices, 36 indices)
		MTNOTEAABB_INST_TEMPLATE_VERTEX verts[24] = {
			{{0,1,1}, {0, 1, 0}}, {{1,1,1}, {0, 1, 0}}, {{0,1,0}, {0, 1, 0}}, {{1,1,0}, {0, 1, 0}},
			{{0,0,0}, {0,-1, 0}}, {{1,0,0}, {0,-1, 0}}, {{0,0,1}, {0,-1, 0}}, {{1,0,1}, {0,-1, 0}},
			{{0,1,0}, {0, 0,-1}}, {{1,1,0}, {0, 0,-1}}, {{0,0,0}, {0, 0,-1}}, {{1,0,0}, {0, 0,-1}},
			{{0,0,1}, {0, 0, 1}}, {{1,0,1}, {0, 0, 1}}, {{0,1,1}, {0, 0, 1}}, {{1,1,1}, {0, 0, 1}},
			{{0,1,1}, {-1, 0, 0}}, {{0,1,0}, {-1, 0, 0}}, {{0,0,1}, {-1, 0, 0}}, {{0,0,0}, {-1, 0, 0}},
			{{1,1,0}, { 1, 0, 0}}, {{1,1,1}, { 1, 0, 0}}, {{1,0,0}, { 1, 0, 0}}, {{1,0,1}, { 1, 0, 0}},
		};
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER, verts, sizeof(verts), &m_pTemplateVB);
		if (result != 0) goto EXIT;

		unsigned long indices[36] = {
			 0, 1, 2,  2, 1, 3,   4, 5, 6,  6, 5, 7,   8, 9,10, 10, 9,11,
			12,13,14, 14,13,15,  16,17,18, 18,17,19,  20,21,22, 22,21,23,
		};
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_INDEX_BUFFER, indices, sizeof(indices), &m_pIndexBuffer);
		if (result != 0) goto EXIT;

		m_IndexCountPerInstance = 36;
	}
	else if (m_Mode == MTAABBMode::Roll2D) {
		// 2D: Z-facing face (4 vertices, 6 indices)
		MTNOTEAABB_INST_TEMPLATE_VERTEX verts[4] = {
			{{0,1,0}, {0, 0, -1}},
			{{1,1,0}, {0, 0, -1}},
			{{0,0,0}, {0, 0, -1}},
			{{1,0,0}, {0, 0, -1}},
		};
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER, verts, sizeof(verts), &m_pTemplateVB);
		if (result != 0) goto EXIT;

		unsigned long indices[6] = { 0, 1, 2, 2, 1, 3 };
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_INDEX_BUFFER, indices, sizeof(indices), &m_pIndexBuffer);
		if (result != 0) goto EXIT;

		m_IndexCountPerInstance = 6;
	}
	else {
		// Rain: 4-vertex quad
		MTNOTEAABB_INST_TEMPLATE_VERTEX verts[4] = {
			{{0, 0, 0}, {0, 1, 0}},
			{{1, 0, 0}, {0, 1, 0}},
			{{1, 1, 0}, {0, 1, 0}},
			{{0, 1, 0}, {0, 1, 0}},
		};
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER, verts, sizeof(verts), &m_pTemplateVB);
		if (result != 0) goto EXIT;

		unsigned long indices[6] = { 0, 2, 1, 0, 3, 2 };
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_INDEX_BUFFER, indices, sizeof(indices), &m_pIndexBuffer);
		if (result != 0) goto EXIT;

		m_IndexCountPerInstance = 6;
	}

EXIT:;
	return result;
}


//******************************************************************************
// Create instance buffer
//******************************************************************************
int MTNoteAABBInstanced11::_CreateInstanceBuffer(ID3D11Device* pDevice, SMSeqData* pSeqData)
{
	int result = 0;

	if (m_Mode != MTAABBMode::Rain && m_NoteCount == 0) goto EXIT;

	if (m_Mode == MTAABBMode::Rain) {
		// Rain: 32B instances from merged note list
		SMTrack track;
		SMNoteList noteList;

		result = pSeqData->GetMergedTrack(&track);
		if (result != 0) goto EXIT;
		result = track.GetNoteList(&noteList);
		if (result != 0) goto EXIT;

		m_NoteCount = noteList.GetSize();
		if (m_NoteCount == 0) goto EXIT;

		std::vector<MTNOTEAABB_INST_INSTANCE_RAIN> instances(m_NoteCount);
		std::vector<unsigned long> startTicks(m_NoteCount);
		std::vector<unsigned long> endTicks(m_NoteCount);

		for (unsigned long i = 0; i < m_NoteCount; i++) {
			SMNote note;
			result = noteList.GetNote(i, &note);
			if (result != 0) goto EXIT;

			float startY = m_pNoteDesign->GetPlayPosX(note.startTime);
			float endY   = m_pNoteDesign->GetPlayPosX(note.endTime);

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

			Color c = m_pNoteDesign->GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
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
		                               (unsigned long)(m_NoteCount * sizeof(MTNOTEAABB_INST_INSTANCE_RAIN)),
		                               &m_pInstanceVB);
		if (result != 0) goto EXIT;

		BuildCullingArrays(startTicks.data(), endTicks.data(), m_NoteCount);
	}
	else {
		// Roll 3D/2D: 44B instances from NoteTracker
		std::vector<MTNOTEAABB_INST_INSTANCE_FULL> instances(m_NoteCount);
		std::vector<unsigned long> startTicks(m_NoteCount);
		std::vector<unsigned long> endTicks(m_NoteCount);

		for (unsigned long i = 0; i < m_NoteCount; i++) {
			const NoteData& note = m_pNoteTracker->GetNote(i);

			Vector3 startCorners[4], endCorners[4];
			m_pNoteDesign->GetNoteBoxVirtexPos(
				note.startTimeTick, note.portNo, note.chNo, note.noteNo,
				&startCorners[0], &startCorners[1], &startCorners[2], &startCorners[3]);
			m_pNoteDesign->GetNoteBoxVirtexPos(
				note.endTimeTick, note.portNo, note.chNo, note.noteNo,
				&endCorners[0], &endCorners[1], &endCorners[2], &endCorners[3]);

			instances[i].vmin[0] = startCorners[3].x;
			instances[i].vmin[1] = startCorners[3].y;
			instances[i].vmin[2] = startCorners[1].z;
			instances[i].vmax[0] = endCorners[0].x;
			instances[i].vmax[1] = startCorners[0].y;
			instances[i].vmax[2] = startCorners[0].z;

			Color c = m_pNoteDesign->GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
			instances[i].color =
				((unsigned long)(c.A() * 255.0f + 0.5f) << 24) |
				((unsigned long)(c.R() * 255.0f + 0.5f) << 16) |
				((unsigned long)(c.G() * 255.0f + 0.5f) <<  8) |
				((unsigned long)(c.B() * 255.0f + 0.5f));

			instances[i].pbIndex = (float)((note.portNo & 0x7) * 16 + note.chNo);
			instances[i].alpha = c.A();
			instances[i].startTimeMs = (float)note.startTimeMs;
			instances[i].endTimeMs = (float)note.endTimeMs;

			startTicks[i] = note.startTimeTick;
			endTicks[i] = note.endTimeTick;
		}

		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER,
		                               instances.data(),
		                               (unsigned long)(m_NoteCount * sizeof(MTNOTEAABB_INST_INSTANCE_FULL)),
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
int MTNoteAABBInstanced11::Update(const MTSceneUpdateContext& ctx)
{
	m_CurTickTime = ctx.curTickTime;
	m_PlayTimeMSec = ctx.playTimeMSec;
	m_CurPos = m_pNoteDesign->GetPlayPosX(m_CurTickTime);

	if (m_Mode == MTAABBMode::Rain) {
		Matrix world = Matrix::CreateTranslation(0.0f, -m_CurPos, 0.0f)
		             * Matrix::CreateRotationY(XMConvertToRadians(ctx.rollAngle));
		XMStoreFloat4x4(&m_World, world);
	}
	else {
		Vector3 moveVector = m_pNoteDesign->GetWorldMoveVector();
		Matrix rotateMatrix = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle));
		Matrix moveMatrix = Matrix::CreateTranslation(moveVector);
		Matrix world = rotateMatrix * moveMatrix;
		XMStoreFloat4x4(&m_World, world);
	}

	return 0;
}


//******************************************************************************
// Draw
//******************************************************************************
int MTNoteAABBInstanced11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	if (m_Mode == MTAABBMode::Rain) {
		return _DrawRain(pContext, viewProj, lightDir);
	}
	else {
		return _DrawRoll(pContext, viewProj, lightDir);
	}
}


//******************************************************************************
// Draw: Roll 3D/2D (2-pass with envelope)
//******************************************************************************
int MTNoteAABBInstanced11::_DrawRoll(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;
	int modeIdx = (m_Mode == MTAABBMode::Roll3D) ? 0 : 1;

	if (!IsEnable() || m_NoteCount == 0) goto EXIT;

	{
		unsigned long tickWindow = (unsigned long)(MTNOTEAABB_CULL_DISTANCE / m_TickToPos);
		unsigned long tickLow = (m_CurTickTime > tickWindow) ? (m_CurTickTime - tickWindow) : 0;
		unsigned long tickHigh = ((0xFFFFFFFF - m_CurTickTime) < tickWindow)
		                         ? 0xFFFFFFFF : (m_CurTickTime + tickWindow);

		unsigned long loNote = 0, hiNote = 0;
		GetVisibleRange(tickLow, tickHigh, &loNote, &hiNote);
		if (loNote >= hiNote) goto EXIT;

		unsigned long loActive = 0, hiActive = 0;
		GetVisibleRange(m_CurTickTime, m_CurTickTime, &loActive, &hiActive);

		MTEnvelopeConfig envConfig = m_pNoteDesign->GetEnvelopeConfig();

		pContext->IASetInputLayout(s_pLayout[modeIdx]);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->VSSetShader(s_pVS[modeIdx], nullptr, 0);
		pContext->PSSetShader(s_pPS[modeIdx], nullptr, 0);
		BindCommonStates(pContext);

		UINT strides[2] = { sizeof(MTNOTEAABB_INST_TEMPLATE_VERTEX), sizeof(MTNOTEAABB_INST_INSTANCE_FULL) };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* buffers[2] = { m_pTemplateVB, m_pInstanceVB };
		pContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		Matrix world = XMLoadFloat4x4(&m_World);
		Matrix wvp = world * viewProj;

		Color emissive = m_pNoteDesign->GetActiveNoteEmissive();
		float growFactor = m_pNoteDesign->GetActiveNoteBoxSizeRatio() - 1.0f;
		float whiteRate = m_pNoteDesign->GetActiveNoteWhiteRate();

		for (int pass = 0; pass < 2; pass++) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = pContext->Map(s_pConstBuf[modeIdx], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

			if (m_Mode == MTAABBMode::Roll3D) {
				CBufferRoll3D* cb = (CBufferRoll3D*)mapped.pData;
				XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(wvp));
				XMStoreFloat4x4(&cb->world, XMMatrixTranspose(world));
				cb->light = XMFLOAT4(lightDir.x, lightDir.y, lightDir.z, 1.2f);
				cb->lambient = XMFLOAT4(0.2f, 0.0f, 0.0f, m_isLightEnable ? 1.0f : 0.0f);
				cb->active = XMFLOAT4((float)m_PlayTimeMSec, growFactor, whiteRate, (float)pass);
				cb->opts = XMFLOAT4(0.0f, emissive.R(), emissive.G(), emissive.B());
				cb->envelope = XMFLOAT4(envConfig.decayDurationMs, envConfig.releaseDurationMs,
				                       envConfig.decayRatio, envConfig.sustainRatio);
				FillPitchBendArray((float*)cb->pb, m_pNotePitchBend,
					[this](short v, unsigned char s) { return m_pNoteDesign->GetPitchBendShift(v, s); });
			}
			else {
				CBufferRoll2D* cb = (CBufferRoll2D*)mapped.pData;
				XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(wvp));
				XMStoreFloat4x4(&cb->world, XMMatrixTranspose(world));
				cb->active = XMFLOAT4((float)m_PlayTimeMSec, growFactor, whiteRate, (float)pass);
				cb->opts = XMFLOAT4(0.0f, emissive.R(), emissive.G(), emissive.B());
				cb->envelope = XMFLOAT4(envConfig.decayDurationMs, envConfig.releaseDurationMs,
				                       envConfig.decayRatio, envConfig.sustainRatio);
				FillPitchBendArray((float*)cb->pb, m_pNotePitchBend,
					[this](short v, unsigned char s) { return m_pNoteDesign->GetPitchBendShift(v, s); });
			}

			pContext->Unmap(s_pConstBuf[modeIdx], 0);
			pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf[modeIdx]);
			pContext->PSSetConstantBuffers(0, 1, &s_pConstBuf[modeIdx]);

			if (pass == 0) {
				pContext->DrawIndexedInstanced(m_IndexCountPerInstance, hiNote - loNote, 0, 0, loNote);
			}
			else {
				if (loActive < hiActive) {
					pContext->DrawIndexedInstanced(m_IndexCountPerInstance, hiActive - loActive, 0, 0, loActive);
				}
			}
		}
	}

EXIT:;
	return result;
}


//******************************************************************************
// Draw: Rain (1-pass, no envelope)
//******************************************************************************
int MTNoteAABBInstanced11::_DrawRain(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;
	const int modeIdx = 2;

	if (!IsEnable() || m_NoteCount == 0) goto EXIT;

	{
		unsigned long tickWindow = (unsigned long)(MTNOTEAABB_CULL_DISTANCE / m_TickToPos);
		unsigned long tickLow = (m_CurTickTime > tickWindow) ? (m_CurTickTime - tickWindow) : 0;
		unsigned long tickHigh = ((0xFFFFFFFF - m_CurTickTime) < tickWindow)
		                         ? 0xFFFFFFFF : (m_CurTickTime + tickWindow);

		unsigned long loNote = 0, hiNote = 0;
		GetVisibleRange(tickLow, tickHigh, &loNote, &hiNote);
		if (loNote >= hiNote) goto EXIT;

		pContext->IASetInputLayout(s_pLayout[modeIdx]);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->VSSetShader(s_pVS[modeIdx], nullptr, 0);
		pContext->PSSetShader(s_pPS[modeIdx], nullptr, 0);
		BindCommonStates(pContext);

		UINT strides[2] = { sizeof(MTNOTEAABB_INST_TEMPLATE_VERTEX), sizeof(MTNOTEAABB_INST_INSTANCE_RAIN) };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* buffers[2] = { m_pTemplateVB, m_pInstanceVB };
		pContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		{
			Matrix world = XMLoadFloat4x4(&m_World);
			Matrix wvp = world * viewProj;

			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = pContext->Map(s_pConstBuf[modeIdx], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

			CBufferRain* cb = (CBufferRain*)mapped.pData;
			XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(wvp));
			XMStoreFloat4x4(&cb->world, XMMatrixTranspose(world));
			cb->light = XMFLOAT4(lightDir.x, lightDir.y, lightDir.z, 1.0f);
			cb->lambient = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);

			FillPitchBendArray((float*)cb->pb, m_pNotePitchBend,
				[this](short v, unsigned char s) { return m_KeyboardDesign.GetPitchBendShift(v, s); });

			pContext->Unmap(s_pConstBuf[modeIdx], 0);
			pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf[modeIdx]);
		}

		pContext->DrawIndexedInstanced(m_IndexCountPerInstance, hiNote - loNote, 0, 0, loNote);
	}

EXIT:;
	return result;
}


//******************************************************************************
// Release
//******************************************************************************
void MTNoteAABBInstanced11::Release()
{
	if (m_pTemplateVB)  { m_pTemplateVB->Release();  m_pTemplateVB = nullptr; }
	if (m_pInstanceVB)  { m_pInstanceVB->Release();  m_pInstanceVB = nullptr; }
	if (m_pIndexBuffer) { m_pIndexBuffer->Release(); m_pIndexBuffer = nullptr; }

	m_pNoteDesign = nullptr;
	m_pNoteTracker = nullptr;
	m_pNotePitchBend = nullptr;
	m_NoteCount = 0;
}


//******************************************************************************
// Reset
//******************************************************************************
void MTNoteAABBInstanced11::Reset()
{
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	m_CurPos = 0.0f;
}


//******************************************************************************
// Note count
//******************************************************************************
unsigned long MTNoteAABBInstanced11::GetNoteCount() const
{
	return m_NoteCount;
}
