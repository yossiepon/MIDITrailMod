//******************************************************************************
//
// MIDITrail / DXNoteBox11
//
// Direct3D 11 instanced note-box renderer (M3)
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "DXNoteBox11.h"
#include <d3dcompiler.h>

using namespace YNBaseLib;
using namespace DirectX;

//--- culling: visible half-window distance in world units (matches DX9 path) ---
#define DXNB11_CULL_DISTANCE  (2200.0f)

// static pipeline objects
ID3D11VertexShader*      DXNoteBox11::s_pVS = NULL;
ID3D11PixelShader*       DXNoteBox11::s_pPS = NULL;
ID3D11InputLayout*       DXNoteBox11::s_pLayout = NULL;
ID3D11Buffer*            DXNoteBox11::s_pConstBuf = NULL;
ID3D11Buffer*            DXNoteBox11::s_pTemplateVB = NULL;
ID3D11Buffer*            DXNoteBox11::s_pBoxIB = NULL;
ID3D11RasterizerState*   DXNoteBox11::s_pRaster = NULL;
ID3D11BlendState*        DXNoteBox11::s_pBlend = NULL;
ID3D11DepthStencilState* DXNoteBox11::s_pDepth = NULL;

DXNoteBox11::BuildProgressFunc DXNoteBox11::s_BuildProgressFunc = NULL;
void*                          DXNoteBox11::s_BuildProgressUser = NULL;

void DXNoteBox11::SetBuildProgressCallback(BuildProgressFunc func, void* user)
{
	s_BuildProgressFunc = func;
	s_BuildProgressUser = user;
}

struct DXNB11_CONSTANTS {
	XMFLOAT4X4 wvp;
	XMFLOAT4X4 world;     // note field world matrix (rotates the normals, as DX9 does)
	XMFLOAT4    active;   // x = now-line X, y = grow amount, z = brighten amount, w = pass (0/1)
	XMFLOAT4    opts;     // x = bend whole channel (0/1); yzw = active-note emissive RGB
	XMFLOAT4    light;    // xyz = light travel direction (object space), w = diffuse level
	XMFLOAT4    lamb;     // rgb = ambient term, w = lighting enable (0/1)
	XMFLOAT4    pb[64];   // per-(port&0xF,ch) pitch-bend Y shift (256 entries)
};

// Active-note effect is done GPU-side (no extra memory): a note whose time span
// (vmin.x..vmax.x) contains the now-line (g_Active.x) is "being played" -> it
// grows in Y/Z (strongest right at the attack) and brightens. Mirrors the DX9
// active-note swell/emissive without a separate active buffer or note list.
static const char* DXNB11_SHADER =
	"cbuffer Constants : register(b0) {\n"
	"  row_major float4x4 g_WVP;\n"
	"  row_major float4x4 g_World;\n"
	"  float4 g_Active;\n"
	"  float4 g_Opts;\n"     // x = bend whole channel (0/1)
	"  float4 g_Light;\n"    // xyz = light travel direction (world space), w = diffuse level
	"  float4 g_LAmb;\n"     // rgb = ambient term, w = lighting enable (0/1)
	"  float4 g_PB[64];\n"   // per-(port&0xF,ch) pitch-bend Y shift, indexed by color.a
	"};\n"
	"struct VSIN {\n"
	"  float3 corner : POSITION;\n"
	"  float3 nrm    : NORMAL;\n"      // face normal (flat-shaded box, 24 template verts)
	"  float3 vmin   : TEXCOORD0;\n"
	"  float3 vmax   : TEXCOORD1;\n"
	"  float4 color  : COLOR0;\n"
	"  float  hidden : TEXCOORD2;\n"
	"  float  alpha  : TEXCOORD3;\n"   // real note opacity (color.a is the pitch-bend index)
	"};\n"
	"struct VSOUT { float4 pos : SV_POSITION; float4 col : COLOR0; float emph : TEXCOORD0; float aflag : TEXCOORD1; };\n"
	// Two passes (g_Active.w): pass 0 = the notes that are NOT sounding, at base size;
	// pass 1 = only the sounding notes, swollen + white-flashed + pitch-bent, drawn
	// AFTER pass 0 so they sit ON TOP of the normal notes - matching DX9, which draws
	// a separate active-note buffer last.
	// ced 20260713: pass 0 now hides the sounding notes, as DX9 does (MTNoteBox::_HideNoteBox
	// blanks the original note in the all-notes buffer for as long as it sounds). Drawing the
	// note in BOTH passes left two coincident boxes, which z-fought once the notes started
	// writing depth again.
	"VSOUT VSMain(VSIN i) {\n"
	"  VSOUT o;\n"
	"  float apass = g_Active.w;\n"   // 'pass' is an HLSL reserved word
	"  float active = ((i.vmin.x <= g_Active.x) && (g_Active.x <= i.vmax.x)) ? 1.0 : 0.0;\n"
	// how far the now-line has passed this note's onset, in world X
	"  float since = g_Active.x - i.vmin.x;\n"
	// DX9 decays the flash/swell over a FIXED time (ActiveNoteDuration), carried here as a
	// world-X distance in g_LAmb.g. A long sustained note therefore flashes only at its
	// onset and then sits at its base colour, instead of staying half-white for its length.
	// decay <= 0 (tempo unknown / stopped): flash only at the exact onset.
	"  float decay = g_LAmb.g;\n"
	"  float prog = (decay > 0.0) ? saturate(since / decay) : ((since <= 0.0) ? 0.0 : 1.0);\n"
	"  float emph = (apass >= 0.5) ? active * (1.0 - prog) : 0.0;\n"
	// a note is drawn by exactly one pass: pass 1 if it is sounding, pass 0 if it is not
	"  float hide = saturate(i.hidden + abs(apass - active));\n"
	"  float3 c   = (i.vmin + i.vmax) * 0.5;\n"
	"  float3 ext = (i.vmax - i.vmin) * 0.5;\n"
	"  float g = 1.0 + emph * g_Active.y;\n"
	"  float3 lo = float3(i.vmin.x, c.y - ext.y*g, c.z - ext.z*g);\n"
	"  float3 hi = float3(i.vmax.x, c.y + ext.y*g, c.z + ext.z*g);\n"
	"  float3 wp = lo + (hi - lo) * i.corner * (1.0 - hide);\n"
	"  uint pbIdx = (uint)(i.color.a * 255.0 + 0.5);\n"   // port/ch index in the alpha byte
	"  float bf = (g_Opts.x >= 0.5) ? 1.0 : (active * ((apass >= 0.5) ? 1.0 : 0.0));\n"   // whole channel vs active-pass-only
	"  wp.y += bf * g_PB[pbIdx >> 2][pbIdx & 3];\n"   // pitch bend in Y
	"  o.pos = mul(float4(wp, 1.0), g_WVP);\n"
	// DX9 fixed-function directional lighting (D3DRS_LIGHTING), which the 3D box scene
	// turns on and the 2D one off. The 3D scene's two lights are exact opposites, so
	// their combined diffuse term collapses to |dot(n, L)|: faces along the light axis
	// stay bright, faces across it fall to the ambient floor - that's the 3D relief the
	// flat (unlit) port had lost. g_LAmb.w = 0 keeps the 2D scene flat, as in DX9.
	// the normal is rotated by the world matrix (the note field rolls about X), exactly
	// as DX9's fixed-function lighting does. Counter-rotating the light instead is only
	// equivalent if you get the inverse the right way round - so don't; do what DX9 does.
	"  float3 n = normalize(mul(float4(i.nrm, 0.0), g_World).xyz);\n"
	"  float3 L = normalize(g_Light.xyz);\n"
	"  float ndl = saturate(dot(n, -L)) + saturate(dot(n, L));\n"
	// DX9 fixed-function lights the note by its OWN colour: ambient and diffuse are both
	// reflectances of the vertex colour, so the term stays coloured and only clamps per
	// channel - it never washes toward white. Adding a grey ambient floor instead (the old
	// code) makes a bright face bleach to pastel. g_LAmb.r = light ambient (0.2), g_Light.w
	// = diffuse (1.2). See MTScenePianoRoll3D::_SetLightColor.
	"  float3 lit = saturate(i.color.rgb * (g_LAmb.r + g_Light.w * ndl));\n"
	"  o.col = float4(lerp(i.color.rgb, lit, g_LAmb.w), i.alpha);\n"   // carry real opacity to the PS
	"  o.emph = emph;\n"
	"  o.aflag = (apass >= 0.5) ? active : 0.0;\n"   // 1 while the note is sounding (pass 1)
	"  return o;\n"
	"}\n"
	// active notes: white-flash (emph*WhiteRate) + add the [ActiveNote] EmissiveRGB
	// (g_Opts.yzw) for as long as the note sounds (DX9 active-note emissive).
	"float4 PSMain(VSOUT i) : SV_TARGET {\n"
	"  float3 base = lerp(i.col.rgb, float3(1,1,1), saturate(i.emph * g_Active.z));\n"
	"  base += i.aflag * g_Opts.yzw;\n"
	"  return float4(saturate(base), i.col.a);\n"   // real note opacity (alpha blending)
	"}\n";


//******************************************************************************
// Constructor / destructor
//******************************************************************************
DXNoteBox11::DXNoteBox11()
{
	m_Ready = false;
	m_CollapsePorts = true;
	m_pPitchBend = NULL;
	m_BendAllNotes = false;
	m_CurTickTime = 0;
	m_SongTickPerMs = 0.0;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//DX9 MTScenePianoRoll3D light 1 (light 2 is its exact opposite, folded into the shader)
	m_LightEnable = false;
	m_LightDir = XMFLOAT3(1.0f, -1.0f, 2.0f);
	m_LightDiffuse = 1.2f;
	m_LightAmbient = 0.2f;   // DX9 light0 ambient (light1 ambient is 0); applied to the note colour
	m_pInstanceVB = NULL;
	m_AllNoteNum = 0;
	m_pNoteStartTime = NULL;
	m_pNoteMaxEndTime = NULL;
	m_pNoteTrackNo = NULL;
}

DXNoteBox11::~DXNoteBox11()
{
	Release();
}

void DXNoteBox11::Release()
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
int DXNoteBox11::InitPipeline(ID3D11Device* pDevice)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pPSBlob = NULL;
	ID3DBlob* pErr = NULL;

	if (s_pVS != NULL) return 0;  // already built
	if (pDevice == NULL) return YN_SET_ERR("Program error.", 0, 0);

	hr = D3DCompile(DXNB11_SHADER, strlen(DXNB11_SHADER), NULL, NULL, NULL, "VSMain", "vs_4_0", 0, 0, &pVSBlob, &pErr);
	if (FAILED(hr) || (pVSBlob == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }
	hr = D3DCompile(DXNB11_SHADER, strlen(DXNB11_SHADER), NULL, NULL, NULL, "PSMain", "ps_4_0", 0, 0, &pPSBlob, &pErr);
	if (FAILED(hr) || (pPSBlob == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &s_pVS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &s_pPS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	{
		// slot0 = box corner + face normal (per vertex); slot1 = per-note instance data
		D3D11_INPUT_ELEMENT_DESC il[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT,       1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32_FLOAT,       1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // real alpha
		};
		hr = pDevice->CreateInputLayout(il, 7, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &s_pLayout);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_BUFFER_DESC cb;
		ZeroMemory(&cb, sizeof(cb));
		cb.ByteWidth = sizeof(DXNB11_CONSTANTS);
		cb.Usage = D3D11_USAGE_DYNAMIC;
		cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr = pDevice->CreateBuffer(&cb, NULL, &s_pConstBuf);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		// ced 20260713: the box is now 24 vertices (4 per face) so each face can carry its
		// own normal, like the DX9 MTNoteBox vertex buffer (MTNOTEBOX_VERTEX has a normal
		// and DX9 fills the six faces with +Y/-Y/-Z/+Z/-X/+X). The old 8 shared corners
		// had nowhere to put a per-face normal, which is why the port drew the notes flat.
		// Corner masks and face grouping match the DX9 index layout exactly.
		struct TemplateVertex { float corner[3]; float normal[3]; };
		static const float corner[8][3] = {
			{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}
		};
		// Faces in DX9's MTNoteBox order (top, bottom, right, left, front, back), each wound
		// outward-clockwise so back-face culling can drop the far side of the box (see the
		// rasterizer state below). Corners: bit0 = X, bit1 = Y, bit2 = Z.
		static const int face[6][4] = {
			{2,3,7,6},   // top    (+Y)
			{0,4,5,1},   // bottom (-Y)
			{1,3,2,0},   // right  (-Z)
			{4,6,7,5},   // left   (+Z)
			{0,2,6,4},   // front  (-X)
			{1,5,7,3},   // back   (+X)
		};
		static const float faceNormal[6][3] = {
			{0,1,0}, {0,-1,0}, {0,0,-1}, {0,0,1}, {-1,0,0}, {1,0,0}
		};
		TemplateVertex verts[24];
		for (int f = 0; f < 6; f++) {
			for (int v = 0; v < 4; v++) {
				TemplateVertex* pv = &verts[f * 4 + v];
				memcpy(pv->corner, corner[face[f][v]], sizeof(pv->corner));
				memcpy(pv->normal, faceNormal[f], sizeof(pv->normal));
			}
		}
		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA sr;
		ZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(verts);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ZeroMemory(&sr, sizeof(sr));
		sr.pSysMem = verts;
		hr = pDevice->CreateBuffer(&bd, &sr, &s_pTemplateVB);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		// two triangles per face, fanned from the quad's first corner so both keep the face's
		// outward-clockwise winding
		unsigned short idx[36];
		for (int f = 0; f < 6; f++) {
			unsigned short b = (unsigned short)(f * 4);
			idx[f * 6 + 0] = b + 0; idx[f * 6 + 1] = b + 1; idx[f * 6 + 2] = b + 2;
			idx[f * 6 + 3] = b + 0; idx[f * 6 + 4] = b + 2; idx[f * 6 + 5] = b + 3;
		}
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
		// ced 20260714: both sides of the box are drawn, as in DX9 - MTScenePianoRoll3D.cpp
		// sets D3DRS_CULLMODE to D3DCULL_NONE. The z-buffer (see the depth state below) is what
		// throws the far faces away, so a note still reads as one lit face.
		rd.CullMode = D3D11_CULL_NONE;
		rd.DepthClipEnable = TRUE;
		rd.MultisampleEnable = TRUE;     // MSAA edge antialiasing
		hr = pDevice->CreateRasterizerState(&rd, &s_pRaster);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		// ced 20260627: note boxes now honour the colour's alpha (RRGGBB-AA) so notes
		// can be drawn semi-transparent. Standard src-over alpha blending. Depth write
		// stays on (no back-to-front sort), so notes blend over the background/scene;
		// overlapping notes blend in draw order (acceptable per design). Fully opaque
		// notes (alpha=FF) look identical to before, at the cost of the ROP dst read.
		D3D11_BLEND_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.RenderTarget[0].BlendEnable           = TRUE;
		bd.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&bd, &s_pBlend);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	{
		D3D11_DEPTH_STENCIL_DESC dd;
		ZeroMemory(&dd, sizeof(dd));
		dd.DepthEnable = TRUE;
		// ced 20260714: notes write depth, as in DX9 - MTScenePianoRoll3D never touches
		// D3DRS_ZWRITEENABLE, so it keeps D3D9's TRUE default. The port used to leave depth
		// writing off, which let overlapping notes resolve by draw order instead of by depth.
		// That is only harmless while every note is the same flat colour; the notes are opaque
		// (the confs use alpha FF), so without a depth write a note drawn later paints straight
		// over a nearer one that was drawn earlier, and the note field renders inside out.
		dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;   // D3D9's D3DCMP_LESSEQUAL default
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

void DXNoteBox11::ReleasePipeline()
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
int DXNoteBox11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		bool collapsePorts
	)
{
	int result = 0;
	SMNoteList* pNotes = NULL;
	const unsigned char* pTrackNo = NULL;
	D3DXVECTOR3 mv;

	(void)pContext;

	Release();
	m_CollapsePorts = collapsePorts;

	result = InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	// ced 20260713: DX9 lit the note boxes only in the 3D box scene (MTScenePianoRoll3D
	// enables the two directional lights; MTScenePianoRoll2D clears m_IsEnableLight).
	// Keep that split here - the 2D scene must stay flat.
	m_LightEnable = ((pSceneName != NULL) && (_tcsncmp(pSceneName, _T("PianoRoll3D"), 11) == 0));

	// track color mode keeps each note's source track (lost by GetMergedTrack);
	// otherwise use the cheaper merged-track path.
	if (m_NoteDesign.IsTrackColorMode()) {
		// shared, cached note list + per-note source track (built once, no per-
		// component re-pairing/sort)
		result = pSeqData->GetMergedNoteListWithTrack(&pNotes, &pTrackNo);
		if (result != 0) goto EXIT;
	}
	else {
		// shared, cached merged note list (built once across all components)
		result = pSeqData->GetMergedNoteList(&pNotes);
		if (result != 0) goto EXIT;
	}

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	result = _CreateInstanceBuffer(pDevice, pNotes, pTrackNo);
	if (result != 0) goto EXIT;

	// the GPU instance buffer + culling arrays hold everything the draw path needs
	m_NoteList.Clear();

	m_Ready = (m_AllNoteNum > 0);

EXIT:;
	return result;
}

//******************************************************************************
// Build the per-note instance buffer (+ culling arrays)
//   Authentic frame: X = time (start..end), Y = pitch, Z = channel.
//******************************************************************************
int DXNoteBox11::_CreateInstanceBuffer(
		ID3D11Device* pDevice,
		SMNoteList* pNoteList,
		const unsigned char* pTrackNo
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	unsigned long i = 0;
	unsigned long maxEnd = 0;
	DXNB11_INSTANCE* pInst = NULL;
	SMNote note;
	D3DXVECTOR3 sc, ec;
	float bh = 0.0f;
	float bw = 0.0f;

	m_AllNoteNum = pNoteList->GetSize();
	if (m_AllNoteNum == 0) goto EXIT;

	try {
		m_pNoteStartTime = new unsigned long[m_AllNoteNum];
		m_pNoteMaxEndTime = new unsigned long[m_AllNoteNum];
		pInst = new DXNB11_INSTANCE[m_AllNoteNum];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	bh = m_NoteDesign.GetNoteBoxHeight();
	bw = m_NoteDesign.GetNoteBoxWidth();

	for (i = 0; i < m_AllNoteNum; i++) {
		// loading-screen progress (throttled): keeps the bar moving on black MIDI
		if ((s_BuildProgressFunc != NULL) && ((i & 0x3FFF) == 0))
			s_BuildProgressFunc(i, m_AllNoteNum, s_BuildProgressUser);

		result = pNoteList->GetNote(i, &note);
		if (result != 0) goto EXIT;

		m_pNoteStartTime[i] = note.startTime;
		if (i == 0) maxEnd = note.endTime;
		else if (note.endTime > maxEnd) maxEnd = note.endTime;
		m_pNoteMaxEndTime[i] = maxEnd;

		// box corners (no live pitch bend for static notes). In single-keyboard
		// mode, collapse every port onto port 0's row so the notes line up with
		// the one keyboard (otherwise each port stacks into its own row in Y).
		unsigned char posPort = m_CollapsePorts ? 0 : note.portNo;
		sc = m_NoteDesign.GetNoteBoxCenterPosX(note.startTime, posPort, note.chNo, note.noteNo, 0, SM_DEFAULT_PITCHBEND_SENSITIVITY);
		ec = m_NoteDesign.GetNoteBoxCenterPosX(note.endTime,   posPort, note.chNo, note.noteNo, 0, SM_DEFAULT_PITCHBEND_SENSITIVITY);

		pInst[i].vmin[0] = sc.x;
		pInst[i].vmin[1] = sc.y - (bh / 2.0f);
		pInst[i].vmin[2] = sc.z - (bw / 2.0f);
		pInst[i].vmax[0] = ec.x;
		pInst[i].vmax[1] = sc.y + (bh / 2.0f);
		pInst[i].vmax[2] = sc.z + (bw / 2.0f);
		// rgb = note color; alpha byte carries the (port&0xF,ch) index for the
		// pitch-bend cbuffer lookup (the shader forces output alpha to 1).
		{
			unsigned long col = (pTrackNo != NULL)
				? (unsigned long)m_NoteDesign.GetTrackChannelColor(pTrackNo[i], note.chNo)
				: (unsigned long)m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
			unsigned long pbIdx = (unsigned long)(((note.portNo & 0x0F) << 4) | (note.chNo & 0x0F));
			//real note opacity is the colour's A byte; keep it separately because the
			//instance colour's A byte is repurposed as the pitch-bend cbuffer index.
			pInst[i].alpha = (float)((col >> 24) & 0xFF) / 255.0f;
			pInst[i].color = (col & 0x00FFFFFF) | (pbIdx << 24);
		}
		pInst[i].hidden  = 0.0f;
	}

	{
		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA sr;
		ZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(DXNB11_INSTANCE) * m_AllNoteNum;
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
// Visible note index range for draw culling (binary search on tick arrays)
//******************************************************************************
void DXNoteBox11::_RangeForTicks(
		unsigned long tickLow,
		unsigned long tickHigh,
		unsigned long* pLoNote,
		unsigned long* pHiNote
	)
{
	unsigned long left = 0;
	unsigned long right = 0;
	unsigned long mid = 0;
	unsigned long lo = 0;

	*pLoNote = 0;
	*pHiNote = m_AllNoteNum;

	if (m_AllNoteNum == 0) return;
	if ((m_pNoteStartTime == NULL) || (m_pNoteMaxEndTime == NULL)) return;

	// lo = first index whose prefix-max end tick >= tickLow
	left = 0; right = m_AllNoteNum;
	while (left < right) {
		mid = left + (right - left) / 2;
		if (m_pNoteMaxEndTime[mid] < tickLow) left = mid + 1;
		else right = mid;
	}
	lo = left;

	// hi = first index whose start tick > tickHigh
	left = 0; right = m_AllNoteNum;
	while (left < right) {
		mid = left + (right - left) / 2;
		if (m_pNoteStartTime[mid] <= tickHigh) left = mid + 1;
		else right = mid;
	}

	if (left < lo) left = lo;

	*pLoNote = lo;
	*pHiNote = left;
}

void DXNoteBox11::_GetVisibleNoteRange(
		unsigned long* pLoNote,
		unsigned long* pHiNote
	)
{
	unsigned long tickLow = 0;
	unsigned long tickHigh = 0;
	unsigned long halfTicks = 0;
	float xPerTick = 0.0f;

	*pLoNote = 0;
	*pHiNote = m_AllNoteNum;

	if (m_AllNoteNum == 0) return;
	if ((m_pNoteStartTime == NULL) || (m_pNoteMaxEndTime == NULL)) return;

	xPerTick = m_NoteDesign.GetPlayPosX(1 << 20) / (float)(1 << 20);
	if (xPerTick <= 0.0f) return;

	halfTicks = (unsigned long)(DXNB11_CULL_DISTANCE / xPerTick);

	tickLow = (m_CurTickTime > halfTicks) ? (m_CurTickTime - halfTicks) : 0;
	if ((0xFFFFFFFF - m_CurTickTime) < halfTicks) {
		tickHigh = 0xFFFFFFFF;
	} else {
		tickHigh = m_CurTickTime + halfTicks;
	}

	_RangeForTicks(tickLow, tickHigh, pLoNote, pHiNote);
}

//******************************************************************************
// Number of notes started by curTick (binary search on the sorted start ticks).
// Tick-based, so it does not drift when note-on messages drop under load.
//******************************************************************************
unsigned long DXNoteBox11::GetPlayedNoteCount(unsigned long curTick)
{
	unsigned long left = 0;
	unsigned long right = m_AllNoteNum;
	unsigned long mid = 0;

	if ((m_pNoteStartTime == NULL) || (m_AllNoteNum == 0)) return 0;

	// first index whose start tick > curTick == count of notes already started
	while (left < right) {
		mid = left + (right - left) / 2;
		if (m_pNoteStartTime[mid] <= curTick) left = mid + 1;
		else right = mid;
	}
	return left;
}

//******************************************************************************
// Draw the visible note range
//******************************************************************************
int DXNoteBox11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		float rollAngle
	)
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE ms;
	unsigned long loNote = 0;
	unsigned long hiNote = 0;
	ID3D11Buffer* vbs[2];
	UINT strides[2];
	UINT offsets[2] = { 0, 0 };
	float blendFactor[4] = { 0, 0, 0, 0 };

	if (!m_Ready) return 0;
	if (m_pInstanceVB == NULL) return 0;
	if (s_pVS == NULL) return YN_SET_ERR("Program error.", 0, 0);

	_GetVisibleNoteRange(&loNote, &hiNote);
	if (hiNote <= loNote) return 0;

	// world = RotX(roll) * translate(worldMove)  (matches DX9 MTNoteBox::Transform)
	DXNB11_CONSTANTS c;
	{
		XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
		               * XMMatrixTranslation(m_WorldMove.x, m_WorldMove.y, m_WorldMove.z);
		XMMATRIX wvp = world * viewProj;
		XMStoreFloat4x4(&c.wvp, wvp);
		XMStoreFloat4x4(&c.world, world);
		// active-note effect: now-line X, grow amount, white-flash rate - both from
		// conf [ActiveNote] SizeRatio / WhiteRate. grow = SizeRatio - 1 (e.g. 1.35 -> 0.35).
		// w = pass (0 = normal notes, 1 = active-note overlay) - set per draw below
		float sizeRatio = m_NoteDesign.GetActiveNoteBoxSizeRatio();
		float grow = (sizeRatio > 1.0f) ? (sizeRatio - 1.0f) : 0.0f;
		c.active = XMFLOAT4(m_NoteDesign.GetPlayPosX(m_CurTickTime), grow, m_NoteDesign.GetActiveNoteWhiteRate(), 0.0f);
		// opts.x = bend whole channel; opts.yzw = active-note emissive ([ActiveNote] EmissiveRGBA)
		D3DXCOLOR emis = m_NoteDesign.GetActiveNoteEmissive();
		c.opts = XMFLOAT4(m_BendAllNotes ? 1.0f : 0.0f, emis.r, emis.g, emis.b);
		// active-note flash/swell decay distance in world X. DX9 decays over a fixed time
		// (ActiveNoteDuration ms since onset), NOT over the note's length. Convert that time
		// to a world-X span at the current tempo: ms -> ticks (m_SongTickPerMs) -> X (xPerTick).
		// 0 when the tempo is unknown (stopped) -> the shader flashes onset-only.
		float xPerTick = m_NoteDesign.GetPlayPosX(1 << 20) / (float)(1 << 20);
		float decayX = (float)((double)m_NoteDesign.GetActiveNoteDuration() * m_SongTickPerMs) * xPerTick;
		// world-space light (the shader rotates the normals into world space, as DX9 does)
		c.light = XMFLOAT4(m_LightDir.x, m_LightDir.y, m_LightDir.z, m_LightDiffuse);
		// r = light ambient (colours the note); g = active-flash decay distance (world X);
		// w = lighting enable
		c.lamb  = XMFLOAT4(m_LightAmbient, decayX, 0.0f, m_LightEnable ? 1.0f : 0.0f);
		// per-(port&0xF,ch) pitch-bend Y shift (active notes bend in pitch)
		ZeroMemory(c.pb, sizeof(c.pb));
		if (m_pPitchBend != NULL) {
			float noteStep = m_NoteDesign.GetNoteStep();
			float* pbf = (float*)c.pb;
			for (int idx = 0; idx < 256; idx++) {
				unsigned char port = (unsigned char)(idx >> 4);
				unsigned char ch   = (unsigned char)(idx & 0x0F);
				short val = m_pPitchBend->GetValue(port, ch);
				unsigned char sens = m_pPitchBend->GetSensitivity(port, ch);
				pbf[idx] = (val < 0) ? (noteStep * sens * ((float)val / 8192.0f))
				                     : (noteStep * sens * ((float)val / 8191.0f));
			}
		}
	}

	vbs[0] = s_pTemplateVB;  strides[0] = sizeof(float) * 6;   // corner + face normal
	vbs[1] = m_pInstanceVB;  strides[1] = sizeof(DXNB11_INSTANCE);

	pContext->IASetInputLayout(s_pLayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
	pContext->IASetIndexBuffer(s_pBoxIB, DXGI_FORMAT_R16_UINT, 0);
	pContext->VSSetShader(s_pVS, NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, &s_pConstBuf);
	pContext->PSSetShader(s_pPS, NULL, 0);
	pContext->PSSetConstantBuffers(0, 1, &s_pConstBuf);   // PS reads g_Active (white-flash rate)
	pContext->RSSetState(s_pRaster);
	pContext->OMSetBlendState(s_pBlend, blendFactor, 0xFFFFFFFF);
	pContext->OMSetDepthStencilState(s_pDepth, 0);

	// Pass 1 only needs the notes whose span actually contains the now-line (the
	// active ones); the rest of the visible window is hidden by the VS anyway. So
	// draw pass 1 over just that straddling sub-range instead of the whole window.
	// For black MIDI this is a tiny fraction of the visible notes -> ~half the
	// instanced vertex work for the field, with identical output.
	unsigned long loActive = loNote;
	unsigned long hiActive = hiNote;
	_RangeForTicks(m_CurTickTime, m_CurTickTime, &loActive, &hiActive);

	// Two passes: pass 0 draws every visible note at base size, pass 1 overlays the
	// swollen/flashing active notes on top (DX9 order).
	for (int pass = 0; pass < 2; pass++) {
		unsigned long lo = (pass == 0) ? loNote : loActive;
		unsigned long hi = (pass == 0) ? hiNote : hiActive;
		if (hi <= lo) continue;
		c.active.w = (float)pass;
		hr = pContext->Map(s_pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, 0);
		memcpy(ms.pData, &c, sizeof(c));
		pContext->Unmap(s_pConstBuf, 0);
		// one instance per note in the pass range, offset into the instance buffer
		pContext->DrawIndexedInstanced(36, hi - lo, 0, 0, lo);
	}
	return 0;
}
