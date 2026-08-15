//******************************************************************************
//
// MIDITrail / MTLoadingScreen11
//
// Loading screen renderer.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTLoadingScreen11.h"
#include <cstdio>

using namespace YNBaseLib;
using namespace DirectX::SimpleMath;

// Layout constants (96 dpi baseline)
static const unsigned long BASE_PANEL_WIDTH     = 520;
static const unsigned long BASE_FONT_SIZE       = 14;
static const unsigned long BASE_PADDING         = 12;
static const unsigned long BASE_SEPARATOR_H     = 1;
static const unsigned long BASE_BAR_HEIGHT      = 22;
static const unsigned long BASE_SPACING         = 6;

// Colors (ced ImGui dark theme equivalents)
static const DWORD COLOR_PANEL_BG    = 0xF00F0F0F;  // RGBA(15, 15, 15, 240)
static const DWORD COLOR_SEPARATOR   = 0x806E6E80;  // RGBA(110, 110, 128, 128)
static const DWORD COLOR_BAR_TRACK   = 0x8A294A7A;  // RGBA(41, 74, 122, 138)
static const DWORD COLOR_BAR_FILL    = 0xFFE6B300;   // RGBA(230, 179, 0, 255)


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTLoadingScreen11::MTLoadingScreen11()
{
	m_hWnd = NULL;
	m_pRenderer = NULL;
	m_Dpi = 96;
	m_isReady = false;
	m_pWhiteTexSRV = NULL;
}

MTLoadingScreen11::~MTLoadingScreen11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTLoadingScreen11::Create(
		HWND hWnd,
		DXRenderer11* pRenderer
	)
{
	int result = 0;

	Release();

	m_hWnd = hWnd;
	m_pRenderer = pRenderer;
	m_Dpi = GetDpiForWindow(hWnd);

	ID3D11Device* pDevice = pRenderer->GetDevice();
	ID3D11DeviceContext* pContext = pRenderer->GetContext();

	result = _CreateWhiteTexture(pDevice);
	if (result != 0) goto EXIT;

	// Panel background
	result = m_PanelPrimitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_PanelPrimitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;
	m_PanelPrimitive.SetLightEnable(false);
	m_PanelPrimitive.SetDepthWrite(false);

	// Separator
	result = m_SeparatorPrimitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_SeparatorPrimitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;
	m_SeparatorPrimitive.SetLightEnable(false);
	m_SeparatorPrimitive.SetDepthWrite(false);

	// Progress bar track
	result = m_BarTrackPrimitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_BarTrackPrimitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;
	m_BarTrackPrimitive.SetLightEnable(false);
	m_BarTrackPrimitive.SetDepthWrite(false);

	// Progress bar fill
	result = m_BarFillPrimitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_BarFillPrimitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;
	m_BarFillPrimitive.SetLightEnable(false);
	m_BarFillPrimitive.SetDepthWrite(false);

	// Set up indices for all rect primitives
	{
		DXPrimitive11* prims[] = {
			&m_PanelPrimitive, &m_SeparatorPrimitive,
			&m_BarTrackPrimitive, &m_BarFillPrimitive
		};
		for (auto* p : prims) {
			unsigned long* pIndex = NULL;
			result = p->LockIndex(pContext, &pIndex);
			if (result != 0) goto EXIT;
			pIndex[0] = 0; pIndex[1] = 1; pIndex[2] = 2;
			pIndex[3] = 2; pIndex[4] = 1; pIndex[5] = 3;
			p->UnlockIndex(pContext);
		}
	}

	// Title caption
	result = m_TitleCaption.Create(
		pDevice, pContext, L"Tahoma", _Scale(BASE_FONT_SIZE), L"Loading MIDI");
	if (result != 0) goto EXIT;

	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTLoadingScreen11::Release()
{
	m_TitleCaption.Release();
	m_MessageCaption.Release();
	m_PercentCaption.Release();

	m_PanelPrimitive.Release();
	m_SeparatorPrimitive.Release();
	m_BarTrackPrimitive.Release();
	m_BarFillPrimitive.Release();

	if (m_pWhiteTexSRV != NULL) {
		m_pWhiteTexSRV->Release();
		m_pWhiteTexSRV = NULL;
	}

	m_isReady = false;
}

//******************************************************************************
// Update: render one frame of the loading screen
//******************************************************************************
int MTLoadingScreen11::Update(
		float progress,
		const char* message
	)
{
	if (!m_isReady || m_pRenderer == NULL) return 0;

	int result = 0;
	ID3D11Device* pDevice = m_pRenderer->GetDevice();
	ID3D11DeviceContext* pContext = m_pRenderer->GetContext();
	unsigned int screenW = m_pRenderer->GetWidth();
	unsigned int screenH = m_pRenderer->GetHeight();

	if (screenW == 0 || screenH == 0) return 0;

	if (progress < 0.0f) progress = 0.0f;
	if (progress > 1.0f) progress = 1.0f;

	// DPI-scaled layout values
	unsigned long panelW   = _Scale(BASE_PANEL_WIDTH);
	unsigned long padding  = _Scale(BASE_PADDING);
	unsigned long sepH     = _Scale(BASE_SEPARATOR_H);
	unsigned long barH     = _Scale(BASE_BAR_HEIGHT);
	unsigned long spacing  = _Scale(BASE_SPACING);
	unsigned long fontSize = _Scale(BASE_FONT_SIZE);

	// Recreate message and percent captions each frame (text changes)
	m_MessageCaption.Release();
	m_PercentCaption.Release();

	const char* displayMsg = (message != NULL) ? message : "Please wait...";
	int msgLen = MultiByteToWideChar(CP_ACP, 0, displayMsg, -1, NULL, 0);
	WCHAR* pMsgW = new WCHAR[msgLen];
	MultiByteToWideChar(CP_ACP, 0, displayMsg, -1, pMsgW, msgLen);
	m_MessageCaption.Create(pDevice, pContext, L"Tahoma", fontSize, pMsgW);
	delete[] pMsgW;

	char percentStr[16];
	sprintf_s(percentStr, sizeof(percentStr), "%d%%", (int)(progress * 100.0f + 0.5f));
	int pctLen = MultiByteToWideChar(CP_ACP, 0, percentStr, -1, NULL, 0);
	WCHAR* pPctW = new WCHAR[pctLen];
	MultiByteToWideChar(CP_ACP, 0, percentStr, -1, pPctW, pctLen);
	m_PercentCaption.Create(pDevice, pContext, L"Tahoma", fontSize, pPctW);
	delete[] pPctW;

	// Calculate panel height from content
	unsigned long titleH = 0, titleW = 0;
	m_TitleCaption.GetTextureSize(&titleH, &titleW);
	unsigned long msgH = 0, msgW = 0;
	m_MessageCaption.GetTextureSize(&msgH, &msgW);

	unsigned long contentH = padding + titleH + spacing + sepH + spacing
	                         + msgH + spacing + barH + padding;

	// Panel position (centered)
	float panelX = ((float)screenW - (float)panelW) * 0.5f;
	float panelY = ((float)screenH - (float)contentH) * 0.5f;
	float contentX = panelX + (float)padding;
	float innerW = (float)(panelW - padding * 2);

	// Begin frame (clear to black)
	result = m_pRenderer->BeginFrame();
	if (result != 0) goto EXIT;

	// Draw panel background
	result = _DrawRect(pContext, &m_PanelPrimitive,
		panelX, panelY, (float)panelW, (float)contentH,
		COLOR_PANEL_BG, screenW, screenH);
	if (result != 0) goto EXIT;

	{
		float curY = panelY + (float)padding;

		// Title
		m_TitleCaption.Draw(pContext, contentX, curY, 1.0f, screenW, screenH);
		curY += (float)titleH + (float)spacing;

		// Separator
		result = _DrawRect(pContext, &m_SeparatorPrimitive,
			contentX, curY, innerW, (float)sepH,
			COLOR_SEPARATOR, screenW, screenH);
		if (result != 0) goto EXIT;
		curY += (float)sepH + (float)spacing;

		// Message
		m_MessageCaption.Draw(pContext, contentX, curY, 1.0f, screenW, screenH);
		curY += (float)msgH + (float)spacing;

		// Progress bar track
		result = _DrawRect(pContext, &m_BarTrackPrimitive,
			contentX, curY, innerW, (float)barH,
			COLOR_BAR_TRACK, screenW, screenH);
		if (result != 0) goto EXIT;

		// Progress bar fill
		float fillW = innerW * progress;
		if (fillW > 0.0f) {
			result = _DrawRect(pContext, &m_BarFillPrimitive,
				contentX, curY, fillW, (float)barH,
				COLOR_BAR_FILL, screenW, screenH);
			if (result != 0) goto EXIT;
		}

		// Percent text (centered on bar)
		unsigned long pctH = 0, pctW = 0;
		m_PercentCaption.GetTextureSize(&pctH, &pctW);
		float pctX = contentX + (innerW - (float)pctW) * 0.5f;
		float pctY = curY + ((float)barH - (float)pctH) * 0.5f;
		m_PercentCaption.Draw(pContext, pctX, pctY, 1.0f, screenW, screenH);
	}

	// Present
	result = m_pRenderer->EndFrame();
	if (result != 0) goto EXIT;

	// Keep window responsive
	_PumpMessages();

EXIT:;
	return result;
}

//******************************************************************************
// Create 1x1 white texture for solid-color rectangles
//******************************************************************************
int MTLoadingScreen11::_CreateWhiteTexture(
		ID3D11Device* pDevice
	)
{
	int result = 0;
	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	unsigned char whitePixel[4] = { 255, 255, 255, 255 };
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = whitePixel;
	initData.SysMemPitch = 4;
	initData.SysMemSlicePitch = 0;

	ID3D11Texture2D* pTex = NULL;
	hr = pDevice->CreateTexture2D(&texDesc, &initData, &pTex);
	if (FAILED(hr)) {
		result = YN_SET_ERR("CreateTexture2D failed.", hr, 0);
		goto EXIT;
	}

	hr = pDevice->CreateShaderResourceView(pTex, NULL, &m_pWhiteTexSRV);
	if (FAILED(hr)) {
		result = YN_SET_ERR("CreateShaderResourceView failed.", hr, 0);
		goto EXIT;
	}

EXIT:;
	if (pTex != NULL) pTex->Release();
	return result;
}

//******************************************************************************
// Draw a solid-color rectangle
//******************************************************************************
int MTLoadingScreen11::_DrawRect(
		ID3D11DeviceContext* pContext,
		DXPrimitive11* pPrimitive,
		float x, float y, float w, float h,
		DWORD color,
		unsigned int screenW, unsigned int screenH
	)
{
	int result = 0;

	float sw = (float)screenW;
	float sh = (float)screenH;
	float ndcX0 = (x / sw) * 2.0f - 1.0f;
	float ndcX1 = ((x + w) / sw) * 2.0f - 1.0f;
	float ndcY0 = 1.0f - (y / sh) * 2.0f;
	float ndcY1 = 1.0f - ((y + h) / sh) * 2.0f;

	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	result = pPrimitive->LockVertex(pContext, &pVertex);
	if (result != 0) return result;

	auto setVtx = [&](unsigned long i, float px, float py) {
		pVertex[i].pos[0] = px;
		pVertex[i].pos[1] = py;
		pVertex[i].pos[2] = 0.0f;
		pVertex[i].normal[0] = 0.0f;
		pVertex[i].normal[1] = 0.0f;
		pVertex[i].normal[2] = -1.0f;
		pVertex[i].color = color;
		pVertex[i].uv[0] = 0.0f;
		pVertex[i].uv[1] = 0.0f;
	};

	setVtx(0, ndcX0, ndcY0);
	setVtx(1, ndcX1, ndcY0);
	setVtx(2, ndcX0, ndcY1);
	setVtx(3, ndcX1, ndcY1);

	pPrimitive->UnlockVertex(pContext);

	pPrimitive->SetTexture(m_pWhiteTexSRV);

	Matrix identity;
	Vector4 lightDir(0.0f, 0.0f, 0.0f, 0.0f);
	result = pPrimitive->Draw(pContext, identity, lightDir);

	return result;
}

//******************************************************************************
// Pump Windows messages to keep window responsive
//******************************************************************************
void MTLoadingScreen11::_PumpMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//******************************************************************************
// DPI scaling helper
//******************************************************************************
unsigned long MTLoadingScreen11::_Scale(
		unsigned long base
	)
{
	return MulDiv(base, m_Dpi, 96);
}
