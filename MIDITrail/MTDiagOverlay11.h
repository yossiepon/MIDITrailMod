//******************************************************************************
//
// MIDITrail / MTDiagOverlay11
//
// Diagnostic information overlay renderer.
// Displays RTDiagLib metrics (machine signature + runtime system) on screen.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "MTDynamicCaption11.h"
#include "MTSceneLayoutInfo.h"
#include "DXPrimitive11.h"
#include "DXTexture11.h"
#include <directxtk/SimpleMath.h>
#include <vector>
#include <string>

struct RDFormattedEntry;

#define MTDIAGOVERLAY11_DPI_SCALING   (false)
#define MTDIAGOVERLAY11_FONTNAME  L"MS Gothic"
#define MTDIAGOVERLAY11_FONTSIZE  (32)
#define MTDIAGOVERLAY11_MAGRATE   (1.0f)
#define MTDIAGOVERLAY11_MARGIN    (5.0f)
#define MTDIAGOVERLAY11_PADDING   (4.0f)
#define MTDIAGOVERLAY11_LINE_SPACING  (2.0f)
#define MTDIAGOVERLAY11_CAPTION_SIZE  (128)
#define MTDIAGOVERLAY11_RIGHT_MARGIN_CHARS  (3)
#define MTDIAGOVERLAY11_BG_ALPHA  (64)


//******************************************************************************
// Layout parameters (shared between background vertex update and text draw)
//******************************************************************************
struct DiagLayoutParams {
	float mag;
	float charW;
	float charH;
	float lineHeight;
	float overlayWidth;
	float overlayHeight;
	float topOffset;
	float textX;
};


//******************************************************************************
// DX11 diagnostic overlay renderer
//******************************************************************************
class MTDiagOverlay11 : public MTSceneComponent11
{
public:

	MTDiagOverlay11();
	virtual ~MTDiagOverlay11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	void Release();

	int Draw(ID3D11DeviceContext* pContext,
	         unsigned int screenWidth, unsigned int screenHeight,
	         const MTSceneLayoutInfo* pLayoutInfo = NULL);

	void Reset() override;
	void OnWindowResize();

private:

	HWND m_hWnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	UINT m_Dpi;

	unsigned long m_StaticLineCount;
	unsigned long m_DynamicLineCount;
	unsigned long m_TotalLineCount;

	std::vector<MTDynamicCaption11*> m_Lines;
	MTFontTexture11 m_SharedFontTexture;

	DXPrimitive11 m_BgPrimitive;
	ID3D11ShaderResourceView* m_pBgSRV;

	DirectX::SimpleMath::Color m_Color;

	unsigned long m_MaxLineChars;
	unsigned long m_StaticMaxChars;

	DiagLayoutParams m_CachedLayout;
	unsigned int m_CachedScreenWidth;
	unsigned int m_CachedScreenHeight;
	float m_CachedTitleAreaH;
	float m_CachedCounterAreaH;
	unsigned long m_CachedMaxLineChars;

	UINT _GetDpi();
	unsigned long _GetScaledFontSize();
	int _CreateBgTexture();
	void _CalcLayout(unsigned int screenWidth, unsigned int screenHeight,
	                 const MTSceneLayoutInfo* pLayoutInfo, DiagLayoutParams* pOut);
	int _UpdateBgVertices(const DiagLayoutParams& layout,
	                      unsigned int screenWidth, unsigned int screenHeight);
	int _DrawBackground(ID3D11DeviceContext* pContext);
	int _RecreateCaptions();
	int _UpdateStaticLines();
	int _UpdateDynamicLines();
	unsigned long _MeasureMaxLineChars(const std::vector<RDFormattedEntry>& entries);
};
