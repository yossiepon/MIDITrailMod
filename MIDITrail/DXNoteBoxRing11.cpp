//******************************************************************************
//
// MIDITrail / DXNoteBoxRing11
//
// DX11 instanced Ring-scene note renderer (M4.9) - port of MTNoteBoxRing.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "DXNoteBoxRing11.h"
#include <d3dcompiler.h>

using namespace YNBaseLib;
using namespace DirectX;

#define DXNBR11_CULL_DISTANCE  (2200.0f)

ID3D11VertexShader*      DXNoteBoxRing11::s_pVS = NULL;
ID3D11PixelShader*       DXNoteBoxRing11::s_pPS = NULL;
ID3D11InputLayout*       DXNoteBoxRing11::s_pLayout = NULL;
ID3D11Buffer*            DXNoteBoxRing11::s_pConstBuf = NULL;
ID3D11Buffer*            DXNoteBoxRing11::s_pTemplateVB = NULL;
ID3D11Buffer*            DXNoteBoxRing11::s_pBoxIB = NULL;
ID3D11RasterizerState*   DXNoteBoxRing11::s_pRaster = NULL;
ID3D11BlendState*        DXNoteBoxRing11::s_pBlend = NULL;
ID3D11DepthStencilState* DXNoteBoxRing11::s_pDepth = NULL;

struct DXNBR11_CONSTANTS {
	XMFLOAT4X4 wvp;
	XMFLOAT4   ring;     // x = radial half-width, y = angular half-step (deg), z = now-line X
	XMFLOAT4   pb[64];   // per-(port&0xF,ch) pitch-bend angle shift (deg), indexed by color.a
};

// corner.x -> time (xStart/xEnd); corner.y -> inner/outer radius; corner.z ->
// angle1/angle2. Ring placement: y = cos(a)*r, z = -sin(a)*r (matches DXH::RotateYZ).
// M4.21 pitch bend: the color alpha byte holds a (port&0xF,ch) index; a note whose
// time span (xStart..xEnd) contains the now-line (g_Ring.z) is sounding -> its
// angle shifts by -g_PB[idx] (DX9 MTNoteDesignRing::_GetNoteAngle adds pb inside the
// negated angle, so a positive bend rotates it the same way as the box scenes).
static const char* DXNBR11_SHADER =
	"cbuffer Constants : register(b0) {\n"
	"  row_major float4x4 g_WVP;\n"
	"  float4 g_Ring;\n"
	"  float4 g_PB[64];\n"
	"};\n"
	"struct VSIN {\n"
	"  float3 corner : POSITION;\n"
	"  float4 inst   : TEXCOORD0;\n"   // xStart, xEnd, radius, angle0(deg)
	"  float4 color  : COLOR0;\n"
	"};\n"
	"struct VSOUT { float4 pos : SV_POSITION; float4 col : COLOR0; };\n"
	"VSOUT VSMain(VSIN i) {\n"
	"  VSOUT o;\n"
	"  uint idx = (uint)(i.color.a * 255.0 + 0.5);\n"
	"  float active = ((i.inst.x <= g_Ring.z) && (g_Ring.z <= i.inst.y)) ? 1.0 : 0.0;\n"
	"  float bf = (g_Ring.w >= 0.5) ? 1.0 : active;\n"   // g_Ring.w = bend whole channel
	"  float x = lerp(i.inst.x, i.inst.y, i.corner.x);\n"
	"  float r = i.inst.z + (i.corner.y * 2.0 - 1.0) * g_Ring.x;\n"
	"  float a = i.inst.w - bf * g_PB[idx >> 2][idx & 3] + (i.corner.z * 2.0 - 1.0) * g_Ring.y;\n"
	"  float rad = radians(a);\n"
	"  float3 wp = float3(x, cos(rad) * r, -sin(rad) * r);\n"
	"  o.pos = mul(float4(wp, 1.0), g_WVP);\n"
	"  o.col = float4(i.color.rgb, 1.0);\n"
	"  return o;\n"
	"}\n"
	"float4 PSMain(VSOUT i) : SV_TARGET { return i.col; }\n";


DXNoteBoxRing11::DXNoteBoxRing11()
{
	m_pPitchBend = NULL;
	m_BendAllNotes = false;
	m_Ready = false;
	m_CurTickTime = 0;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_HalfWidth = 0.0f;
	m_HalfAngleDeg = 0.0f;
	m_pInstanceVB = NULL;
	m_AllNoteNum = 0;
	m_pNoteStartTime = NULL;
	m_pNoteMaxEndTime = NULL;
	m_pNoteTrackNo = NULL;
}

DXNoteBoxRing11::~DXNoteBoxRing11()
{
	Release();
}

void DXNoteBoxRing11::Release()
{
	if (m_pInstanceVB != NULL) { m_pInstanceVB->Release(); m_pInstanceVB = NULL; }
	if (m_pNoteStartTime != NULL) { delete [] m_pNoteStartTime; m_pNoteStartTime = NULL; }
	if (m_pNoteMaxEndTime != NULL) { delete [] m_pNoteMaxEndTime; m_pNoteMaxEndTime = NULL; }
	if (m_pNoteTrackNo != NULL) { delete [] m_pNoteTrackNo; m_pNoteTrackNo = NULL; }
	m_NoteList.Clear();
	m_AllNoteNum = 0;
	m_Ready = false;
}

//******************************************************************************
// Shared pipeline
//******************************************************************************
int DXNoteBoxRing11::InitPipeline(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pPSBlob = NULL;
	ID3DBlob* pErr = NULL;

	if (s_pVS != NULL) return 0;
	if (pDevice == NULL) return YN_SET_ERR("Program error.", 0, 0);

	hr = D3DCompile(DXNBR11_SHADER, strlen(DXNBR11_SHADER), NULL, NULL, NULL, "VSMain", "vs_4_0", 0, 0, &pVSBlob, &pErr);
	if (FAILED(hr) || (pVSBlob == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }
	hr = D3DCompile(DXNBR11_SHADER, strlen(DXNBR11_SHADER), NULL, NULL, NULL, "PSMain", "ps_4_0", 0, 0, &pPSBlob, &pErr);
	if (FAILED(hr) || (pPSBlob == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &s_pVS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &s_pPS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	{
		D3D11_INPUT_ELEMENT_DESC il[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,     1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		hr = pDevice->CreateInputLayout(il, 3, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &s_pLayout);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_BUFFER_DESC cb;
		ZeroMemory(&cb, sizeof(cb));
		cb.ByteWidth = sizeof(DXNBR11_CONSTANTS);
		cb.Usage = D3D11_USAGE_DYNAMIC;
		cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr = pDevice->CreateBuffer(&cb, NULL, &s_pConstBuf);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		static const float corners[8][3] = {
			{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}
		};
		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA sr;
		ZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(corners);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ZeroMemory(&sr, sizeof(sr));
		sr.pSysMem = corners;
		hr = pDevice->CreateBuffer(&bd, &sr, &s_pTemplateVB);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		static const unsigned short idx[36] = {
			0,1,2, 2,1,3,   4,5,6, 6,5,7,
			0,1,4, 4,1,5,   2,3,6, 6,3,7,
			0,2,4, 4,2,6,   1,3,5, 5,3,7
		};
		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA sr;
		ZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(idx);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ZeroMemory(&sr, sizeof(sr));
		sr.pSysMem = idx;
		hr = pDevice->CreateBuffer(&bd, &sr, &s_pBoxIB);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_RASTERIZER_DESC rd;
		ZeroMemory(&rd, sizeof(rd));
		rd.FillMode = D3D11_FILL_SOLID;
		rd.CullMode = D3D11_CULL_NONE;
		rd.DepthClipEnable = TRUE;
		rd.MultisampleEnable = TRUE;
		hr = pDevice->CreateRasterizerState(&rd, &s_pRaster);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		// Ring notes are opaque (the PS always outputs alpha = 1.0), so SrcAlpha/
		// InvSrcAlpha blending is a no-op (result = src) that still costs a
		// framebuffer read per fragment. Disable blend for the identical image
		// without the ROP dst read.
		D3D11_BLEND_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.RenderTarget[0].BlendEnable = FALSE;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&bd, &s_pBlend);
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
	}

EXIT:;
	if (pVSBlob != NULL) pVSBlob->Release();
	if (pPSBlob != NULL) pPSBlob->Release();
	if (pErr != NULL) pErr->Release();
	if (result != 0) ReleasePipeline();
	return result;
}

void DXNoteBoxRing11::ReleasePipeline()
{
	if (s_pDepth != NULL)      { s_pDepth->Release();      s_pDepth = NULL; }
	if (s_pBlend != NULL)      { s_pBlend->Release();      s_pBlend = NULL; }
	if (s_pRaster != NULL)     { s_pRaster->Release();     s_pRaster = NULL; }
	if (s_pBoxIB != NULL)      { s_pBoxIB->Release();      s_pBoxIB = NULL; }
	if (s_pTemplateVB != NULL) { s_pTemplateVB->Release(); s_pTemplateVB = NULL; }
	if (s_pConstBuf != NULL)   { s_pConstBuf->Release();   s_pConstBuf = NULL; }
	if (s_pLayout != NULL)     { s_pLayout->Release();     s_pLayout = NULL; }
	if (s_pPS != NULL)         { s_pPS->Release();         s_pPS = NULL; }
	if (s_pVS != NULL)         { s_pVS->Release();         s_pVS = NULL; }
}

//******************************************************************************
// Create
//******************************************************************************
int DXNoteBoxRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMTrack track;
	D3DXVECTOR3 mv;

	(void)pContext;

	Release();

	result = InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	// track color mode keeps each note's source track (lost by GetMergedTrack)
	if (m_NoteDesign.IsTrackColorMode()) {
		std::vector<unsigned char> trackNoList;
		result = MTNoteDesign::BuildMergedNoteListWithTrack(pSeqData, &m_NoteList, &trackNoList);
		if (result != 0) goto EXIT;
		if (!trackNoList.empty()) {
			try { m_pNoteTrackNo = new unsigned char[trackNoList.size()]; }
			catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
			memcpy(m_pNoteTrackNo, &trackNoList[0], trackNoList.size());
		}
	}
	else {
		result = pSeqData->GetMergedTrack(&track);
		if (result != 0) goto EXIT;
		result = track.GetNoteList(&m_NoteList);
		if (result != 0) goto EXIT;
	}

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);
	m_HalfWidth = m_NoteDesign.GetNoteBoxWidth() / 2.0f;
	m_HalfAngleDeg = (360.0f / (float)SM_MAX_NOTE_NUM) / 2.0f;

	result = _CreateInstanceBuffer(pDevice);
	if (result != 0) goto EXIT;

	m_NoteList.Clear();
	if (m_pNoteTrackNo != NULL) { delete [] m_pNoteTrackNo; m_pNoteTrackNo = NULL; }
	m_Ready = (m_AllNoteNum > 0);

EXIT:;
	return result;
}

//******************************************************************************
// Build the per-note instance buffer (+ culling arrays)
//******************************************************************************
int DXNoteBoxRing11::_CreateInstanceBuffer(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;
	unsigned long i = 0;
	unsigned long maxEnd = 0;
	DXNBR11_INSTANCE* pInst = NULL;
	SMNote note;
	float step = 360.0f / (float)SM_MAX_NOTE_NUM;

	m_AllNoteNum = m_NoteList.GetSize();
	if (m_AllNoteNum == 0) goto EXIT;

	try {
		m_pNoteStartTime = new unsigned long[m_AllNoteNum];
		m_pNoteMaxEndTime = new unsigned long[m_AllNoteNum];
		pInst = new DXNBR11_INSTANCE[m_AllNoteNum];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	for (i = 0; i < m_AllNoteNum; i++) {
		result = m_NoteList.GetNote(i, &note);
		if (result != 0) goto EXIT;

		m_pNoteStartTime[i] = note.startTime;
		if (i == 0) maxEnd = note.endTime;
		else if (note.endTime > maxEnd) maxEnd = note.endTime;
		m_pNoteMaxEndTime[i] = maxEnd;

		pInst[i].xStart = m_NoteDesign.GetPlayPosX(note.startTime);
		pInst[i].xEnd   = m_NoteDesign.GetPlayPosX(note.endTime);
		pInst[i].radius = m_NoteDesign.GetPortOriginY(note.portNo) + (m_NoteDesign.GetChStep() * note.chNo);
		// _GetNoteAngle (no pitch bend): angle = -((step*noteNo) + step/2)
		pInst[i].angle0 = -((step * (float)note.noteNo) + (step / 2.0f));
		// M4.21: stash a (port&0xF,ch) index in the unused alpha byte (the shader
		// forces alpha=1) so the VS can look up this note's pitch-bend angle shift.
		{
			unsigned int idx = (unsigned int)(((note.portNo & 0x0F) << 4) | (note.chNo & 0x0F));
			D3DCOLOR col = (m_pNoteTrackNo != NULL)
				? (D3DCOLOR)m_NoteDesign.GetTrackChannelColor(m_pNoteTrackNo[i], note.chNo)
				: (D3DCOLOR)m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
			pInst[i].color = (col & 0x00FFFFFF) | ((unsigned long)idx << 24);
		}
	}

	{
		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA sr;
		ZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(DXNBR11_INSTANCE) * m_AllNoteNum;
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ZeroMemory(&sr, sizeof(sr));
		sr.pSysMem = pInst;
		hr = pDevice->CreateBuffer(&bd, &sr, &m_pInstanceVB);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, m_AllNoteNum); goto EXIT; }
	}

EXIT:;
	if (pInst != NULL) delete [] pInst;
	return result;
}

//******************************************************************************
// Visible note range (binary search on tick arrays, X = time)
//******************************************************************************
void DXNoteBoxRing11::_GetVisibleNoteRange(unsigned long* pLoNote, unsigned long* pHiNote)
{
	unsigned long lo = 0, hi = m_AllNoteNum;
	unsigned long tickLow = 0, tickHigh = 0, halfTicks = 0;
	float xPerTick = 0.0f;
	unsigned long left = 0, right = 0, mid = 0;

	*pLoNote = 0;
	*pHiNote = m_AllNoteNum;
	if (m_AllNoteNum == 0) return;
	if ((m_pNoteStartTime == NULL) || (m_pNoteMaxEndTime == NULL)) return;

	xPerTick = m_NoteDesign.GetPlayPosX(1 << 20) / (float)(1 << 20);
	if (xPerTick <= 0.0f) return;

	halfTicks = (unsigned long)(DXNBR11_CULL_DISTANCE / xPerTick);
	tickLow = (m_CurTickTime > halfTicks) ? (m_CurTickTime - halfTicks) : 0;
	if ((0xFFFFFFFF - m_CurTickTime) < halfTicks) tickHigh = 0xFFFFFFFF;
	else tickHigh = m_CurTickTime + halfTicks;

	left = 0; right = m_AllNoteNum;
	while (left < right) {
		mid = left + (right - left) / 2;
		if (m_pNoteMaxEndTime[mid] < tickLow) left = mid + 1;
		else right = mid;
	}
	lo = left;

	left = 0; right = m_AllNoteNum;
	while (left < right) {
		mid = left + (right - left) / 2;
		if (m_pNoteStartTime[mid] <= tickHigh) left = mid + 1;
		else right = mid;
	}
	hi = left;

	if (hi < lo) hi = lo;
	*pLoNote = lo;
	*pHiNote = hi;
}

unsigned long DXNoteBoxRing11::GetPlayedNoteCount(unsigned long curTick)
{
	unsigned long left = 0, right = m_AllNoteNum, mid = 0;
	if ((m_pNoteStartTime == NULL) || (m_AllNoteNum == 0)) return 0;
	while (left < right) {
		mid = left + (right - left) / 2;
		if (m_pNoteStartTime[mid] <= curTick) left = mid + 1;
		else right = mid;
	}
	return left;
}

//******************************************************************************
// Draw  (world = RotX(roll) * Trans(worldMove); roll spins the ring)
//******************************************************************************
int DXNoteBoxRing11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		float rollAngle
	)
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE ms;
	unsigned long loNote = 0, hiNote = 0;
	ID3D11Buffer* vbs[2];
	UINT strides[2];
	UINT offsets[2] = { 0, 0 };
	float blendFactor[4] = { 0, 0, 0, 0 };

	if (!m_Ready || (m_pInstanceVB == NULL)) return 0;
	if (s_pVS == NULL) return YN_SET_ERR("Program error.", 0, 0);

	_GetVisibleNoteRange(&loNote, &hiNote);
	if (hiNote <= loNote) return 0;

	{
		XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
		               * XMMatrixTranslation(m_WorldMove.x, m_WorldMove.y, m_WorldMove.z);
		XMMATRIX wvp = world * viewProj;
		DXNBR11_CONSTANTS c;
		XMStoreFloat4x4(&c.wvp, wvp);
		// ring.z = now-line X (GetPlayPosX(curTick)); a note sounding when its time
		// span contains it. M4.21 pitch bend: per-(port&0xF,ch) angle shift (deg).
		c.ring = XMFLOAT4(m_HalfWidth, m_HalfAngleDeg, m_NoteDesign.GetPlayPosX(m_CurTickTime), m_BendAllNotes ? 1.0f : 0.0f);
		ZeroMemory(c.pb, sizeof(c.pb));
		if (m_pPitchBend != NULL) {
			float step = 360.0f / (float)SM_MAX_NOTE_NUM;   // = MTNoteDesignRing m_NoteAngleStep
			float* pbf = (float*)c.pb;
			for (int idx = 0; idx < 256; idx++) {
				unsigned char port = (unsigned char)(idx >> 4);
				unsigned char ch   = (unsigned char)(idx & 0x0F);
				short val = m_pPitchBend->GetValue(port, ch);
				unsigned char sens = m_pPitchBend->GetSensitivity(port, ch);
				pbf[idx] = (val < 0) ? (step * sens * ((float)val / 8192.0f))
				                     : (step * sens * ((float)val / 8191.0f));
			}
		}
		hr = pContext->Map(s_pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, 0);
		memcpy(ms.pData, &c, sizeof(c));
		pContext->Unmap(s_pConstBuf, 0);
	}

	vbs[0] = s_pTemplateVB;  strides[0] = sizeof(float) * 3;
	vbs[1] = m_pInstanceVB;  strides[1] = sizeof(DXNBR11_INSTANCE);

	pContext->IASetInputLayout(s_pLayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
	pContext->IASetIndexBuffer(s_pBoxIB, DXGI_FORMAT_R16_UINT, 0);
	pContext->VSSetShader(s_pVS, NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf);
	pContext->PSSetShader(s_pPS, NULL, 0);
	pContext->RSSetState(s_pRaster);
	pContext->OMSetBlendState(s_pBlend, blendFactor, 0xFFFFFFFF);
	pContext->OMSetDepthStencilState(s_pDepth, 0);

	pContext->DrawIndexedInstanced(36, hiNote - loNote, 0, 0, loNote);
	return 0;
}
