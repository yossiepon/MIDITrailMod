//******************************************************************************
//
// MIDITrail / DXRenderer11
//
// Direct3D 11 renderer (migration target)
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "DXScene.h"
#include "DXRenderer11.h"
#include "MTKeyboard11.h"
#include "MTFirstPersonCam.h"
#include "DXNoteBox11.h"
#include "MTNoteRipple11.h"
#include "MTNoteLyrics11.h"
#include "MTNoteBoxLive11.h"
#include "MTNoteRainLive11.h"
#include "MTGridBox11.h"
#include <d3dcompiler.h>
#include "MTDashboard11.h"
#include "MTConfigManager11.h"
#include "MTTimeIndicator11.h"
#include "MTPictBoard11.h"
#include "DXNoteRain11.h"
#include "MTKeyboardRain11.h"
#include "DXNoteBoxRing11.h"
#include "MTGridRing11.h"
#include "MTTimeIndicatorRing11.h"
#include "MTPictBoardRing11.h"
#include "MTBackgroundImage11.h"
#include "MTStars11.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <tchar.h>
#include "imgui_ja_gryph_ranges.cpp"

using namespace YNBaseLib;
using namespace DirectX;


//******************************************************************************
// Constructor
//******************************************************************************
DXRenderer11::DXRenderer11()
{
	m_hWnd = NULL;
	m_pDevice = NULL;
	m_pContext = NULL;
	m_pSwapChain = NULL;
	m_pRTV = NULL;
	m_pDepthTex = NULL;
	m_pDSV = NULL;
	m_ImContext = NULL;
	m_Width = 0;
	m_Height = 0;
	m_SampleCount = 1;
	//ced 20260628: SSAA
	m_SuperSample = 1;
	m_pSSColor = NULL;
	m_pSSRTV = NULL;
	m_pSSSRV = NULL;
	m_pSSDepth = NULL;
	m_pSSDSV = NULL;
	m_pBlitVS = NULL;
	m_pBlitPS = NULL;
	m_pBlitCB = NULL;
	m_pBlitSampler = NULL;
	m_pBlitNoDepth = NULL;
	m_pBlitRaster = NULL;
	m_TestQuadReady = false;
	m_pKbd11 = NULL;
	m_pCam11 = NULL;
	m_pNoteBox11 = NULL;
	m_pNoteRipple11 = NULL;
	m_pNoteLyrics11 = NULL;
	m_pNoteBoxLive11 = NULL;
	m_pNoteRainLive11 = NULL;
	m_pGridBox11 = NULL;
	m_pDashboard11 = NULL;
	m_pConfigMgr11 = NULL;
	m_pTimeIndicator11 = NULL;
	m_pPictBoard11 = NULL;
	m_pNoteRain11 = NULL;
	m_pKbdRain11 = NULL;
	m_pNoteBoxRing11 = NULL;
	m_pGridRing11 = NULL;
	m_pTimeIndicatorRing11 = NULL;
	m_pPictBoardRing11 = NULL;
	m_pBackgroundImage11 = NULL;
	m_pStars11 = NULL;
	m_BgColor = 0;
	m_pOffTex = NULL;
	m_pOffRTV = NULL;
	m_pOffDepth = NULL;
	m_pOffDSV = NULL;
	m_pOffStaging[0] = NULL;
	m_pOffStaging[1] = NULL;
	m_pOffTexMS = NULL;
	m_pOffRTVMS = NULL;
	m_pOffDepthMS = NULL;
	m_pOffDSVMS = NULL;
	m_OffSampleCount = 1;
	m_OffW = 0;
	m_OffH = 0;
	m_Off360 = false;
	m_CubeFaceRes = 0;
	m_pCubeTex = NULL;
	for (int i = 0; i < 6; i++) m_pCubeRTV[i] = NULL;
	m_pCubeDepth = NULL;
	m_pCubeDSV = NULL;
	m_pCubeSRV = NULL;
	m_pCubeFaceMS = NULL;
	m_pCubeFaceMSRTV = NULL;
	m_pCubeFaceMSDepth = NULL;
	m_pCubeFaceMSDSV = NULL;
	m_CubeSampleCount = 1;
	m_pEqVS = NULL;
	m_pEqPS = NULL;
	m_pEqCB = NULL;
	m_pEqSampler = NULL;
	m_pEqNoDepth = NULL;
	m_pEqRaster = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
DXRenderer11::~DXRenderer11()
{
	Terminate();
}

//******************************************************************************
// Initialize : device + swap chain + targets + ImGui
//******************************************************************************
int DXRenderer11::Initialize(
		HWND hWnd,
		unsigned long multiSampleType,
		bool isFullScreen,
		unsigned long superSample
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	RECT rc;
	UINT width = 0;
	UINT height = 0;
	DXGI_SWAP_CHAIN_DESC sd;
	D3D_FEATURE_LEVEL gotLevel = D3D_FEATURE_LEVEL_11_0;
	UINT createFlags = 0;

	(void)multiSampleType;
	(void)isFullScreen;

	//ced 20260628: SSAA 倍率（1=off, 2..4）。上限はメモリ/性能のため4倍まで。
	m_SuperSample = (superSample >= 1) ? superSample : 1;
	if (m_SuperSample > 4) m_SuperSample = 4;

	m_hWnd = hWnd;

	if (!GetClientRect(hWnd, &rc)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	width = (UINT)(rc.right - rc.left);
	height = (UINT)(rc.bottom - rc.top);
	if (width == 0) width = 1;
	if (height == 0) height = 1;

	static const D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	// desired MSAA sample count: use the configured antialias level if set,
	// otherwise default to 4x (the swap effect is DISCARD, so a multisampled
	// backbuffer auto-resolves on Present).
	{
		unsigned int wantSamples = ((multiSampleType >= 2) && (multiSampleType <= 16))
				? (unsigned int)multiSampleType : 8;   // default 8x MSAA (16x: ced 20260628)
		unsigned int trySamples[6] = { wantSamples, 16, 8, 4, 2, 1 };
		int s = 0;
		for (s = 0; s < 6; s++) {
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 2;
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = trySamples[s];
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

			hr = D3D11CreateDeviceAndSwapChain(
					NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createFlags,
					levels, (UINT)(sizeof(levels) / sizeof(levels[0])),
					D3D11_SDK_VERSION, &sd,
					&m_pSwapChain, &m_pDevice, &gotLevel, &m_pContext);
			if (FAILED(hr)) {
				//fall back to WARP software device
				hr = D3D11CreateDeviceAndSwapChain(
						NULL, D3D_DRIVER_TYPE_WARP, NULL, createFlags,
						levels, (UINT)(sizeof(levels) / sizeof(levels[0])),
						D3D11_SDK_VERSION, &sd,
						&m_pSwapChain, &m_pDevice, &gotLevel, &m_pContext);
			}
			if (SUCCEEDED(hr)) { m_SampleCount = trySamples[s]; break; }
		}
	}
	if (FAILED(hr)) {
		result = YN_SET_ERR("DirectX API error.", hr, 0);
		goto EXIT;
	}

	//ced 20260628: SSAA 用の縮小ブリットを準備（失敗時は SSAA を無効化して継続）
	if (m_SuperSample > 1) {
		if (_InitBlit() != 0) m_SuperSample = 1;
	}

	result = _CreateTargets(width, height);
	if (result != 0) goto EXIT;

	m_ImContext = CreateImContext();

	// TEMP (M2 verification): set up a test quad to confirm the DX11 geometry pipeline
	_InitTestQuad();

	// M4.3: startup / title logo (shown until a song is loaded)
	m_Logo11.Create(m_pDevice, m_pContext);

EXIT:;
	return result;
}

//******************************************************************************
// TEMP (M2 verification): build a clip-space test quad via DXPrimitive11
//******************************************************************************
int DXRenderer11::_InitTestQuad()
{
	int result = 0;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;
	int k = 0;

	result = DXPrimitive11::InitPipeline(m_pDevice);
	if (result != 0) return result;
	result = m_TestQuad.CreateVertexBuffer(m_pDevice, 4);
	if (result != 0) return result;
	result = m_TestQuad.CreateIndexBuffer(m_pDevice, 6);
	if (result != 0) return result;

	result = m_TestQuad.LockVertex(m_pContext, &pv);
	if (result != 0) return result;
	{
		float px[4] = { -0.5f,  0.5f, -0.5f, 0.5f };
		float py[4] = { -0.5f, -0.5f,  0.5f, 0.5f };
		unsigned long col[4] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00 };
		for (k = 0; k < 4; k++) {
			pv[k].pos[0] = px[k]; pv[k].pos[1] = py[k]; pv[k].pos[2] = 0.5f;
			pv[k].normal[0] = 0.0f; pv[k].normal[1] = 0.0f; pv[k].normal[2] = -1.0f;
			pv[k].color = col[k];
			pv[k].uv[0] = 0.0f; pv[k].uv[1] = 0.0f;
		}
	}
	m_TestQuad.UnlockVertex(m_pContext);

	result = m_TestQuad.LockIndex(m_pContext, &pi);
	if (result != 0) return result;
	pi[0] = 0; pi[1] = 1; pi[2] = 2;  pi[3] = 2; pi[4] = 1; pi[5] = 3;
	m_TestQuad.UnlockIndex(m_pContext);

	m_TestQuad.SetMaterialAmbient(0.9f, 0.9f, 0.9f);
	m_TestQuadReady = true;
	return 0;
}

//******************************************************************************
// Create render target view + depth buffer/view + viewport
//******************************************************************************
int DXRenderer11::_CreateTargets(
		unsigned int width,
		unsigned int height
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3D11Texture2D* pBackBuffer = NULL;
	D3D11_TEXTURE2D_DESC dd;
	D3D11_VIEWPORT vp;

	m_Width = width;
	m_Height = height;

	//render target view from the swap chain back buffer
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr) || (pBackBuffer == NULL)) {
		result = YN_SET_ERR("DirectX API error.", hr, 0);
		goto EXIT;
	}
	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRTV);
	pBackBuffer->Release();
	pBackBuffer = NULL;
	if (FAILED(hr)) {
		result = YN_SET_ERR("DirectX API error.", hr, 0);
		goto EXIT;
	}

	//depth/stencil
	ZeroMemory(&dd, sizeof(dd));
	dd.Width = width;
	dd.Height = height;
	dd.MipLevels = 1;
	dd.ArraySize = 1;
	dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dd.SampleDesc.Count = m_SampleCount;   // match the (possibly MSAA) backbuffer
	dd.SampleDesc.Quality = 0;
	dd.Usage = D3D11_USAGE_DEFAULT;
	dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = m_pDevice->CreateTexture2D(&dd, NULL, &m_pDepthTex);
	if (FAILED(hr)) {
		result = YN_SET_ERR("DirectX API error.", hr, 0);
		goto EXIT;
	}
	hr = m_pDevice->CreateDepthStencilView(m_pDepthTex, NULL, &m_pDSV);
	if (FAILED(hr)) {
		result = YN_SET_ERR("DirectX API error.", hr, 0);
		goto EXIT;
	}

	//viewport
	ZeroMemory(&vp, sizeof(vp));
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &vp);

	//ced 20260628: SSAA 用のオフスクリーン（width*ss x height*ss）。失敗したら SSAA 無効化。
	if (m_SuperSample > 1) {
		if (_CreateSSTargets(width, height) != 0) {
			_ReleaseSSTargets();
			m_SuperSample = 1;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release render targets (for resize / terminate)
//******************************************************************************
void DXRenderer11::_ReleaseTargets()
{
	if (m_pContext != NULL) m_pContext->OMSetRenderTargets(0, NULL, NULL);
	_ReleaseSSTargets();
	if (m_pDSV != NULL)      { m_pDSV->Release();      m_pDSV = NULL; }
	if (m_pDepthTex != NULL) { m_pDepthTex->Release(); m_pDepthTex = NULL; }
	if (m_pRTV != NULL)      { m_pRTV->Release();      m_pRTV = NULL; }
}

//******************************************************************************
// ced 20260628: SSAA オフスクリーン作成／破棄／縮小ブリット
//******************************************************************************
int DXRenderer11::_CreateSSTargets(unsigned int width, unsigned int height)
{
	int result = 0;
	HRESULT hr = S_OK;
	unsigned int sw = width * m_SuperSample;
	unsigned int sh = height * m_SuperSample;
	D3D11_TEXTURE2D_DESC td;
	D3D11_TEXTURE2D_DESC dd;

	if ((m_pDevice == NULL) || (sw == 0) || (sh == 0)) {
		return YN_SET_ERR("Program error.", 0, 0);
	}

	//シーン色（単一サンプル、SRV 付き）
	ZeroMemory(&td, sizeof(td));
	td.Width = sw; td.Height = sh; td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1; td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	hr = m_pDevice->CreateTexture2D(&td, NULL, &m_pSSColor);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = m_pDevice->CreateRenderTargetView(m_pSSColor, NULL, &m_pSSRTV);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = m_pDevice->CreateShaderResourceView(m_pSSColor, NULL, &m_pSSSRV);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	//深度
	ZeroMemory(&dd, sizeof(dd));
	dd.Width = sw; dd.Height = sh; dd.MipLevels = 1; dd.ArraySize = 1;
	dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dd.SampleDesc.Count = 1; dd.SampleDesc.Quality = 0;
	dd.Usage = D3D11_USAGE_DEFAULT;
	dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = m_pDevice->CreateTexture2D(&dd, NULL, &m_pSSDepth);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = m_pDevice->CreateDepthStencilView(m_pSSDepth, NULL, &m_pSSDSV);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

EXIT:;
	return result;
}

void DXRenderer11::_ReleaseSSTargets()
{
	if (m_pSSDSV != NULL)   { m_pSSDSV->Release();   m_pSSDSV = NULL; }
	if (m_pSSDepth != NULL) { m_pSSDepth->Release(); m_pSSDepth = NULL; }
	if (m_pSSSRV != NULL)   { m_pSSSRV->Release();   m_pSSSRV = NULL; }
	if (m_pSSRTV != NULL)   { m_pSSRTV->Release();   m_pSSRTV = NULL; }
	if (m_pSSColor != NULL) { m_pSSColor->Release(); m_pSSColor = NULL; }
}

void DXRenderer11::_BlitSSToBackbuffer()
{
	D3D11_VIEWPORT vp;
	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	UINT zero = 0;
	ID3D11Buffer* noVB[1] = { NULL };
	float bf[4] = { 0, 0, 0, 0 };

	if ((m_pBlitVS == NULL) || (m_pSSSRV == NULL)) return;

	//cbuffer 更新：SS テクセルサイズと倍率（box フィルタ用）
	if (m_pBlitCB != NULL) {
		D3D11_MAPPED_SUBRESOURCE ms;
		unsigned int sw = m_Width * m_SuperSample;
		unsigned int sh = m_Height * m_SuperSample;
		if (SUCCEEDED(m_pContext->Map(m_pBlitCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
			float* p = (float*)ms.pData;
			p[0] = (sw > 0) ? (1.0f / (float)sw) : 0.0f;   // texelSize.x
			p[1] = (sh > 0) ? (1.0f / (float)sh) : 0.0f;   // texelSize.y
			p[2] = (float)m_SuperSample;                   // factor
			p[3] = 0.0f;                                   // pad
			m_pContext->Unmap(m_pBlitCB, 0);
		}
	}

	//バックバッファへ等倍ビューポートで描画
	ZeroMemory(&vp, sizeof(vp));
	vp.Width = (float)m_Width; vp.Height = (float)m_Height;
	vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &vp);
	m_pContext->OMSetRenderTargets(1, &m_pRTV, NULL);

	m_pContext->IASetInputLayout(NULL);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->IASetVertexBuffers(0, 1, noVB, &zero, &zero);
	m_pContext->VSSetShader(m_pBlitVS, NULL, 0);
	m_pContext->PSSetShader(m_pBlitPS, NULL, 0);
	m_pContext->PSSetShaderResources(0, 1, &m_pSSSRV);
	m_pContext->PSSetConstantBuffers(0, 1, &m_pBlitCB);
	m_pContext->PSSetSamplers(0, 1, &m_pBlitSampler);
	m_pContext->OMSetDepthStencilState(m_pBlitNoDepth, 0);
	m_pContext->OMSetBlendState(NULL, bf, 0xFFFFFFFF);
	m_pContext->RSSetState(m_pBlitRaster);
	m_pContext->Draw(3, 0);

	//SRV を解放（次フレームでレンダーターゲットとして使うため）
	m_pContext->PSSetShaderResources(0, 1, nullSRV);
}

//******************************************************************************
// ImGui context
//******************************************************************************
ImGuiContext* DXRenderer11::CreateImContext()
{
	IMGUI_CHECKVERSION();
	ImGuiContext* ImContext = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImFontConfig font_config = {};
	font_config.FontNo = 0; // Tahoma
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 14.0f, &font_config, io.Fonts->GetGlyphRangesDefault());
	font_config.FontNo = 1; // MS Gothic
	font_config.MergeMode = true;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msgothic.ttc", 14.0f, &font_config, glyphRangesJapanese);
	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	return ImContext;
}

//******************************************************************************
// Accessors
//******************************************************************************
ID3D11Device* DXRenderer11::GetDevice()
{
	return m_pDevice;
}

ID3D11DeviceContext* DXRenderer11::GetContext()
{
	return m_pContext;
}

//******************************************************************************
// MSAA support query for the graphics config dialog
//******************************************************************************
bool DXRenderer11::IsMultiSampleSupported(unsigned int sampleCount)
{
	if (m_pDevice == NULL) return false;
	if (sampleCount <= 1) return true;   // "off" is always available
	UINT quality = 0;
	HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampleCount, &quality);
	return SUCCEEDED(hr) && (quality > 0);
}

//******************************************************************************
// Begin an ImGui frame
//******************************************************************************
void DXRenderer11::_BeginImGuiFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

//******************************************************************************
// Render one frame (M1: clear + ImGui; scene geometry ported in M2+)
//******************************************************************************
int DXRenderer11::RenderScene(
		DXScene* pScene
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//ced 20260628: SSAA 用（goto をまたぐ初期化を避けるため先頭で宣言）
	bool ss = false;
	unsigned int renderW = 0;
	unsigned int renderH = 0;
	ID3D11RenderTargetView* sceneRTV = NULL;
	ID3D11DepthStencilView* sceneDSV = NULL;

	if (m_pContext == NULL) goto EXIT;

	//M5: background clear color from conf [Color] BackGroundRGB (the DX9 scene's
	//SetBGColor path is dead now that m_pScene is NULL). D3DCOLOR is 0xAARRGGBB;
	//alpha is forced opaque for the backbuffer clear.
	{
		D3DCOLOR bg = m_BgColor;
		clearColor[0] = (float)((bg >> 16) & 0xFF) / 255.0f;
		clearColor[1] = (float)((bg >> 8) & 0xFF) / 255.0f;
		clearColor[2] = (float)(bg & 0xFF) / 255.0f;
		clearColor[3] = 1.0f;
	}

	_BeginImGuiFrame();

	//ced 20260628: SSAA 有効時はシーンを SS オフスクリーン（高解像度）へ描画し、
	//後段でバックバッファへ線形縮小する。無効時は従来どおりバックバッファへ直接描画。
	ss = (m_SuperSample > 1) && (m_pSSRTV != NULL) && (m_pSSDSV != NULL) && (m_pBlitVS != NULL);
	renderW = ss ? (m_Width * m_SuperSample) : m_Width;
	renderH = ss ? (m_Height * m_SuperSample) : m_Height;
	sceneRTV = ss ? m_pSSRTV : m_pRTV;
	sceneDSV = ss ? m_pSSDSV : m_pDSV;
	if (ss) {
		D3D11_VIEWPORT vp; ZeroMemory(&vp, sizeof(vp));
		vp.Width = (float)renderW; vp.Height = (float)renderH;
		vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
		m_pContext->RSSetViewports(1, &vp);
	}

	m_pContext->OMSetRenderTargets(1, &sceneRTV, sceneDSV);
	m_pContext->ClearRenderTargetView(sceneRTV, clearColor);
	m_pContext->ClearDepthStencilView(sceneDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// M4.15: background image, behind everything (screen space, far-plane z)
	if ((m_pBackgroundImage11 != NULL) && m_pBackgroundImage11->IsReady()) {
		m_pBackgroundImage11->DrawDX11(m_pContext, renderW, renderH);
	}

	// M2/M3: draw the scene (note field + keyboard) via the real camera
	bool hasKbd   = (m_pKbd11 != NULL) && m_pKbd11->IsReady();
	bool hasNotes = (m_pNoteBox11 != NULL) && m_pNoteBox11->IsReady();
	bool hasRain  = (m_pNoteRain11 != NULL) && m_pNoteRain11->IsReady();
	bool hasRainKbd = (m_pKbdRain11 != NULL) && m_pKbdRain11->IsReady();
	bool hasRing  = (m_pNoteBoxRing11 != NULL) && m_pNoteBoxRing11->IsReady();
	bool hasLive  = (m_pNoteBoxLive11 != NULL) && m_pNoteBoxLive11->IsReady();
	bool hasRainLive = (m_pNoteRainLive11 != NULL) && m_pNoteRainLive11->IsReady();
	if (hasKbd || hasNotes || hasRain || hasRainKbd || hasRing || hasLive || hasRainLive) {
		float aspect = (m_Height > 0) ? ((float)m_Width / (float)m_Height) : 1.0f;
		XMMATRIX view = XMMatrixIdentity();
		XMMATRIX proj = XMMatrixIdentity();
		if (m_pCam11 != NULL) {
			// real first-person camera: poll input, then read its matrices
			D3DXMATRIX d3dView, d3dProj;
			m_pCam11->TransformDX11();
			m_pCam11->GetMatrices(aspect, &d3dView, &d3dProj);
			// D3DXMATRIX and XMMATRIX are both row-major 4x4 float
			view = XMLoadFloat4x4((const XMFLOAT4X4*)&d3dView);
			proj = XMLoadFloat4x4((const XMFLOAT4X4*)&d3dProj);
		}
		XMMATRIX viewProj = view * proj;
		// scene roll (manual wheel + auto), applied to the object world matrices
		float rollAngle = (m_pCam11 != NULL) ? m_pCam11->GetManualRollAngle() : 0.0f;
		// camera position (used by the starfield and the ripples)
		XMFLOAT3 camPos(0.0f, 0.0f, 0.0f);
		if (m_pCam11 != NULL) {
			D3DXVECTOR3 p;
			m_pCam11->GetPosition(&p);
			camPos = XMFLOAT3(p.x, p.y, p.z);
		}
		// starfield: behind the scene, translated to the camera (infinite sky)
		if ((m_pStars11 != NULL) && m_pStars11->IsReady()) {
			m_pStars11->DrawDX11(m_pContext, viewProj, camPos);
		}
		// grid lines first (behind the notes), as a spatial reference
		if ((m_pGridBox11 != NULL) && m_pGridBox11->IsReady()) {
			XMFLOAT4 gridLight(0.3f, -0.6f, 0.5f, 0.0f);
			m_pGridBox11->DrawDX11(m_pContext, viewProj, gridLight, rollAngle);
		}
		// picture board (textured billboard in the playback section, behind notes)
		if ((m_pPictBoard11 != NULL) && m_pPictBoard11->IsReady()) {
			XMFLOAT4 pbLight(0.0f, 0.0f, 1.0f, 0.0f);
			m_pPictBoard11->DrawDX11(m_pContext, viewProj, pbLight, rollAngle);
		}
		// Box scene: keyboard first (writes depth), then the note field. Notes
		// test depth but don't write it (M3.14 anti-flicker), so a note in front
		// of the keyboard surface shows over it and a note behind is occluded -
		// matching the DX9 z-buffer result. Drawing notes first made the later
		// keyboard always cover them ("notes always behind", worst in flat 2D).
		if (hasKbd) {
			XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
			m_pKbd11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle, camPos);
		}
		if (hasNotes) {
			m_pNoteBox11->DrawDX11(m_pContext, viewProj, rollAngle);
		}
		// live monitor: dynamic note boxes (real-time MIDI input); ring live reuses
		// this same renderer slot (the live object is built in ring mode)
		if (hasLive) {
			// DX9 MTScenePianoRoll3DLive light direction (the note boxes carry the DX9
			// per-face normals, so this is what gives them their 3D relief)
			XMFLOAT4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);
			m_pNoteBoxLive11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle);
		}
		// Rain scene: keyboard first (writes depth), then the falling notes, so
		// notes fallen behind the keyboard are occluded (matches DX9 draw order).
		if (hasRainKbd) {
			XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
			m_pKbdRain11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle);
		}
		if (hasRain) {
			m_pNoteRain11->DrawDX11(m_pContext, viewProj, rollAngle);
		}
		// Rain live monitor: dynamic falling notes (real-time MIDI input)
		if ((m_pNoteRainLive11 != NULL) && m_pNoteRainLive11->IsReady()) {
			XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
			m_pNoteRainLive11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle);
		}
		// Ring scene: grid + board (behind), circular notes, then the indicator
		if ((m_pGridRing11 != NULL) && m_pGridRing11->IsReady()) {
			XMFLOAT4 grLight(0.3f, -0.6f, 0.5f, 0.0f);
			m_pGridRing11->DrawDX11(m_pContext, viewProj, grLight, rollAngle);
		}
		if ((m_pPictBoardRing11 != NULL) && m_pPictBoardRing11->IsReady()) {
			XMFLOAT4 pbLight(0.0f, 0.0f, 1.0f, 0.0f);
			m_pPictBoardRing11->DrawDX11(m_pContext, viewProj, pbLight, rollAngle);
		}
		if (hasRing) {
			m_pNoteBoxRing11->DrawDX11(m_pContext, viewProj, rollAngle);
		}
		if ((m_pTimeIndicatorRing11 != NULL) && m_pTimeIndicatorRing11->IsReady()) {
			XMFLOAT4 tiLight(0.0f, 0.0f, 1.0f, 0.0f);
			m_pTimeIndicatorRing11->DrawDX11(m_pContext, viewProj, tiLight, rollAngle);
		}
		// time indicator (translucent playback section, drawn over the notes)
		if ((m_pTimeIndicator11 != NULL) && m_pTimeIndicator11->IsReady()) {
			XMFLOAT4 tiLight(0.0f, 0.0f, 1.0f, 0.0f);
			m_pTimeIndicator11->DrawDX11(m_pContext, viewProj, tiLight, rollAngle);
		}
		// note ripples (camera-facing, needs the camera position)
		if ((m_pNoteRipple11 != NULL) && m_pNoteRipple11->IsReady()) {
			XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
			m_pNoteRipple11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle, camPos);
		}
		// note lyrics (text quads over the played notes, camera-facing)
		if ((m_pNoteLyrics11 != NULL) && m_pNoteLyrics11->IsReady()) {
			XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
			m_pNoteLyrics11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle, camPos);
		}
	}
	else if (m_Logo11.IsReady()) {
		// M4.3: startup / title screen (no song loaded) - the MIDITrail logo
		float aspect = (m_Height > 0) ? ((float)m_Width / (float)m_Height) : 1.0f;
		m_Logo11.DrawDX11(m_pContext, aspect);
	}

	//ced 20260628: SSAA 有効時はここで SS オフスクリーンをバックバッファへ線形縮小。
	//ダッシュボード（オーバーレイ文字）と ImGui は縮小の“後”にバックバッファ等倍で描く。
	//固定ピクセル配置のダッシュボードを SS 解像度で描くと NDC がずれて位置/サイズが
	//崩れる（=消えて見える）ため、ネイティブ解像度で描くのが正しく綺麗。
	if (ss) {
		_BlitSSToBackbuffer();
	}

	// M4: dashboard overlay (on-screen info text), drawn over the scene at native res
	if ((m_pDashboard11 != NULL) && m_pDashboard11->IsReady()) {
		m_pDashboard11->DrawDX11(m_pContext, m_Width, m_Height);
	}

	// Mod Mod: Config Manager (ImGui) - GUI editor for conf/*.ini, toggled from
	// Options -> Config Manager. Drawn as an interactive ImGui window over the scene.
	if (m_pConfigMgr11 != NULL) {
		m_pConfigMgr11->RenderImGui();
	}

	// ImGui draws no other overlay during normal frames; it stays initialized for
	// the loading screen too. File open is via the Win32 menu / drag-drop.

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	hr = m_pSwapChain->Present(1, 0);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		result = DXRENDERER11_ERR_DEVICE_LOST;
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// M6 (video export): off-screen render at an arbitrary resolution
//******************************************************************************
int DXRenderer11::BeginOffscreen(int width, int height, bool equirect360)
{
	HRESULT hr = S_OK;
	int result = 0;

	EndOffscreen();
	if ((m_pDevice == NULL) || (width <= 0) || (height <= 0)) {
		return YN_SET_ERR("Program error.", width, height);
	}
	m_OffW = (unsigned int)width;
	m_OffH = (unsigned int)height;
	m_Off360 = equirect360;

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = m_OffW; td.Height = m_OffH; td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.SampleDesc.Count = 1; td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT; td.BindFlags = D3D11_BIND_RENDER_TARGET;

	hr = m_pDevice->CreateTexture2D(&td, NULL, &m_pOffTex);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }
	hr = m_pDevice->CreateRenderTargetView(m_pOffTex, NULL, &m_pOffRTV);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }

	{
		D3D11_TEXTURE2D_DESC dd = td;
		dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		hr = m_pDevice->CreateTexture2D(&dd, NULL, &m_pOffDepth);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }
		hr = m_pDevice->CreateDepthStencilView(m_pOffDepth, NULL, &m_pOffDSV);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }
	}

	// MSAA render target for the export (resolved into m_pOffTex before readback)
	// so the exported video is antialiased like the live view. Try 8x/4x/2x and
	// TOLERATE allocation failure (a 2x-supersampled export may not fit 8x): fall
	// back to fewer samples, or to no-MSAA (the supersample downscale still AAs).
	m_OffSampleCount = 1;
	if (!equirect360)
	{
		unsigned int tryC[3] = { 8, 4, 2 };
		for (int i = 0; i < 3; i++) {
			UINT q = 0;
			if (FAILED(m_pDevice->CheckMultisampleQualityLevels(td.Format, tryC[i], &q)) || (q == 0)) continue;

			D3D11_TEXTURE2D_DESC md = td;
			md.SampleDesc.Count = tryC[i];
			md.SampleDesc.Quality = 0;
			D3D11_TEXTURE2D_DESC mdd = md;
			mdd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			mdd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			bool good =
				SUCCEEDED(m_pDevice->CreateTexture2D(&md, NULL, &m_pOffTexMS)) &&
				SUCCEEDED(m_pDevice->CreateRenderTargetView(m_pOffTexMS, NULL, &m_pOffRTVMS)) &&
				SUCCEEDED(m_pDevice->CreateTexture2D(&mdd, NULL, &m_pOffDepthMS)) &&
				SUCCEEDED(m_pDevice->CreateDepthStencilView(m_pOffDepthMS, NULL, &m_pOffDSVMS));
			if (good) { m_OffSampleCount = tryC[i]; break; }

			// release any partial objects and try a lower sample count
			if (m_pOffDSVMS)   { m_pOffDSVMS->Release();   m_pOffDSVMS = NULL; }
			if (m_pOffDepthMS) { m_pOffDepthMS->Release(); m_pOffDepthMS = NULL; }
			if (m_pOffRTVMS)   { m_pOffRTVMS->Release();   m_pOffRTVMS = NULL; }
			if (m_pOffTexMS)   { m_pOffTexMS->Release();   m_pOffTexMS = NULL; }
		}
	}

	{
		D3D11_TEXTURE2D_DESC sd = td;
		sd.Usage = D3D11_USAGE_STAGING;
		sd.BindFlags = 0;
		sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		for (int i = 0; i < 2; i++) {
			hr = m_pDevice->CreateTexture2D(&sd, NULL, &m_pOffStaging[i]);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }
		}
	}

	// 360: cubemap render target (6 faces) + a shared face depth + the equirect pass
	if (equirect360) {
		// face resolution ~ the equirect height (capped) gives enough detail per face
		unsigned int fr = m_OffH; if (fr > 2048) fr = 2048; if (fr < 256) fr = 256;
		m_CubeFaceRes = fr;

		D3D11_TEXTURE2D_DESC cd;
		ZeroMemory(&cd, sizeof(cd));
		cd.Width = fr; cd.Height = fr; cd.MipLevels = 1; cd.ArraySize = 6;
		cd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		cd.SampleDesc.Count = 1;
		cd.Usage = D3D11_USAGE_DEFAULT;
		cd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		cd.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		hr = m_pDevice->CreateTexture2D(&cd, NULL, &m_pCubeTex);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }

		for (int f = 0; f < 6; f++) {
			D3D11_RENDER_TARGET_VIEW_DESC rv;
			ZeroMemory(&rv, sizeof(rv));
			rv.Format = cd.Format;
			rv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rv.Texture2DArray.MipSlice = 0;
			rv.Texture2DArray.FirstArraySlice = f;
			rv.Texture2DArray.ArraySize = 1;
			hr = m_pDevice->CreateRenderTargetView(m_pCubeTex, &rv, &m_pCubeRTV[f]);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC sv;
		ZeroMemory(&sv, sizeof(sv));
		sv.Format = cd.Format;
		sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		sv.TextureCube.MipLevels = 1;
		hr = m_pDevice->CreateShaderResourceView(m_pCubeTex, &sv, &m_pCubeSRV);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }

		{
			D3D11_TEXTURE2D_DESC dd;
			ZeroMemory(&dd, sizeof(dd));
			dd.Width = fr; dd.Height = fr; dd.MipLevels = 1; dd.ArraySize = 1;
			dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dd.SampleDesc.Count = 1;
			dd.Usage = D3D11_USAGE_DEFAULT;
			dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			hr = m_pDevice->CreateTexture2D(&dd, NULL, &m_pCubeDepth);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }
			hr = m_pDevice->CreateDepthStencilView(m_pCubeDepth, NULL, &m_pCubeDSV);
			if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto FAIL; }
		}

		// per-face MSAA render target (each face is rendered here then resolved into
		// the cube face) so the panorama is antialiased. Try 8x/4x/2x; tolerate failure.
		m_CubeSampleCount = 1;
		{
			unsigned int tryC[3] = { 8, 4, 2 };
			for (int i = 0; i < 3; i++) {
				UINT q = 0;
				if (FAILED(m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, tryC[i], &q)) || (q == 0)) continue;
				D3D11_TEXTURE2D_DESC md;
				ZeroMemory(&md, sizeof(md));
				md.Width = fr; md.Height = fr; md.MipLevels = 1; md.ArraySize = 1;
				md.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				md.SampleDesc.Count = tryC[i];
				md.Usage = D3D11_USAGE_DEFAULT; md.BindFlags = D3D11_BIND_RENDER_TARGET;
				D3D11_TEXTURE2D_DESC mdd = md;
				mdd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; mdd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				bool good =
					SUCCEEDED(m_pDevice->CreateTexture2D(&md, NULL, &m_pCubeFaceMS)) &&
					SUCCEEDED(m_pDevice->CreateRenderTargetView(m_pCubeFaceMS, NULL, &m_pCubeFaceMSRTV)) &&
					SUCCEEDED(m_pDevice->CreateTexture2D(&mdd, NULL, &m_pCubeFaceMSDepth)) &&
					SUCCEEDED(m_pDevice->CreateDepthStencilView(m_pCubeFaceMSDepth, NULL, &m_pCubeFaceMSDSV));
				if (good) { m_CubeSampleCount = tryC[i]; break; }
				if (m_pCubeFaceMSDSV)   { m_pCubeFaceMSDSV->Release();   m_pCubeFaceMSDSV = NULL; }
				if (m_pCubeFaceMSDepth) { m_pCubeFaceMSDepth->Release(); m_pCubeFaceMSDepth = NULL; }
				if (m_pCubeFaceMSRTV)   { m_pCubeFaceMSRTV->Release();   m_pCubeFaceMSRTV = NULL; }
				if (m_pCubeFaceMS)      { m_pCubeFaceMS->Release();      m_pCubeFaceMS = NULL; }
			}
		}

		result = _InitEquirect();
		if (result != 0) goto FAIL;
	}

	return 0;

FAIL:;
	EndOffscreen();
	return result;
}

int DXRenderer11::RenderOffscreenFrame(bool transparent, int stageSlot)
{
	if ((m_pContext == NULL) || (m_pOffRTV == NULL)) return YN_SET_ERR("Program error.", 0, 0);
	if (stageSlot < 0 || stageSlot > 1) return YN_SET_ERR("Program error.", stageSlot, 0);

	if (m_Off360) return _RenderFrame360(transparent, stageSlot);

	float clr[4];
	if (transparent) {
		clr[0] = clr[1] = clr[2] = 0.0f; clr[3] = 0.0f;
	} else {
		D3DCOLOR bg = m_BgColor;
		clr[0] = (float)((bg >> 16) & 0xFF) / 255.0f;
		clr[1] = (float)((bg >> 8) & 0xFF) / 255.0f;
		clr[2] = (float)(bg & 0xFF) / 255.0f;
		clr[3] = 1.0f;
	}

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0; vp.TopLeftY = 0;
	vp.Width = (float)m_OffW; vp.Height = (float)m_OffH;
	vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &vp);
	// render into the MSAA target when available (resolved into m_pOffTex below)
	ID3D11RenderTargetView* pRTV = (m_OffSampleCount > 1) ? m_pOffRTVMS : m_pOffRTV;
	ID3D11DepthStencilView* pDSV = (m_OffSampleCount > 1) ? m_pOffDSVMS : m_pOffDSV;
	m_pContext->OMSetRenderTargets(1, &pRTV, pDSV);
	m_pContext->ClearRenderTargetView(pRTV, clr);
	m_pContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// background image (skipped for transparent output so the alpha stays clear)
	if (!transparent && (m_pBackgroundImage11 != NULL) && m_pBackgroundImage11->IsReady()) {
		m_pBackgroundImage11->DrawDX11(m_pContext, m_OffW, m_OffH);
	}

	// scene: follow the playback scroll (no input poll), then draw every component
	{
		float aspect = (m_OffH > 0) ? ((float)m_OffW / (float)m_OffH) : 1.0f;
		XMMATRIX view = XMMatrixIdentity(), proj = XMMatrixIdentity();
		float rollAngle = 0.0f;
		XMFLOAT3 camPos(0.0f, 0.0f, 0.0f);
		if (m_pCam11 != NULL) {
			m_pCam11->UpdateScrollForExport();
			D3DXMATRIX d3dView, d3dProj;
			m_pCam11->GetMatrices(aspect, &d3dView, &d3dProj);
			view = XMLoadFloat4x4((const XMFLOAT4X4*)&d3dView);
			proj = XMLoadFloat4x4((const XMFLOAT4X4*)&d3dProj);
			rollAngle = m_pCam11->GetManualRollAngle();
			D3DXVECTOR3 p; m_pCam11->GetPosition(&p);
			camPos = XMFLOAT3(p.x, p.y, p.z);
		}
		_DrawSceneComponents(view * proj, camPos, rollAngle, transparent);
	}

	// dashboard overlay (file name + counter), same as the live frame
	if ((m_pDashboard11 != NULL) && m_pDashboard11->IsReady()) {
		m_pDashboard11->DrawDX11(m_pContext, m_OffW, m_OffH);
	}

	// resolve the MSAA target into the single-sample resolve texture (antialias),
	// then queue a GPU copy to this frame's staging texture (async; read next frame)
	if (m_OffSampleCount > 1) {
		m_pContext->ResolveSubresource(m_pOffTex, 0, m_pOffTexMS, 0, DXGI_FORMAT_B8G8R8A8_UNORM);
	}
	m_pContext->CopyResource(m_pOffStaging[stageSlot], m_pOffTex);
	return 0;
}

//******************************************************************************
// draw every active scene component with the given camera (shared by the normal
// offscreen frame and each 360 cube face)
//******************************************************************************
void DXRenderer11::_DrawSceneComponents(const XMMATRIX& viewProj, const XMFLOAT3& camPos,
		float rollAngle, bool transparent)
{
	(void)transparent;
	bool hasKbd   = (m_pKbd11 != NULL) && m_pKbd11->IsReady();
	bool hasNotes = (m_pNoteBox11 != NULL) && m_pNoteBox11->IsReady();
	bool hasRain  = (m_pNoteRain11 != NULL) && m_pNoteRain11->IsReady();
	bool hasRainKbd = (m_pKbdRain11 != NULL) && m_pKbdRain11->IsReady();
	bool hasRing  = (m_pNoteBoxRing11 != NULL) && m_pNoteBoxRing11->IsReady();
	bool hasLive  = (m_pNoteBoxLive11 != NULL) && m_pNoteBoxLive11->IsReady();
	bool hasRainLive = (m_pNoteRainLive11 != NULL) && m_pNoteRainLive11->IsReady();
	if (!(hasKbd || hasNotes || hasRain || hasRainKbd || hasRing || hasLive || hasRainLive)) return;

	if ((m_pStars11 != NULL) && m_pStars11->IsReady())
		m_pStars11->DrawDX11(m_pContext, viewProj, camPos);
	if ((m_pGridBox11 != NULL) && m_pGridBox11->IsReady()) {
		XMFLOAT4 gridLight(0.3f, -0.6f, 0.5f, 0.0f);
		m_pGridBox11->DrawDX11(m_pContext, viewProj, gridLight, rollAngle);
	}
	if ((m_pPictBoard11 != NULL) && m_pPictBoard11->IsReady()) {
		XMFLOAT4 pbLight(0.0f, 0.0f, 1.0f, 0.0f);
		m_pPictBoard11->DrawDX11(m_pContext, viewProj, pbLight, rollAngle);
	}
	if (hasKbd) {
		XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
		m_pKbd11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle, camPos);
	}
	if (hasNotes) m_pNoteBox11->DrawDX11(m_pContext, viewProj, rollAngle);
	if (hasLive) {
		// DX9 MTScenePianoRoll3DLive light direction (see the main render path)
		XMFLOAT4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);
		m_pNoteBoxLive11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle);
	}
	if (hasRainKbd) {
		XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
		m_pKbdRain11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle);
	}
	if (hasRain) m_pNoteRain11->DrawDX11(m_pContext, viewProj, rollAngle);
	if (hasRainLive) {
		XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
		m_pNoteRainLive11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle);
	}
	if ((m_pGridRing11 != NULL) && m_pGridRing11->IsReady()) {
		XMFLOAT4 grLight(0.3f, -0.6f, 0.5f, 0.0f);
		m_pGridRing11->DrawDX11(m_pContext, viewProj, grLight, rollAngle);
	}
	if ((m_pPictBoardRing11 != NULL) && m_pPictBoardRing11->IsReady()) {
		XMFLOAT4 pbLight(0.0f, 0.0f, 1.0f, 0.0f);
		m_pPictBoardRing11->DrawDX11(m_pContext, viewProj, pbLight, rollAngle);
	}
	if (hasRing) m_pNoteBoxRing11->DrawDX11(m_pContext, viewProj, rollAngle);
	if ((m_pTimeIndicatorRing11 != NULL) && m_pTimeIndicatorRing11->IsReady()) {
		XMFLOAT4 tiLight(0.0f, 0.0f, 1.0f, 0.0f);
		m_pTimeIndicatorRing11->DrawDX11(m_pContext, viewProj, tiLight, rollAngle);
	}
	if ((m_pTimeIndicator11 != NULL) && m_pTimeIndicator11->IsReady()) {
		XMFLOAT4 tiLight(0.0f, 0.0f, 1.0f, 0.0f);
		m_pTimeIndicator11->DrawDX11(m_pContext, viewProj, tiLight, rollAngle);
	}
	if ((m_pNoteRipple11 != NULL) && m_pNoteRipple11->IsReady()) {
		XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
		m_pNoteRipple11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle, camPos);
	}
	if ((m_pNoteLyrics11 != NULL) && m_pNoteLyrics11->IsReady()) {
		XMFLOAT4 lightDir(0.3f, -0.6f, 0.5f, 0.0f);
		m_pNoteLyrics11->DrawDX11(m_pContext, viewProj, lightDir, rollAngle, camPos);
	}
}

//******************************************************************************
// equirectangular pipeline (fullscreen triangle that samples the scene cubemap)
//******************************************************************************
//******************************************************************************
// ced 20260628: SSAA 縮小ブリット（フルスクリーン三角形で SS テクスチャを線形サンプル）
//******************************************************************************
static const char* DXR11_BLIT_SHADER =
	"Texture2D g_Tex : register(t0);\n"
	"SamplerState g_Samp : register(s0);\n"
	"cbuffer BlitCB : register(b0) { float2 g_TexelSize; float g_Factor; float g_Pad; };\n"
	"struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };\n"
	"VSOut VSMain(uint id : SV_VertexID) {\n"
	"  VSOut o; float2 t = float2((id << 1) & 2, id & 2);\n"
	"  o.pos = float4(t.x * 2.0 - 1.0, 1.0 - t.y * 2.0, 0.0, 1.0);\n"
	"  o.uv = t; return o;\n"
	"}\n"
	// box downsample: average the f x f source texels that map to this output pixel.
	// a single bilinear tap only blends 2x2, so 3x/4x SSAA would still alias.
	"float4 PSMain(VSOut i) : SV_TARGET {\n"
	"  int f = (int)(g_Factor + 0.5);\n"
	"  if (f < 1) f = 1;\n"
	"  float4 sum = float4(0,0,0,0);\n"
	"  [loop] for (int y = 0; y < f; y++) {\n"
	"    [loop] for (int x = 0; x < f; x++) {\n"
	"      float2 off = float2((x + 0.5 - f * 0.5) * g_TexelSize.x,\n"
	"                          (y + 0.5 - f * 0.5) * g_TexelSize.y);\n"
	"      sum += g_Tex.SampleLevel(g_Samp, i.uv + off, 0.0);\n"
	"    }\n"
	"  }\n"
	"  return sum / (float)(f * f);\n"
	"}\n";

int DXRenderer11::_InitBlit()
{
	if (m_pBlitVS != NULL) return 0;   // already built
	if (m_pDevice == NULL) return YN_SET_ERR("Program error.", 0, 0);

	HRESULT hr = S_OK;
	int result = 0;
	ID3DBlob* pVS = NULL; ID3DBlob* pPS = NULL; ID3DBlob* pErr = NULL;

	hr = D3DCompile(DXR11_BLIT_SHADER, strlen(DXR11_BLIT_SHADER), NULL, NULL, NULL, "VSMain", "vs_4_0", 0, 0, &pVS, &pErr);
	if (FAILED(hr) || (pVS == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }
	hr = D3DCompile(DXR11_BLIT_SHADER, strlen(DXR11_BLIT_SHADER), NULL, NULL, NULL, "PSMain", "ps_4_0", 0, 0, &pPS, &pErr);
	if (FAILED(hr) || (pPS == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }

	hr = m_pDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), NULL, &m_pBlitVS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = m_pDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), NULL, &m_pBlitPS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	{
		D3D11_BUFFER_DESC cb; ZeroMemory(&cb, sizeof(cb));
		cb.ByteWidth = sizeof(float) * 4;   // texelSize.xy + factor + pad
		cb.Usage = D3D11_USAGE_DYNAMIC;
		cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER; cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr = m_pDevice->CreateBuffer(&cb, NULL, &m_pBlitCB);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}
	{
		D3D11_SAMPLER_DESC sd; ZeroMemory(&sd, sizeof(sd));
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.MaxLOD = D3D11_FLOAT32_MAX;
		hr = m_pDevice->CreateSamplerState(&sd, &m_pBlitSampler);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}
	{
		D3D11_DEPTH_STENCIL_DESC dd; ZeroMemory(&dd, sizeof(dd));
		dd.DepthEnable = FALSE; dd.StencilEnable = FALSE;
		hr = m_pDevice->CreateDepthStencilState(&dd, &m_pBlitNoDepth);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}
	{
		D3D11_RASTERIZER_DESC rd; ZeroMemory(&rd, sizeof(rd));
		rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_NONE; rd.DepthClipEnable = TRUE;
		hr = m_pDevice->CreateRasterizerState(&rd, &m_pBlitRaster);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

EXIT:;
	if (pVS != NULL) pVS->Release();
	if (pPS != NULL) pPS->Release();
	if (pErr != NULL) pErr->Release();
	if (result != 0) _ReleaseBlit();
	return result;
}

void DXRenderer11::_ReleaseBlit()
{
	if (m_pBlitRaster != NULL)  { m_pBlitRaster->Release();  m_pBlitRaster = NULL; }
	if (m_pBlitNoDepth != NULL) { m_pBlitNoDepth->Release(); m_pBlitNoDepth = NULL; }
	if (m_pBlitSampler != NULL) { m_pBlitSampler->Release(); m_pBlitSampler = NULL; }
	if (m_pBlitCB != NULL)      { m_pBlitCB->Release();      m_pBlitCB = NULL; }
	if (m_pBlitPS != NULL)      { m_pBlitPS->Release();      m_pBlitPS = NULL; }
	if (m_pBlitVS != NULL)      { m_pBlitVS->Release();      m_pBlitVS = NULL; }
}

//******************************************************************************
// equirectangular pipeline (fullscreen triangle that samples the scene cubemap)
//******************************************************************************
static const char* DXR11_EQ_SHADER =
	"TextureCube g_Cube : register(t0);\n"
	"SamplerState g_Samp : register(s0);\n"
	"cbuffer EqCB : register(b0) { row_major float4x4 g_CamToWorld; };\n"
	"struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };\n"
	"VSOut VSMain(uint id : SV_VertexID) {\n"
	"  VSOut o; float2 t = float2((id << 1) & 2, id & 2);\n"
	"  o.pos = float4(t.x * 2.0 - 1.0, 1.0 - t.y * 2.0, 0.0, 1.0);\n"
	"  o.uv = t; return o;\n"
	"}\n"
	"float4 PSMain(VSOut i) : SV_TARGET {\n"
	"  const float PI = 3.14159265358979;\n"
	"  float lon = (i.uv.x - 0.5) * 2.0 * PI;\n"   // -PI..PI, center = forward
	"  float lat = (0.5 - i.uv.y) * PI;\n"          // +PI/2 (top) .. -PI/2 (bottom)
	"  float cl = cos(lat);\n"
	"  float3 dirCam = float3(sin(lon) * cl, sin(lat), cos(lon) * cl);\n"
	"  float3 dirWorld = normalize(mul(float4(dirCam, 0.0), g_CamToWorld).xyz);\n"
	"  return g_Cube.Sample(g_Samp, dirWorld);\n"
	"}\n";

int DXRenderer11::_InitEquirect()
{
	if (m_pEqVS != NULL) return 0;   // already built
	if (m_pDevice == NULL) return YN_SET_ERR("Program error.", 0, 0);

	HRESULT hr = S_OK;
	int result = 0;
	ID3DBlob* pVS = NULL; ID3DBlob* pPS = NULL; ID3DBlob* pErr = NULL;

	hr = D3DCompile(DXR11_EQ_SHADER, strlen(DXR11_EQ_SHADER), NULL, NULL, NULL, "VSMain", "vs_4_0", 0, 0, &pVS, &pErr);
	if (FAILED(hr) || (pVS == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }
	hr = D3DCompile(DXR11_EQ_SHADER, strlen(DXR11_EQ_SHADER), NULL, NULL, NULL, "PSMain", "ps_4_0", 0, 0, &pPS, &pErr);
	if (FAILED(hr) || (pPS == NULL)) { result = YN_SET_ERR("Shader compile error.", hr, 0); goto EXIT; }

	hr = m_pDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), NULL, &m_pEqVS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	hr = m_pDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), NULL, &m_pEqPS);
	if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }

	{
		D3D11_BUFFER_DESC cb; ZeroMemory(&cb, sizeof(cb));
		cb.ByteWidth = sizeof(float) * 16; cb.Usage = D3D11_USAGE_DYNAMIC;
		cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER; cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr = m_pDevice->CreateBuffer(&cb, NULL, &m_pEqCB);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}
	{
		D3D11_SAMPLER_DESC sd; ZeroMemory(&sd, sizeof(sd));
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.MaxLOD = D3D11_FLOAT32_MAX;
		hr = m_pDevice->CreateSamplerState(&sd, &m_pEqSampler);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}
	{
		D3D11_DEPTH_STENCIL_DESC dd; ZeroMemory(&dd, sizeof(dd));
		dd.DepthEnable = FALSE; dd.StencilEnable = FALSE;
		hr = m_pDevice->CreateDepthStencilState(&dd, &m_pEqNoDepth);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}
	{
		D3D11_RASTERIZER_DESC rd; ZeroMemory(&rd, sizeof(rd));
		rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_NONE; rd.DepthClipEnable = TRUE;
		hr = m_pDevice->CreateRasterizerState(&rd, &m_pEqRaster);
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

EXIT:;
	if (pVS != NULL) pVS->Release();
	if (pPS != NULL) pPS->Release();
	if (pErr != NULL) pErr->Release();
	if (result != 0) _ReleaseEquirect();
	return result;
}

void DXRenderer11::_ReleaseEquirect()
{
	if (m_pEqRaster != NULL)  { m_pEqRaster->Release();  m_pEqRaster = NULL; }
	if (m_pEqNoDepth != NULL) { m_pEqNoDepth->Release(); m_pEqNoDepth = NULL; }
	if (m_pEqSampler != NULL) { m_pEqSampler->Release(); m_pEqSampler = NULL; }
	if (m_pEqCB != NULL)      { m_pEqCB->Release();      m_pEqCB = NULL; }
	if (m_pEqPS != NULL)      { m_pEqPS->Release();      m_pEqPS = NULL; }
	if (m_pEqVS != NULL)      { m_pEqVS->Release();      m_pEqVS = NULL; }
}

//******************************************************************************
// 360 frame: render the scene into the cubemap (6 world-axis faces from the
// camera position), then remap to a 2:1 equirectangular frame oriented so the
// camera forward is the panorama center.
//******************************************************************************
int DXRenderer11::_RenderFrame360(bool transparent, int stageSlot)
{
	XMFLOAT3 camPos(0.0f, 0.0f, 0.0f);
	float rollAngle = 0.0f;
	XMMATRIX camToWorld = XMMatrixIdentity();
	if (m_pCam11 != NULL) {
		m_pCam11->UpdateScrollForExport();
		D3DXMATRIX d3dView, d3dProj;
		m_pCam11->GetMatrices(1.0f, &d3dView, &d3dProj);
		XMMATRIX view = XMLoadFloat4x4((const XMFLOAT4X4*)&d3dView);
		view.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);   // drop translation -> pure rotation
		camToWorld = XMMatrixTranspose(view);              // inverse of the orthonormal view rotation
		rollAngle = m_pCam11->GetManualRollAngle();
		D3DXVECTOR3 p; m_pCam11->GetPosition(&p);
		camPos = XMFLOAT3(p.x, p.y, p.z);
	}

	float clr[4];
	if (transparent) { clr[0] = clr[1] = clr[2] = clr[3] = 0.0f; }
	else {
		D3DCOLOR bg = m_BgColor;
		clr[0] = (float)((bg >> 16) & 0xFF) / 255.0f;
		clr[1] = (float)((bg >> 8) & 0xFF) / 255.0f;
		clr[2] = (float)(bg & 0xFF) / 255.0f;
		clr[3] = 1.0f;
	}

	// D3D cubemap face order: +X,-X,+Y,-Y,+Z,-Z
	static const float fwd[6][3] = { {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
	static const float up6[6][3] = { {0,1,0},{0,1,0},{0,0,-1},{0,0,1},{0,1,0},{0,1,0} };

	D3D11_VIEWPORT fvp;
	fvp.TopLeftX = 0; fvp.TopLeftY = 0;
	fvp.Width = (float)m_CubeFaceRes; fvp.Height = (float)m_CubeFaceRes;
	fvp.MinDepth = 0.0f; fvp.MaxDepth = 1.0f;
	XMVECTOR eye = XMVectorSet(camPos.x, camPos.y, camPos.z, 1.0f);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 1.0f, 1000.0f);
	bool faceMS = (m_CubeSampleCount > 1) && (m_pCubeFaceMSRTV != NULL);
	for (int f = 0; f < 6; f++) {
		XMVECTOR at = XMVectorSet(camPos.x + fwd[f][0], camPos.y + fwd[f][1], camPos.z + fwd[f][2], 1.0f);
		XMVECTOR up = XMVectorSet(up6[f][0], up6[f][1], up6[f][2], 0.0f);
		XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
		ID3D11RenderTargetView* pRTV = faceMS ? m_pCubeFaceMSRTV : m_pCubeRTV[f];
		ID3D11DepthStencilView* pDSV = faceMS ? m_pCubeFaceMSDSV : m_pCubeDSV;
		m_pContext->RSSetViewports(1, &fvp);
		m_pContext->OMSetRenderTargets(1, &pRTV, pDSV);
		m_pContext->ClearRenderTargetView(pRTV, clr);
		m_pContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_DrawSceneComponents(view * proj, camPos, rollAngle, transparent);
		if (faceMS) {
			// resolve the MSAA face into cube array slice f (antialiased)
			m_pContext->ResolveSubresource(m_pCubeTex, (UINT)f, m_pCubeFaceMS, 0, DXGI_FORMAT_B8G8R8A8_UNORM);
		}
	}

	// equirect remap -> m_pOffTex
	ID3D11RenderTargetView* nullRTV[1] = { NULL };
	m_pContext->OMSetRenderTargets(1, nullRTV, NULL);   // free the cube RTVs to bind as SRV
	D3D11_VIEWPORT ovp;
	ovp.TopLeftX = 0; ovp.TopLeftY = 0;
	ovp.Width = (float)m_OffW; ovp.Height = (float)m_OffH;
	ovp.MinDepth = 0.0f; ovp.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &ovp);
	m_pContext->OMSetRenderTargets(1, &m_pOffRTV, NULL);
	m_pContext->ClearRenderTargetView(m_pOffRTV, clr);

	D3D11_MAPPED_SUBRESOURCE ms;
	if (SUCCEEDED(m_pContext->Map(m_pEqCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
		XMStoreFloat4x4((XMFLOAT4X4*)ms.pData, camToWorld);
		m_pContext->Unmap(m_pEqCB, 0);
	}
	UINT zero = 0;
	ID3D11Buffer* noVB[1] = { NULL };
	m_pContext->IASetInputLayout(NULL);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->IASetVertexBuffers(0, 1, noVB, &zero, &zero);
	m_pContext->VSSetShader(m_pEqVS, NULL, 0);
	m_pContext->PSSetShader(m_pEqPS, NULL, 0);
	m_pContext->PSSetConstantBuffers(0, 1, &m_pEqCB);
	m_pContext->PSSetShaderResources(0, 1, &m_pCubeSRV);
	m_pContext->PSSetSamplers(0, 1, &m_pEqSampler);
	m_pContext->OMSetDepthStencilState(m_pEqNoDepth, 0);
	{ float bf[4] = { 0,0,0,0 }; m_pContext->OMSetBlendState(NULL, bf, 0xFFFFFFFF); }
	m_pContext->RSSetState(m_pEqRaster);
	m_pContext->Draw(3, 0);

	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	m_pContext->PSSetShaderResources(0, 1, nullSRV);

	m_pContext->CopyResource(m_pOffStaging[stageSlot], m_pOffTex);
	return 0;
}

int DXRenderer11::ReadOffscreenBGRA(int stageSlot, unsigned char* pDst, int dstRowBytes)
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE ms;
	unsigned int y = 0;
	int rowBytes = (int)m_OffW * 4;

	if (stageSlot < 0 || stageSlot > 1) return YN_SET_ERR("Program error.", stageSlot, 0);
	if ((m_pContext == NULL) || (m_pOffStaging[stageSlot] == NULL) || (pDst == NULL)) {
		return YN_SET_ERR("Program error.", 0, 0);
	}
	if (dstRowBytes < rowBytes) return YN_SET_ERR("Program error.", dstRowBytes, rowBytes);

	// map the staging copied by a previous RenderOffscreenFrame (GPU already done)
	hr = m_pContext->Map(m_pOffStaging[stageSlot], 0, D3D11_MAP_READ, 0, &ms);
	if (FAILED(hr)) return YN_SET_ERR("DirectX API error.", hr, 0);

	for (y = 0; y < m_OffH; y++) {
		memcpy(pDst + (size_t)y * dstRowBytes,
		       (const unsigned char*)ms.pData + (size_t)y * ms.RowPitch,
		       rowBytes);
	}
	m_pContext->Unmap(m_pOffStaging[stageSlot], 0);
	return 0;
}

void DXRenderer11::EndOffscreen()
{
	for (int i = 0; i < 2; i++) {
		if (m_pOffStaging[i] != NULL) { m_pOffStaging[i]->Release(); m_pOffStaging[i] = NULL; }
	}
	if (m_pOffDSVMS != NULL)   { m_pOffDSVMS->Release();   m_pOffDSVMS = NULL; }
	if (m_pOffDepthMS != NULL) { m_pOffDepthMS->Release(); m_pOffDepthMS = NULL; }
	if (m_pOffRTVMS != NULL)   { m_pOffRTVMS->Release();   m_pOffRTVMS = NULL; }
	if (m_pOffTexMS != NULL)   { m_pOffTexMS->Release();   m_pOffTexMS = NULL; }
	if (m_pOffDSV != NULL)     { m_pOffDSV->Release();     m_pOffDSV = NULL; }
	if (m_pOffDepth != NULL)   { m_pOffDepth->Release();   m_pOffDepth = NULL; }
	if (m_pOffRTV != NULL)     { m_pOffRTV->Release();     m_pOffRTV = NULL; }
	if (m_pOffTex != NULL)     { m_pOffTex->Release();     m_pOffTex = NULL; }
	// 360 resources
	if (m_pCubeFaceMSDSV != NULL)   { m_pCubeFaceMSDSV->Release();   m_pCubeFaceMSDSV = NULL; }
	if (m_pCubeFaceMSDepth != NULL) { m_pCubeFaceMSDepth->Release(); m_pCubeFaceMSDepth = NULL; }
	if (m_pCubeFaceMSRTV != NULL)   { m_pCubeFaceMSRTV->Release();   m_pCubeFaceMSRTV = NULL; }
	if (m_pCubeFaceMS != NULL)      { m_pCubeFaceMS->Release();      m_pCubeFaceMS = NULL; }
	m_CubeSampleCount = 1;
	if (m_pCubeSRV != NULL)   { m_pCubeSRV->Release();   m_pCubeSRV = NULL; }
	if (m_pCubeDSV != NULL)   { m_pCubeDSV->Release();   m_pCubeDSV = NULL; }
	if (m_pCubeDepth != NULL) { m_pCubeDepth->Release(); m_pCubeDepth = NULL; }
	for (int f = 0; f < 6; f++) { if (m_pCubeRTV[f] != NULL) { m_pCubeRTV[f]->Release(); m_pCubeRTV[f] = NULL; } }
	if (m_pCubeTex != NULL)   { m_pCubeTex->Release();   m_pCubeTex = NULL; }
	_ReleaseEquirect();
	m_Off360 = false;
	m_CubeFaceRes = 0;
	m_OffSampleCount = 1;
	m_OffW = 0; m_OffH = 0;

	// restore the backbuffer render target + viewport for live rendering
	if (m_pContext != NULL) {
		if (m_pRTV != NULL) m_pContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
		if ((m_Width > 0) && (m_Height > 0)) {
			D3D11_VIEWPORT vp;
			vp.TopLeftX = 0; vp.TopLeftY = 0;
			vp.Width = (float)m_Width; vp.Height = (float)m_Height;
			vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
			m_pContext->RSSetViewports(1, &vp);
		}
	}
}

//******************************************************************************
// Loading screen (drawn periodically while a file is loading)
//******************************************************************************
int DXRenderer11::DrawLoadingScreen(
		const char* message,
		float progress
	)
{
	MSG msg;
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (m_pContext == NULL) return 0;
	if (ImGui::GetCurrentContext() == NULL) return 0;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			PostQuitMessage((int)msg.wParam);
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	_BeginImGuiFrame();

	m_pContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
	m_pContext->ClearRenderTargetView(m_pRTV, clearColor);
	m_pContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	{
		if (progress < 0.0f) progress = 0.0f;
		if (progress > 1.0f) progress = 1.0f;

		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(520.0f, 0.0f));
		ImGui::Begin("Loading", NULL,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoTitleBar);

		ImGui::Text("Loading MIDI");
		ImGui::Separator();
		ImGui::Spacing();

		// current step (e.g. "Building notes: 524288 / 1000000")
		ImGui::TextWrapped("%s", (message != NULL) ? message : "Please wait...");
		ImGui::Spacing();

		// progress bar with the overall percentage printed on it
		char overlay[32];
		sprintf_s(overlay, sizeof(overlay), "%d%%", (int)(progress * 100.0f + 0.5f));
		ImGui::ProgressBar(progress, ImVec2(-1.0f, 22.0f), overlay);

		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// no vsync while loading: progress updates (e.g. per-note-batch) stay snappy
	// instead of blocking ~16ms on each present.
	m_pSwapChain->Present(0, 0);

	return 0;
}

//******************************************************************************
// Resize : recreate backbuffer-sized targets
//******************************************************************************
int DXRenderer11::Resize(
		unsigned int width,
		unsigned int height
	)
{
	int result = 0;
	HRESULT hr = S_OK;

	if (m_pSwapChain == NULL) goto EXIT;
	if (width == 0) width = 1;
	if (height == 0) height = 1;

	_ReleaseTargets();

	hr = m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr)) {
		result = YN_SET_ERR("DirectX API error.", hr, 0);
		goto EXIT;
	}

	result = _CreateTargets(width, height);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Terminate
//******************************************************************************
void DXRenderer11::Terminate()
{
	// NOTE: Terminate() runs after the window is already destroyed (WM_DESTROY
	// -> WM_QUIT -> Run() returns -> Terminate()). Calling ImGui_ImplWin32_Shutdown()
	// at that point touches the dead window and crashes on close. The original DX9
	// renderer never shut ImGui down either; the OS reclaims it on process exit.
	m_ImContext = NULL;
	m_Logo11.Release();
	_ReleaseTargets();
	_ReleaseBlit();   //ced 20260628: SSAA 縮小ブリット
	// Release the shared static pipelines so they rebuild on the next device.
	// These are created once (InitPipeline early-returns if already built); after
	// a device recreation (AA / window-size change) they would otherwise keep
	// referencing the destroyed device -> textured draws render garbage ("red").
	DXPrimitive11::ReleasePipeline();
	DXNoteBox11::ReleasePipeline();
	DXNoteRain11::ReleasePipeline();
	DXNoteBoxRing11::ReleasePipeline();
	if (m_pSwapChain != NULL) { m_pSwapChain->Release(); m_pSwapChain = NULL; }
	if (m_pContext != NULL)   { m_pContext->Release();   m_pContext = NULL; }
	if (m_pDevice != NULL)    { m_pDevice->Release();    m_pDevice = NULL; }
}
