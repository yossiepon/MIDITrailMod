//******************************************************************************
//
// MIDITrail / MTNoteCylindricalInstanced11
//
// GPU-instanced note renderer for Ring scenes.
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Ced (MIDITrail Mod Mod). All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include <d3dcompiler.h>
#include "YNBaseLib.h"
#include "MTNoteCylindricalInstanced11.h"
#include "MTNoteInstancedShaderSnippets.h"
#include "DXPrimitive11.h"
#include "MTFormatUtil.h"
#include "RDDiagManager.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#pragma comment(lib, "d3dcompiler.lib")


//******************************************************************************
// Culling distance
//******************************************************************************
#define MTNOTECYLINDRICAL_CULL_DISTANCE  (2200.0f)


//******************************************************************************
// Inline HLSL shader (cylindrical corner mask)
//******************************************************************************
static const char* MTNOTECYLINDRICAL_SHADER =
	"cbuffer Constants : register(b0) {\n"
	"  float4x4 g_WVP;\n"
	"  float4 g_PB[32];\n"
	"  float4x4 g_World;\n"
	"  float4 g_Active;\n"     // x=playTimeMSec, y=growFactor, z=whiteRate, w=pass(0/1)
	"  float4 g_Opts;\n"       // x=unused, yzw=emissiveRGB
	"#ifdef HAS_LIGHTING\n"
	"  float4 g_Light;\n"      // xyz=lightDir, w=diffuseLevel
	"  float4 g_LAmb;\n"       // x=ambientLevel, y=unused, z=unused, w=lightEnable(0/1)
	"#endif\n"
	"  float4 g_Envelope;\n"   // x=decayDurMs, y=releaseDurMs, z=decayRatio, w=sustainRatio
	"  float4 g_Ring;\n"       // x=halfNoteWidth, y=halfAngleStep(deg), z/w=unused
	"};\n"

	"struct VSIN {\n"
	"  float3 corner : POSITION;\n"
	"  float3 nrm    : NORMAL;\n"
	"  float  xStart : TEXCOORD0;\n"
	"  float  xEnd   : TEXCOORD1;\n"
	"  float  rad    : TEXCOORD2;\n"
	"  float  ang    : TEXCOORD3;\n"
	"  float4 color  : COLOR0;\n"
	"  float  pbIdxF : TEXCOORD4;\n"
	"  float  alpha  : TEXCOORD5;\n"
	"  float  startMs: TEXCOORD6;\n"
	"  float  endMs  : TEXCOORD7;\n"
	"};\n"

	"struct VSOUT {\n"
	"  float4 pos   : SV_POSITION;\n"
	"  float4 col   : COLOR0;\n"
	"  float  emph  : TEXCOORD0;\n"
	"  float  aflag : TEXCOORD1;\n"
	"};\n"

	"static const float PI = 3.14159265358979;\n"
	MTNOTEINST_HLSL_SHARED_FUNCTIONS

	"VSOUT VSMain(VSIN i) {\n"
	"  VSOUT o;\n"
	"  float playMs = g_Active.x;\n"
	"  float apass = g_Active.w;\n"
	"  float active = ((i.startMs <= playMs) && (playMs <= i.endMs)) ? 1.0 : 0.0;\n"
	"  float hide = abs(apass - active);\n"

	// Envelope
	"  float keyDownRate = 0.0;\n"
	"  float decayCoeff = 0.0;\n"
	"  if (active > 0.5 && apass > 0.5) {\n"
	"    keyDownRate = CalcEnvelope(playMs, i.startMs, i.endMs,\n"
	"                              g_Envelope.x, g_Envelope.y, g_Envelope.z, g_Envelope.w);\n"
	"    decayCoeff = GetDecayCoeff(keyDownRate);\n"
	"  }\n"

	// Cylindrical corner mask with grow
	"  float growFactor = 1.0 + decayCoeff * g_Active.y;\n"
	"  float x = lerp(i.xStart, i.xEnd, i.corner.x);\n"
	"  float r = i.rad + (i.corner.y - 0.5) * g_Ring.x * 2.0 * growFactor;\n"
	"  float halfAng = g_Ring.y * growFactor;\n"

	// Pitch bend angle shift
	"  uint pbIdx = (uint)(i.pbIdxF + 0.5);\n"
	"  float pbShift = g_PB[pbIdx >> 2][pbIdx & 3];\n"
	"  float theta = (i.ang - pbShift + (i.corner.z - 0.5) * halfAng * 2.0) * PI / 180.0;\n"

	// Cylindrical to Cartesian
	"  float3 wp = float3(x, cos(theta) * r, -sin(theta) * r);\n"

	// Collapse hidden notes: all vertices converge to note center (degenerate triangles)
	"  float3 center = float3(i.xStart, cos(i.ang * PI / 180.0) * i.rad, -sin(i.ang * PI / 180.0) * i.rad);\n"
	"  wp = lerp(wp, center, hide);\n"
	"  o.pos = mul(float4(wp, 1.0), g_WVP);\n"

	// Lighting (radial normal) -- only when HAS_LIGHTING is defined
	"#ifdef HAS_LIGHTING\n"
	"  float3 n = normalize(float3(0, cos(theta), -sin(theta)));\n"
	"  n = normalize(mul(float4(n, 0.0), g_World).xyz);\n"
	"  float3 L = normalize(g_Light.xyz);\n"
	"  float ndl = saturate(dot(n, -L)) + saturate(dot(n, L));\n"
	"  float3 lit = saturate(i.color.rgb * (g_LAmb.x + g_Light.w * ndl));\n"
	"  o.col = float4(lerp(i.color.rgb, lit, g_LAmb.w), i.alpha);\n"
	"#else\n"
	"  o.col = float4(i.color.rgb, i.alpha);\n"
	"#endif\n"
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
ID3D11VertexShader*      MTNoteCylindricalInstanced11::s_pVS       = nullptr;
ID3D11PixelShader*       MTNoteCylindricalInstanced11::s_pPS       = nullptr;
ID3D11InputLayout*       MTNoteCylindricalInstanced11::s_pLayout   = nullptr;
ID3D11Buffer*            MTNoteCylindricalInstanced11::s_pConstBuf = nullptr;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteCylindricalInstanced11::MTNoteCylindricalInstanced11()
{
	m_pNoteDesign = nullptr;
	m_pNoteTracker = nullptr;
	m_pNotePitchBend = nullptr;
	m_pTemplateVB = nullptr;
	m_pInstanceVB = nullptr;
	m_pIndexBuffer = nullptr;
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	m_NoteCount = 0;
	m_XPerTick = 1.0f;
	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
}

MTNoteCylindricalInstanced11::~MTNoteCylindricalInstanced11()
{
	Release();
}


//******************************************************************************
// Initialize static pipeline
//******************************************************************************
int MTNoteCylindricalInstanced11::InitPipeline(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;
	ID3DBlob* pErrors = nullptr;

	if (s_pVS != nullptr) return 0;

	D3D_SHADER_MACRO defines[] = {
		{ nullptr, nullptr }
	};
	hr = D3DCompile(MTNOTECYLINDRICAL_SHADER, strlen(MTNOTECYLINDRICAL_SHADER), "MTNoteCylindricalInstanced11",
	                defines, nullptr, "VSMain", "vs_5_0", 0, 0, &pVSBlob, &pErrors);
	if (FAILED(hr)) {
		result = YN_SET_ERR("Shader compile error (VS).", hr, 0);
		goto EXIT;
	}
	hr = D3DCompile(MTNOTECYLINDRICAL_SHADER, strlen(MTNOTECYLINDRICAL_SHADER), "MTNoteCylindricalInstanced11",
	                defines, nullptr, "PSMain", "ps_5_0", 0, 0, &pPSBlob, &pErrors);
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
			{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT,       1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // xStart
			{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,       1,  4, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // xEnd
			{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT,       1,  8, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // radius
			{ "TEXCOORD", 3, DXGI_FORMAT_R32_FLOAT,       1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // angle0
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // color
			{ "TEXCOORD", 4, DXGI_FORMAT_R32_FLOAT,       1, 20, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // pbIndex
			{ "TEXCOORD", 5, DXGI_FORMAT_R32_FLOAT,       1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // alpha
			{ "TEXCOORD", 6, DXGI_FORMAT_R32_FLOAT,       1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // startTimeMs
			{ "TEXCOORD", 7, DXGI_FORMAT_R32_FLOAT,       1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // endTimeMs
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

	DXPrimitive11::RegisterDeviceCleanup([]{ MTNoteCylindricalInstanced11::ReleasePipeline(); });

EXIT:;
	if (pVSBlob) pVSBlob->Release();
	if (pPSBlob) pPSBlob->Release();
	if (pErrors) pErrors->Release();
	return result;
}


//******************************************************************************
// Release static pipeline
//******************************************************************************
void MTNoteCylindricalInstanced11::ReleasePipeline()
{
	if (s_pVS)       { s_pVS->Release();       s_pVS = nullptr; }
	if (s_pPS)       { s_pPS->Release();       s_pPS = nullptr; }
	if (s_pLayout)   { s_pLayout->Release();   s_pLayout = nullptr; }
	if (s_pConstBuf) { s_pConstBuf->Release(); s_pConstBuf = nullptr; }
}


//******************************************************************************
// Create
//******************************************************************************
int MTNoteCylindricalInstanced11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNoteTracker* pNoteTracker,
		MTNotePitchBend* pNotePitchBend,
		MTNoteDesignRing11* pNoteDesign,
		const MTLoadProgressContext* pProgress
	)
{
	int result = 0;

	Release();

	m_pProgress = pProgress;

	if (pSeqData == NULL || pNoteTracker == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

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
// Create template geometry (4-vertex quad in cylindrical space)
//******************************************************************************
int MTNoteCylindricalInstanced11::_CreateTemplateGeometry(ID3D11Device* pDevice)
{
	int result = 0;

	// C++20: declare before the first goto below so the jump to EXIT does not
	// bypass this variable's initialization while it remains in scope (C2362).
	unsigned long indices[6] = {};

	// corner: (x_mask, r_mask, angle_mask)
	// x_mask: 0=xStart, 1=xEnd (not used for Ring flat quads, kept at 0)
	// r_mask: 0=inner(0), 1=outer(1)
	// angle_mask: 0=left(0), 1=right(1)
	// Ring flat quad: spans time (corner.x) × angle (corner.z) at outer radius (corner.y=1)
	MTNOTECYLINDRICAL_INST_TEMPLATE_VERTEX verts[4] = {
		{{0, 1, 0}, {0, 1, 0}},   // xStart, outer, leftAngle
		{{1, 1, 0}, {0, 1, 0}},   // xEnd, outer, leftAngle
		{{1, 1, 1}, {0, 1, 0}},   // xEnd, outer, rightAngle
		{{0, 1, 1}, {0, 1, 0}},   // xStart, outer, rightAngle
	};

	result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER,
	                               verts, sizeof(verts), &m_pTemplateVB);
	if (result != 0) goto EXIT;

	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	result = CreateImmutableBuffer(pDevice, D3D11_BIND_INDEX_BUFFER,
	                               indices, sizeof(indices), &m_pIndexBuffer);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// Create instance buffer
//******************************************************************************
int MTNoteCylindricalInstanced11::_CreateInstanceBuffer(ID3D11Device* pDevice)
{
	int result = 0;

	if (m_NoteCount == 0) goto EXIT;

	{
		std::vector<MTNOTECYLINDRICAL_INST_INSTANCE> instances(m_NoteCount);
		std::vector<unsigned long> startTicks(m_NoteCount);
		std::vector<unsigned long> endTicks(m_NoteCount);

		for (unsigned long i = 0; i < m_NoteCount; i++) {
			const NoteData& note = m_pNoteTracker->GetNote(i);

			instances[i].xStart = m_pNoteDesign->GetPlayPosX(note.startTimeTick);
			instances[i].xEnd   = m_pNoteDesign->GetPlayPosX(note.endTimeTick);

			// Radius = port origin Y + channel offset
			instances[i].radius = m_pNoteDesign->GetPortOriginY(note.portNo)
			                    + m_pNoteDesign->GetChStep() * note.chNo;

			// Base angle (without PB) = -(noteAngleStep * noteNo + noteAngleStep/2)
			float angleStep = m_pNoteDesign->GetNoteAngleStep();
			instances[i].angle0 = -((angleStep * note.noteNo) + (angleStep / 2.0f));

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

			if (m_pProgress != NULL && (i & 0x3FFF) == 0) {
				char fmtA[32], fmtB[32], msg[80];
				MTFormatWithCommas(fmtA, sizeof(fmtA), i);
				MTFormatWithCommas(fmtB, sizeof(fmtB), m_NoteCount);
				snprintf(msg, sizeof(msg), "Rendering notes: %s / %s", fmtA, fmtB);
				m_pProgress->Fire(i, m_NoteCount, msg);
			}
		}

		result = CreateImmutableBuffer(pDevice, D3D11_BIND_VERTEX_BUFFER,
		                               instances.data(),
		                               (unsigned long)(m_NoteCount * sizeof(MTNOTECYLINDRICAL_INST_INSTANCE)),
		                               &m_pInstanceVB);
		if (result != 0) goto EXIT;
		RDDiagManager::SetInt(RDMetricId::RenderInstanceBufferSizeKB,
			static_cast<int64_t>(m_NoteCount * sizeof(MTNOTECYLINDRICAL_INST_INSTANCE) / 1024));

		BuildCullingArrays(startTicks.data(), endTicks.data(), m_NoteCount);
	}

EXIT:;
	return result;
}


//******************************************************************************
// Update
//******************************************************************************
int MTNoteCylindricalInstanced11::Update(const MTSceneUpdateContext& ctx)
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
int MTNoteCylindricalInstanced11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!IsEnable() || m_NoteCount == 0) goto EXIT;

	{
		unsigned long tickWindow = (unsigned long)(MTNOTECYLINDRICAL_CULL_DISTANCE / m_XPerTick);
		unsigned long tickLow = (m_CurTickTime > tickWindow) ? (m_CurTickTime - tickWindow) : 0;
		unsigned long tickHigh = ((0xFFFFFFFF - m_CurTickTime) < tickWindow)
		                         ? 0xFFFFFFFF : (m_CurTickTime + tickWindow);

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

		UINT strides[2] = { sizeof(MTNOTECYLINDRICAL_INST_TEMPLATE_VERTEX), sizeof(MTNOTECYLINDRICAL_INST_INSTANCE) };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* buffers[2] = { m_pTemplateVB, m_pInstanceVB };
		pContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		Matrix world = XMLoadFloat4x4(&m_World);
		Matrix wvp = world * viewProj;

		Color emissive = m_pNoteDesign->GetActiveNoteEmissive();
		float growFactor = m_pNoteDesign->GetActiveNoteBoxSizeRatio() - 1.0f;
		float whiteRate = m_pNoteDesign->GetActiveNoteWhiteRate();
		float halfWidth = m_pNoteDesign->GetNoteBoxWidth() / 2.0f;
		float halfAngle = m_pNoteDesign->GetNoteAngleStep() / 2.0f;

		for (int pass = 0; pass < 2; pass++) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = pContext->Map(s_pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

			CBuffer* cb = (CBuffer*)mapped.pData;
			XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(wvp));
			XMStoreFloat4x4(&cb->world, XMMatrixTranspose(world));
			cb->active = XMFLOAT4((float)m_PlayTimeMSec, growFactor, whiteRate, (float)pass);
			cb->opts = XMFLOAT4(0.0f, emissive.R(), emissive.G(), emissive.B());
			cb->envelope = XMFLOAT4(envConfig.decayDurationMs, envConfig.releaseDurationMs,
			                       envConfig.decayRatio, envConfig.sustainRatio);
			cb->ringParams = XMFLOAT4(halfWidth, halfAngle, 0.0f, 0.0f);

			FillPitchBendArray((float*)cb->pb, m_pNotePitchBend,
				[this](short v, unsigned char s) { return m_pNoteDesign->GetPitchBendAngleShift(v, s); });

			pContext->Unmap(s_pConstBuf, 0);
			pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf);
			pContext->PSSetConstantBuffers(0, 1, &s_pConstBuf);

			if (pass == 0) {
				pContext->DrawIndexedInstanced(6, hiNote - loNote, 0, 0, loNote);
				int64_t prev = RDDiagManager::GetInt(RDMetricId::RenderInstanceCount);
				RDDiagManager::SetInt(RDMetricId::RenderInstanceCount, prev + (hiNote - loNote));
			}
			else {
				if (loActive < hiActive) {
					pContext->DrawIndexedInstanced(6, hiActive - loActive, 0, 0, loActive);
					int64_t prev = RDDiagManager::GetInt(RDMetricId::RenderInstanceCount);
					RDDiagManager::SetInt(RDMetricId::RenderInstanceCount, prev + (hiActive - loActive));
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
void MTNoteCylindricalInstanced11::Release()
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
void MTNoteCylindricalInstanced11::Reset()
{
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
}


//******************************************************************************
// Note count
//******************************************************************************
unsigned long MTNoteCylindricalInstanced11::GetNoteCount() const
{
	return m_NoteCount;
}
