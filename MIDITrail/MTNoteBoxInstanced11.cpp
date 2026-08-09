//******************************************************************************
//
// MIDITrail / MTNoteBoxInstanced11
//
// GPU-instanced note box renderer for PianoRoll 3D/2D scenes.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include <d3dcompiler.h>
#include "YNBaseLib.h"
#include "MTNoteBoxInstanced11.h"


using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#pragma comment(lib, "d3dcompiler.lib")


//******************************************************************************
// Culling distance (world-space units from camera to note visibility boundary)
//******************************************************************************
#define MTNOTEBOXINST_CULL_DISTANCE  (2200.0f)


//******************************************************************************
// Inline HLSL shader
//******************************************************************************
static const char* MTNOTEBOXINST_SHADER =
	"cbuffer Constants : register(b0) {\n"
	"  float4x4 g_WVP;\n"
	"  float4x4 g_World;\n"
	"  float4 g_Active;\n"    // x=playTimeMSec, y=growFactor(sizeRatio-1), z=whiteRate, w=pass(0/1)
	"  float4 g_Opts;\n"      // x=unused, yzw=emissiveRGB
	"  float4 g_Light;\n"     // xyz=lightDir, w=diffuseLevel
	"  float4 g_LAmb;\n"      // x=ambientLevel, y=unused, z=unused, w=lightEnable(0/1)
	"  float4 g_Envelope;\n"  // x=decayDurMs, y=releaseDurMs, z=decayRatio, w=sustainRatio
	"  float4 g_PB[32];\n"    // pitch-bend Y shift (128 values packed in 32 float4s)
	"};\n"

	"struct VSIN {\n"
	"  float3 corner : POSITION;\n"
	"  float3 nrm    : NORMAL;\n"
	"  float3 vmin   : TEXCOORD0;\n"
	"  float3 vmax   : TEXCOORD1;\n"
	"  float4 color  : COLOR0;\n"
	"  float  pbIdxF : TEXCOORD2;\n"
	"  float  alpha  : TEXCOORD3;\n"
	"  float  startMs: TEXCOORD4;\n"
	"  float  endMs  : TEXCOORD5;\n"
	"};\n"

	"struct VSOUT {\n"
	"  float4 pos   : SV_POSITION;\n"
	"  float4 col   : COLOR0;\n"
	"  float  emph  : TEXCOORD0;\n"
	"  float  aflag : TEXCOORD1;\n"
	"};\n"

	"static const float DECAY_SATURATION = " MTNOTEDESIGN_STRINGIFY(MTNOTEDESIGN_DECAY_SATURATION_SMOOTH) ";\n"

	// S-curve decay (mirrors MTNoteDesignMod::GetDecayCoefficient)
	"float GetDecayCoeff(float rate) {\n"
	"  float c;\n"
	"  if (rate < 0.5) {\n"
	"    c = (exp2((0.5 - rate) * 8.0) + 14.0) / DECAY_SATURATION;\n"
	"  } else {\n"
	"    c = (16.0 - exp2((rate - 0.5) * 8.0)) / DECAY_SATURATION;\n"
	"  }\n"
	"  return saturate(c);\n"
	"}\n"

	// 3-phase envelope (mirrors MTNoteDesignMod::CalcNoteEnvelope in ms)
	"float CalcEnvelope(float playMs, float startMs, float endMs,\n"
	"                   float decDur, float relDur, float decR, float susR) {\n"
	"  float noteLen = endMs - startMs;\n"
	"  float relR = 1.0 - decR - susR;\n"
	"  if (noteLen < decDur) {\n"
	"  } else if (noteLen < decDur + relDur) {\n"
	"    relDur = noteLen - decDur;\n"
	"    decR = 0.5; susR = 0.0; relR = 0.5;\n"
	"  } else if (noteLen < (decDur + relDur) * 2.0) {\n"
	"    float mid = (startMs + decDur + endMs - relDur) * 0.5;\n"
	"    decDur = mid - startMs;\n"
	"    relDur = endMs - mid;\n"
	"    decR = 0.5; susR = 0.0; relR = 0.5;\n"
	"  }\n"
	"  float progress = playMs - startMs;\n"
	"  if (progress < decDur) {\n"
	"    return (decDur > 0.0) ? (decR * progress / decDur) : 0.0;\n"
	"  }\n"
	"  float sustainEnd = noteLen - relDur;\n"
	"  if (progress <= sustainEnd) {\n"
	"    float sLen = sustainEnd - decDur;\n"
	"    return (sLen > 0.0) ? (decR + susR * (progress - decDur) / sLen) : (decR + susR);\n"
	"  }\n"
	"  float rProg = progress - sustainEnd;\n"
	"  return (relDur > 0.0) ? (decR + susR + relR * rProg / relDur) : 1.0;\n"
	"}\n"

	"VSOUT VSMain(VSIN i) {\n"
	"  VSOUT o;\n"
	"  float playMs = g_Active.x;\n"
	"  float apass = g_Active.w;\n"
	"  float active = ((i.startMs <= playMs) && (playMs <= i.endMs)) ? 1.0 : 0.0;\n"

	// 2-pass visibility: pass 0 draws non-active, pass 1 draws active only
	"  float hide = abs(apass - active);\n"

	// Envelope for active notes (pass 1 only)
	"  float keyDownRate = 0.0;\n"
	"  float decayCoeff = 0.0;\n"
	"  if (active > 0.5 && apass > 0.5) {\n"
	"    keyDownRate = CalcEnvelope(playMs, i.startMs, i.endMs,\n"
	"                              g_Envelope.x, g_Envelope.y, g_Envelope.z, g_Envelope.w);\n"
	"    decayCoeff = GetDecayCoeff(keyDownRate);\n"
	"  }\n"

	// Size growth from envelope
	"  float growFactor = 1.0 + decayCoeff * g_Active.y;\n"
	"  float3 c   = (i.vmin + i.vmax) * 0.5;\n"
	"  float3 ext = (i.vmax - i.vmin) * 0.5;\n"
	"  float3 lo = float3(i.vmin.x, c.y - ext.y * growFactor, c.z - ext.z * growFactor);\n"
	"  float3 hi = float3(i.vmax.x, c.y + ext.y * growFactor, c.z + ext.z * growFactor);\n"
	"  float3 wp = lo + (hi - lo) * i.corner * (1.0 - hide);\n"

	// Pitch bend
	"  uint pbIdx = (uint)(i.pbIdxF + 0.5);\n"
	"  float pbShift = g_PB[pbIdx >> 2][pbIdx & 3];\n"
	"  wp.y += active * apass * pbShift;\n"

	"  o.pos = mul(float4(wp, 1.0), g_WVP);\n"

	// Lighting (2-lamp opposing directional, DX9-compatible)
	"  float3 n = normalize(mul(float4(i.nrm, 0.0), g_World).xyz);\n"
	"  float3 L = normalize(g_Light.xyz);\n"
	"  float ndl = saturate(dot(n, -L)) + saturate(dot(n, L));\n"
	"  float3 lit = saturate(i.color.rgb * (g_LAmb.x + g_Light.w * ndl));\n"
	"  o.col = float4(lerp(i.color.rgb, lit, g_LAmb.w), i.alpha);\n"
	"  o.emph = decayCoeff * g_Active.z;\n"
	"  o.aflag = (apass > 0.5) ? active : 0.0;\n"
	"  return o;\n"
	"}\n"

	"float4 PSMain(VSOUT i) : SV_TARGET {\n"
	"  float3 base = lerp(i.col.rgb, float3(1,1,1), saturate(i.emph));\n"
	"  base += i.aflag * g_Opts.yzw;\n"
	"  return float4(saturate(base), i.col.a);\n"
	"}\n";


//******************************************************************************
// Static pipeline members
//******************************************************************************
ID3D11VertexShader*      MTNoteBoxInstanced11::s_pVS         = nullptr;
ID3D11PixelShader*       MTNoteBoxInstanced11::s_pPS         = nullptr;
ID3D11InputLayout*       MTNoteBoxInstanced11::s_pLayout     = nullptr;
ID3D11Buffer*            MTNoteBoxInstanced11::s_pConstBuf   = nullptr;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteBoxInstanced11::MTNoteBoxInstanced11()
{
	m_pNoteDesign = nullptr;
	m_pNoteTracker = nullptr;
	m_pNotePitchBend = nullptr;
	m_pTemplateVB = nullptr;
	m_pInstanceVB = nullptr;
	m_pIndexBuffer = nullptr;
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	m_isLightEnable = true;
	m_is2D = false;
	m_NoteCount = 0;
	m_IndexCountPerInstance = 36;
	m_XPerTick = 1.0f;
	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
}

MTNoteBoxInstanced11::~MTNoteBoxInstanced11()
{
	Release();
}


//******************************************************************************
// Initialize static pipeline
//******************************************************************************
int MTNoteBoxInstanced11::InitPipeline(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;
	ID3DBlob* pErrors = nullptr;

	if (s_pVS != nullptr) return 0;

	hr = D3DCompile(MTNOTEBOXINST_SHADER, strlen(MTNOTEBOXINST_SHADER), "MTNoteBoxInstanced11",
	                nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &pVSBlob, &pErrors);
	if (FAILED(hr)) {
		result = YN_SET_ERR("Shader compile error (VS).", hr, 0);
		goto EXIT;
	}
	hr = D3DCompile(MTNOTEBOXINST_SHADER, strlen(MTNOTEBOXINST_SHADER), "MTNoteBoxInstanced11",
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
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT,       1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32_FLOAT,       1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, DXGI_FORMAT_R32_FLOAT,       1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 5, DXGI_FORMAT_R32_FLOAT,       1, 40, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
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

	result = InitCommonStates(pDevice);
	if (result != 0) goto EXIT;

EXIT:;
	if (pVSBlob) pVSBlob->Release();
	if (pPSBlob) pPSBlob->Release();
	if (pErrors) pErrors->Release();
	return result;
}


//******************************************************************************
// Release static pipeline
//******************************************************************************
void MTNoteBoxInstanced11::ReleasePipeline()
{
	if (s_pVS)          { s_pVS->Release();          s_pVS = nullptr; }
	if (s_pPS)          { s_pPS->Release();          s_pPS = nullptr; }
	if (s_pLayout)      { s_pLayout->Release();      s_pLayout = nullptr; }
	if (s_pConstBuf)    { s_pConstBuf->Release();    s_pConstBuf = nullptr; }
}


//******************************************************************************
// Create
//******************************************************************************
int MTNoteBoxInstanced11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNoteTracker* pNoteTracker,
		MTNotePitchBend* pNotePitchBend,
		MTNoteDesignMod* pNoteDesign,
		bool is2D
	)
{
	int result = 0;

	Release();

	if (pSeqData == NULL || pNoteTracker == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_pNoteTracker = pNoteTracker;
	m_pNotePitchBend = pNotePitchBend;
	m_is2D = is2D;

	if (pNoteDesign != NULL) {
		m_pNoteDesign = pNoteDesign;
	}
	else {
		result = m_NoteDesignLocal.Initialize(pSceneName, pSeqData);
		if (result != 0) goto EXIT;
		m_pNoteDesign = &m_NoteDesignLocal;
	}

	m_NoteCount = m_pNoteTracker->GetNoteCount();
	m_XPerTick = m_pNoteDesign->GetPlayPosX(1);

	result = InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = _CreateTemplateGeometry(pDevice);
	if (result != 0) goto EXIT;

	result = _CreateInstanceBuffer(pDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// Create template geometry (24-vertex box)
//******************************************************************************
int MTNoteBoxInstanced11::_CreateTemplateGeometry(ID3D11Device* pDevice)
{
	int result = 0;

	if (m_is2D) {
		// 2D mode: Z-facing face (4 vertices, 6 indices)
		// Spans X (time) × Y (pitch) at fixed Z=vmin.z
		// NoteBoxWidth=0.0 でも Y 方向（NoteBoxHeight）で展開されるため退化しない
		MTNOTEBOX_INST_TEMPLATE_VERTEX verts[4] = {
			{{0,1,0}, {0, 0, -1}},   // startX, topY, Z
			{{1,1,0}, {0, 0, -1}},   // endX, topY, Z
			{{0,0,0}, {0, 0, -1}},   // startX, bottomY, Z
			{{1,0,0}, {0, 0, -1}},   // endX, bottomY, Z
		};
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER,
		                               verts, sizeof(verts), &m_pTemplateVB);
		if (result != 0) goto EXIT;

		unsigned long indices[6] = { 0, 1, 2, 2, 1, 3 };
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_INDEX_BUFFER,
		                               indices, sizeof(indices), &m_pIndexBuffer);
		if (result != 0) goto EXIT;

		m_IndexCountPerInstance = 6;
	}
	else {
		// 3D mode: full box (24 vertices, 36 indices)
		MTNOTEBOX_INST_TEMPLATE_VERTEX verts[24] = {
			// Top face (normal 0,+1,0)
			{{0,1,1}, {0, 1, 0}}, {{1,1,1}, {0, 1, 0}}, {{0,1,0}, {0, 1, 0}}, {{1,1,0}, {0, 1, 0}},
			// Bottom face (normal 0,-1,0)
			{{0,0,0}, {0,-1, 0}}, {{1,0,0}, {0,-1, 0}}, {{0,0,1}, {0,-1, 0}}, {{1,0,1}, {0,-1, 0}},
			// Right face (z=0, normal 0,0,-1)
			{{0,1,0}, {0, 0,-1}}, {{1,1,0}, {0, 0,-1}}, {{0,0,0}, {0, 0,-1}}, {{1,0,0}, {0, 0,-1}},
			// Left face (z=1, normal 0,0,+1)
			{{0,0,1}, {0, 0, 1}}, {{1,0,1}, {0, 0, 1}}, {{0,1,1}, {0, 0, 1}}, {{1,1,1}, {0, 0, 1}},
			// Front face (x=0, normal -1,0,0)
			{{0,1,1}, {-1, 0, 0}}, {{0,1,0}, {-1, 0, 0}}, {{0,0,1}, {-1, 0, 0}}, {{0,0,0}, {-1, 0, 0}},
			// Back face (x=1, normal +1,0,0)
			{{1,1,0}, { 1, 0, 0}}, {{1,1,1}, { 1, 0, 0}}, {{1,0,0}, { 1, 0, 0}}, {{1,0,1}, { 1, 0, 0}},
		};
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER,
		                               verts, sizeof(verts), &m_pTemplateVB);
		if (result != 0) goto EXIT;

		unsigned long indices[36] = {
			 0, 1, 2,  2, 1, 3,
			 4, 5, 6,  6, 5, 7,
			 8, 9,10, 10, 9,11,
			12,13,14, 14,13,15,
			16,17,18, 18,17,19,
			20,21,22, 22,21,23,
		};
		result = CreateImmutableBuffer(pDevice, D3D11_BIND_INDEX_BUFFER,
		                               indices, sizeof(indices), &m_pIndexBuffer);
		if (result != 0) goto EXIT;

		m_IndexCountPerInstance = 36;
	}

EXIT:;
	return result;
}


//******************************************************************************
// Create instance buffer
//******************************************************************************
int MTNoteBoxInstanced11::_CreateInstanceBuffer(ID3D11Device* pDevice)
{
	int result = 0;

	if (m_NoteCount == 0) goto EXIT;

	{
		std::vector<MTNOTEBOX_INST_INSTANCE> instances(m_NoteCount);
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

			// AABB from GetNoteBoxVirtexPos results:
			// [0]=LU(top,left), [1]=RU(top,right), [2]=LD(bottom,left), [3]=RD(bottom,right)
			instances[i].vmin[0] = startCorners[3].x;  // startX
			instances[i].vmin[1] = startCorners[3].y;  // bottomY
			instances[i].vmin[2] = startCorners[1].z;  // rightZ (low Z)
			instances[i].vmax[0] = endCorners[0].x;    // endX
			instances[i].vmax[1] = startCorners[0].y;  // topY
			instances[i].vmax[2] = startCorners[0].z;  // leftZ (high Z)

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
		                               (unsigned long)(m_NoteCount * sizeof(MTNOTEBOX_INST_INSTANCE)),
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
int MTNoteBoxInstanced11::Update(const MTSceneUpdateContext& ctx)
{
	m_CurTickTime = ctx.curTickTime;
	m_PlayTimeMSec = ctx.playTimeMSec;

	Vector3 moveVector = m_pNoteDesign->GetWorldMoveVector();
	Matrix rotateMatrix = Matrix::CreateRotationX(XMConvertToRadians(ctx.rollAngle));
	Matrix moveMatrix = Matrix::CreateTranslation(moveVector);
	Matrix world = rotateMatrix * moveMatrix;
	XMStoreFloat4x4(&m_World, world);

	return 0;
}


//******************************************************************************
// Draw
//******************************************************************************
int MTNoteBoxInstanced11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!IsEnable() || m_NoteCount == 0) goto EXIT;

	{
		unsigned long tickWindow = (unsigned long)(MTNOTEBOXINST_CULL_DISTANCE / m_XPerTick);
		unsigned long tickLow = (m_CurTickTime > tickWindow) ? (m_CurTickTime - tickWindow) : 0;
		unsigned long tickHigh = m_CurTickTime + tickWindow;

		unsigned long loNote = 0, hiNote = 0;
		GetVisibleRange(tickLow, tickHigh, &loNote, &hiNote);
		if (loNote >= hiNote) goto EXIT;

		unsigned long loActive = 0, hiActive = 0;
		GetVisibleRange(m_CurTickTime, m_CurTickTime, &loActive, &hiActive);

		MTEnvelopeConfig envConfig = m_pNoteDesign->GetEnvelopeConfig();

		pContext->IASetInputLayout(s_pLayout);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->VSSetShader(s_pVS, nullptr, 0);
		pContext->PSSetShader(s_pPS, nullptr, 0);
		BindCommonStates(pContext);

		UINT strides[2] = { sizeof(MTNOTEBOX_INST_TEMPLATE_VERTEX), sizeof(MTNOTEBOX_INST_INSTANCE) };
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
			HRESULT hr = pContext->Map(s_pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

			CBuffer* cb = (CBuffer*)mapped.pData;
			XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(wvp));
			XMStoreFloat4x4(&cb->world, XMMatrixTranspose(world));
			cb->active = XMFLOAT4((float)m_PlayTimeMSec, growFactor, whiteRate, (float)pass);
			cb->opts = XMFLOAT4(0.0f, emissive.R(), emissive.G(), emissive.B());
			cb->light = XMFLOAT4(lightDir.x, lightDir.y, lightDir.z, 1.2f);
			cb->lambient = XMFLOAT4(0.2f, 0.0f, 0.0f, m_isLightEnable ? 1.0f : 0.0f);
			cb->envelope = XMFLOAT4(envConfig.decayDurationMs, envConfig.releaseDurationMs,
			                       envConfig.decayRatio, envConfig.sustainRatio);

			FillPitchBendArray((float*)cb->pb, m_pNotePitchBend,
				[this](short v, unsigned char s) { return m_pNoteDesign->GetPitchBendShift(v, s); });

			pContext->Unmap(s_pConstBuf, 0);
			pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf);
			pContext->PSSetConstantBuffers(0, 1, &s_pConstBuf);

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
// Release
//******************************************************************************
void MTNoteBoxInstanced11::Release()
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
void MTNoteBoxInstanced11::Reset()
{
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
}


//******************************************************************************
// Note count
//******************************************************************************
unsigned long MTNoteBoxInstanced11::GetNoteCount() const
{
	return m_NoteCount;
}
