//******************************************************************************
//
// MIDITrail / MTDiagOverlay11
//
// Diagnostic information overlay renderer.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "MTDiagOverlay11.h"
#include "RDDiagManager.h"
#include "RDFormatProfiles.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTDiagOverlay11::MTDiagOverlay11()
	: m_hWnd(NULL)
	, m_pDevice(nullptr)
	, m_pContext(nullptr)
	, m_Dpi(USER_DEFAULT_SCREEN_DPI)
	, m_StaticLineCount(0)
	, m_DynamicLineCount(0)
	, m_TotalLineCount(0)
	, m_pBgSRV(nullptr)
	, m_MaxLineChars(0)
	, m_StaticMaxChars(0)
	, m_CachedScreenWidth(0)
	, m_CachedScreenHeight(0)
	, m_CachedTitleAreaH(0.0f)
	, m_CachedCounterAreaH(0.0f)
	, m_CachedMaxLineChars(0)
	, m_Color(1.0f, 1.0f, 1.0f, 1.0f)
{
	memset(&m_CachedLayout, 0, sizeof(m_CachedLayout));
}

MTDiagOverlay11::~MTDiagOverlay11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTDiagOverlay11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		HWND hWnd
	)
{
	int result = 0;

	Release();

	m_pDevice = pDevice;
	m_pContext = pContext;
	m_hWnd = hWnd;
	m_Dpi = _GetDpi();

	m_StaticLineCount = (unsigned long)RDFormatProfile::OverlayMachineSignatureCount;
	m_DynamicLineCount = (unsigned long)RDFormatProfile::OverlayRuntimeCount;
	m_TotalLineCount = m_StaticLineCount + m_DynamicLineCount;

	unsigned long scaledFontSize = _GetScaledFontSize();

	{
		unsigned long rgb = 0x00FFFFFF;
		result = m_SharedFontTexture.SetFont(MTDIAGOVERLAY11_FONTNAME, scaledFontSize, rgb, true);
		if (result != 0) goto EXIT;
		result = m_SharedFontTexture.CreateTexture(pDevice, MT_ASCII_PRINTABLE_CHARS);
		if (result != 0) goto EXIT;
	}

	{
		unsigned long texH = 0, texW = 0;
		m_SharedFontTexture.GetTextureSize(&texH, &texW);

		for (unsigned long i = 0; i < m_TotalLineCount; i++) {
			auto* pCaption = new MTDynamicCaption11();
			result = pCaption->CreateWithSharedTexture(
				pDevice, pContext,
				m_SharedFontTexture.GetTexture(), texW, texH,
				MT_ASCII_PRINTABLE_CHARS, MTDIAGOVERLAY11_CAPTION_SIZE);
			if (result != 0) {
				delete pCaption;
				goto EXIT;
			}
			pCaption->SetColor(m_Color);
			m_Lines.push_back(pCaption);
		}
	}

	result = _CreateBgTexture();
	if (result != 0) goto EXIT;

	result = _UpdateStaticLines();
	if (result != 0) goto EXIT;

	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		unsigned int sw = rect.right - rect.left;
		unsigned int sh = rect.bottom - rect.top;
		DiagLayoutParams layout;
		_CalcLayout(sw, sh, NULL, &layout);
		result = _UpdateBgVertices(layout, sw, sh);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTDiagOverlay11::Release()
{
	for (auto* p : m_Lines) {
		p->Release();
		delete p;
	}
	m_Lines.clear();
	m_SharedFontTexture.Clear();
	m_BgPrimitive.Release();
	if (m_pBgSRV != nullptr) {
		m_pBgSRV->Release();
		m_pBgSRV = nullptr;
	}
	m_StaticLineCount = 0;
	m_DynamicLineCount = 0;
	m_TotalLineCount = 0;
	m_MaxLineChars = 0;
	m_StaticMaxChars = 0;
	m_CachedScreenWidth = 0;
	m_CachedScreenHeight = 0;
	m_CachedTitleAreaH = 0.0f;
	m_CachedCounterAreaH = 0.0f;
	m_CachedMaxLineChars = 0;
}

//******************************************************************************
// Create background texture (1x1 black with alpha)
//******************************************************************************
int MTDiagOverlay11::_CreateBgTexture()
{
	int result = 0;
	unsigned char pixel[4] = { 0, 0, 0, MTDIAGOVERLAY11_BG_ALPHA };

	result = DXTexture11::CreateFromRGBA(m_pDevice, pixel, 1, 1, &m_pBgSRV);
	if (result != 0) goto EXIT;

	result = m_BgPrimitive.CreateVertexBuffer(m_pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_BgPrimitive.CreateIndexBuffer(m_pDevice, 6);
	if (result != 0) goto EXIT;

	{
		unsigned long* pIdx = nullptr;
		result = m_BgPrimitive.LockIndex(m_pContext, &pIdx);
		if (result != 0) goto EXIT;
		pIdx[0] = 0; pIdx[1] = 1; pIdx[2] = 2;
		pIdx[3] = 1; pIdx[4] = 3; pIdx[5] = 2;
		m_BgPrimitive.UnlockIndex(m_pContext);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Calculate layout parameters (single source of truth)
//******************************************************************************
void MTDiagOverlay11::_CalcLayout(
		unsigned int screenWidth,
		unsigned int screenHeight,
		const MTSceneLayoutInfo* pLayoutInfo,
		DiagLayoutParams* pOut
	)
{
	if (m_Lines.empty()) {
		memset(pOut, 0, sizeof(*pOut));
		return;
	}

	unsigned long maxChars = m_MaxLineChars + MTDIAGOVERLAY11_RIGHT_MARGIN_CHARS;
	if (maxChars == 0) maxChars = 1;

	float mag = MTDIAGOVERLAY11_MAGRATE;
	float charW = 0.0f, charH = 0.0f;
	m_Lines[0]->GetDisplayCharSize(mag, &charW, &charH);

	float lineHeight = charH + MTDIAGOVERLAY11_LINE_SPACING;
	float overlayWidth = charW * (float)maxChars;
	float dashTitleH = (pLayoutInfo != NULL) ? pLayoutInfo->titleAreaHeight : 0.0f;
	float dashCounterH = (pLayoutInfo != NULL) ? pLayoutInfo->counterAreaHeight : 0.0f;
	float topOffset = dashTitleH + MTDIAGOVERLAY11_MARGIN + lineHeight;
	float overlayHeight = lineHeight * (float)m_TotalLineCount;

	float availW = (float)screenWidth - MTDIAGOVERLAY11_MARGIN * 2.0f;
	float availH = (float)screenHeight - topOffset - dashCounterH - MTDIAGOVERLAY11_MARGIN;

	float shrink = 1.0f;
	if (overlayWidth > availW && overlayWidth > 0.0f) {
		shrink = availW / overlayWidth;
	}
	if (overlayHeight > availH && overlayHeight > 0.0f) {
		float hShrink = availH / overlayHeight;
		if (hShrink < shrink) shrink = hShrink;
	}
	if (shrink < 1.0f) {
		mag *= shrink;
		m_Lines[0]->GetDisplayCharSize(mag, &charW, &charH);
		lineHeight = charH + MTDIAGOVERLAY11_LINE_SPACING * shrink;
		overlayWidth = charW * (float)maxChars;
		overlayHeight = lineHeight * (float)m_TotalLineCount;
		topOffset = dashTitleH + MTDIAGOVERLAY11_MARGIN + lineHeight;
	}

	float textX = (float)screenWidth - overlayWidth - MTDIAGOVERLAY11_MARGIN;
	if (textX < MTDIAGOVERLAY11_MARGIN) textX = MTDIAGOVERLAY11_MARGIN;

	pOut->mag = mag;
	pOut->charW = charW;
	pOut->charH = charH;
	pOut->lineHeight = lineHeight;
	pOut->overlayWidth = overlayWidth;
	pOut->overlayHeight = overlayHeight;
	pOut->topOffset = topOffset;
	pOut->textX = textX;
}

//******************************************************************************
// Update background quad vertices (only when layout changes)
//******************************************************************************
int MTDiagOverlay11::_UpdateBgVertices(
		const DiagLayoutParams& layout,
		unsigned int screenWidth,
		unsigned int screenHeight
	)
{
	int result = 0;
	if (m_Lines.empty()) goto EXIT;

	{
		float bgX = (float)screenWidth - layout.overlayWidth - MTDIAGOVERLAY11_MARGIN - MTDIAGOVERLAY11_PADDING;
		float bgY = layout.topOffset - MTDIAGOVERLAY11_PADDING;
		float bgW = layout.overlayWidth + MTDIAGOVERLAY11_PADDING * 2.0f;
		float bgH = layout.overlayHeight + MTDIAGOVERLAY11_PADDING * 2.0f;
		if (bgX < 0.0f) bgX = 0.0f;

		float sw = (float)screenWidth;
		float sh = (float)screenHeight;
		float ndcX0 = (bgX / sw) * 2.0f - 1.0f;
		float ndcX1 = ((bgX + bgW) / sw) * 2.0f - 1.0f;
		float ndcY0 = 1.0f - (bgY / sh) * 2.0f;
		float ndcY1 = 1.0f - ((bgY + bgH) / sh) * 2.0f;

		DXPRIMITIVE11_VERTEX* pVtx = nullptr;
		result = m_BgPrimitive.LockVertex(m_pContext, &pVtx);
		if (result != 0) goto EXIT;

		pVtx[0].pos[0] = ndcX0; pVtx[0].pos[1] = ndcY0; pVtx[0].pos[2] = 0.0f;
		pVtx[0].normal[0] = 0; pVtx[0].normal[1] = 0; pVtx[0].normal[2] = -1.0f;
		pVtx[0].color = 0xFFFFFFFF; pVtx[0].uv[0] = 0.0f; pVtx[0].uv[1] = 0.0f;

		pVtx[1].pos[0] = ndcX1; pVtx[1].pos[1] = ndcY0; pVtx[1].pos[2] = 0.0f;
		pVtx[1].normal[0] = 0; pVtx[1].normal[1] = 0; pVtx[1].normal[2] = -1.0f;
		pVtx[1].color = 0xFFFFFFFF; pVtx[1].uv[0] = 1.0f; pVtx[1].uv[1] = 0.0f;

		pVtx[2].pos[0] = ndcX0; pVtx[2].pos[1] = ndcY1; pVtx[2].pos[2] = 0.0f;
		pVtx[2].normal[0] = 0; pVtx[2].normal[1] = 0; pVtx[2].normal[2] = -1.0f;
		pVtx[2].color = 0xFFFFFFFF; pVtx[2].uv[0] = 0.0f; pVtx[2].uv[1] = 1.0f;

		pVtx[3].pos[0] = ndcX1; pVtx[3].pos[1] = ndcY1; pVtx[3].pos[2] = 0.0f;
		pVtx[3].normal[0] = 0; pVtx[3].normal[1] = 0; pVtx[3].normal[2] = -1.0f;
		pVtx[3].color = 0xFFFFFFFF; pVtx[3].uv[0] = 1.0f; pVtx[3].uv[1] = 1.0f;

		m_BgPrimitive.UnlockVertex(m_pContext);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Draw background quad (vertices already set)
//******************************************************************************
int MTDiagOverlay11::_DrawBackground(ID3D11DeviceContext* pContext)
{
	int result = 0;

	m_BgPrimitive.SetTexture(m_pBgSRV);

	{
		Matrix identity;
		Vector4 lightDir(0.0f, 0.0f, 0.0f, 0.0f);
		result = m_BgPrimitive.Draw(pContext, identity, lightDir);
	}

	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTDiagOverlay11::Draw(
		ID3D11DeviceContext* pContext,
		unsigned int screenWidth,
		unsigned int screenHeight,
		const MTSceneLayoutInfo* pLayoutInfo
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;
	if (m_Lines.empty()) goto EXIT;

	result = _UpdateDynamicLines();
	if (result != 0) goto EXIT;

	{
		float titleH = (pLayoutInfo != NULL) ? pLayoutInfo->titleAreaHeight : 0.0f;
		float counterH = (pLayoutInfo != NULL) ? pLayoutInfo->counterAreaHeight : 0.0f;

		bool layoutChanged = (screenWidth != m_CachedScreenWidth)
			|| (screenHeight != m_CachedScreenHeight)
			|| (titleH != m_CachedTitleAreaH)
			|| (counterH != m_CachedCounterAreaH)
			|| (m_MaxLineChars != m_CachedMaxLineChars);

		DiagLayoutParams layout;
		if (layoutChanged) {
			_CalcLayout(screenWidth, screenHeight, pLayoutInfo, &layout);
			result = _UpdateBgVertices(layout, screenWidth, screenHeight);
			if (result != 0) goto EXIT;

			m_CachedLayout = layout;
			m_CachedScreenWidth = screenWidth;
			m_CachedScreenHeight = screenHeight;
			m_CachedTitleAreaH = titleH;
			m_CachedCounterAreaH = counterH;
			m_CachedMaxLineChars = m_MaxLineChars;
		} else {
			layout = m_CachedLayout;
		}

		result = _DrawBackground(pContext);
		if (result != 0) goto EXIT;

		for (unsigned long i = 0; i < m_TotalLineCount; i++) {
			float y = layout.topOffset + layout.lineHeight * (float)i;

			result = m_Lines[i]->Draw(pContext, layout.textX, y,
						layout.mag, screenWidth, screenHeight);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTDiagOverlay11::Reset()
{
}

//******************************************************************************
// Window resize notification
//******************************************************************************
void MTDiagOverlay11::OnWindowResize()
{
	UINT newDpi = _GetDpi();
	if (newDpi != m_Dpi) {
		m_Dpi = newDpi;
		_RecreateCaptions();
	}

	m_CachedScreenWidth = 0;
	m_CachedScreenHeight = 0;
}

//******************************************************************************
// DPI helpers
//******************************************************************************
UINT MTDiagOverlay11::_GetDpi()
{
	if (!MTDIAGOVERLAY11_DPI_SCALING) return USER_DEFAULT_SCREEN_DPI;
	if (m_hWnd != NULL) {
		return GetDpiForWindow(m_hWnd);
	}
	return USER_DEFAULT_SCREEN_DPI;
}

unsigned long MTDiagOverlay11::_GetScaledFontSize()
{
	return MulDiv(MTDIAGOVERLAY11_FONTSIZE, m_Dpi, USER_DEFAULT_SCREEN_DPI);
}


int MTDiagOverlay11::_RecreateCaptions()
{
	int result = 0;
	unsigned long scaledFontSize = _GetScaledFontSize();

	for (auto* p : m_Lines) {
		p->Release();
	}

	m_SharedFontTexture.Clear();
	{
		unsigned long rgb = 0x00FFFFFF;
		result = m_SharedFontTexture.SetFont(MTDIAGOVERLAY11_FONTNAME, scaledFontSize, rgb, true);
		if (result != 0) goto EXIT;
		result = m_SharedFontTexture.CreateTexture(m_pDevice, MT_ASCII_PRINTABLE_CHARS);
		if (result != 0) goto EXIT;
	}

	{
		unsigned long texH = 0, texW = 0;
		m_SharedFontTexture.GetTextureSize(&texH, &texW);

		for (unsigned long i = 0; i < m_TotalLineCount; i++) {
			result = m_Lines[i]->CreateWithSharedTexture(
				m_pDevice, m_pContext,
				m_SharedFontTexture.GetTexture(), texW, texH,
				MT_ASCII_PRINTABLE_CHARS, MTDIAGOVERLAY11_CAPTION_SIZE);
			if (result != 0) goto EXIT;
			m_Lines[i]->SetColor(m_Color);
		}
	}

	result = _UpdateStaticLines();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Measure max line width in characters from formatted entries
//******************************************************************************
unsigned long MTDiagOverlay11::_MeasureMaxLineChars(
		const std::vector<RDFormattedEntry>& entries
	)
{
	unsigned long maxChars = 0;
	for (const auto& e : entries) {
		if (e.label[0] == '\0' && e.value[0] == '\0') continue;
		unsigned long len = (unsigned long)(strlen(e.label) + 2 + strlen(e.value));
		if (len > maxChars) maxChars = len;
	}
	return maxChars;
}

//******************************************************************************
// Update static lines (machine signature, called once)
//******************************************************************************
int MTDiagOverlay11::_UpdateStaticLines()
{
	int result = 0;

	auto entries = RDDiagManager::Format(
		RDFormatProfile::OverlayMachineSignature,
		RDFormatProfile::OverlayMachineSignatureCount);

	m_StaticMaxChars = _MeasureMaxLineChars(entries);
	m_MaxLineChars = (m_StaticMaxChars > m_MaxLineChars) ? m_StaticMaxChars : m_MaxLineChars;

	for (size_t i = 0; i < entries.size() && i < m_StaticLineCount; i++) {
		std::string line;
		if (entries[i].label[0] == '\0' && entries[i].value[0] == '\0') {
			line = "";
		} else {
			line = std::string(entries[i].label) + ": " + entries[i].value;
		}

		int len = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, NULL, 0);
		std::vector<WCHAR> wbuf(len);
		MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, wbuf.data(), len);

		result = m_Lines[i]->SetString(wbuf.data());
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update dynamic lines (runtime system, called every frame)
//******************************************************************************
int MTDiagOverlay11::_UpdateDynamicLines()
{
	int result = 0;

	auto entries = RDDiagManager::Format(
		RDFormatProfile::OverlayRuntime,
		RDFormatProfile::OverlayRuntimeCount);

	unsigned long dynamicMax = _MeasureMaxLineChars(entries);
	unsigned long newMax = (m_StaticMaxChars > dynamicMax) ? m_StaticMaxChars : dynamicMax;
	if (newMax > m_MaxLineChars) {
		m_MaxLineChars = newMax;
	}

	for (size_t i = 0; i < entries.size() && i < m_DynamicLineCount; i++) {
		if (entries[i].label[0] == '\0') {
			result = m_Lines[m_StaticLineCount + i]->SetString(L"");
			if (result != 0) goto EXIT;
			continue;
		}

		std::string line = std::string(entries[i].label) + ": " + entries[i].value;

		int len = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, NULL, 0);
		std::vector<WCHAR> wbuf(len);
		MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, wbuf.data(), len);

		result = m_Lines[m_StaticLineCount + i]->SetString(wbuf.data());
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}
