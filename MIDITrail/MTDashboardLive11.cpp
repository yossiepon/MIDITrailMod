//******************************************************************************
//
// MIDITrail / MTDashboardLive11
//
// Live monitor dashboard renderer.
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "MTDashboardLive11.h"

using namespace YNBaseLib;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTDashboardLive11::MTDashboardLive11()
{
	m_hWnd = NULL;
	m_pDevice = NULL;
	m_pContext = NULL;
	m_Dpi = USER_DEFAULT_SCREEN_DPI;
	m_PosCounterX = 0.0f;
	m_PosCounterY = 0.0f;
	m_isMonitoring = false;
	m_NoteCount = 0;
	m_CaptionColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

MTDashboardLive11::~MTDashboardLive11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTDashboardLive11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		HWND hWnd
	)
{
	int result = 0;
	WCHAR counter[100];

	Release();

	m_pDevice = pDevice;
	m_pContext = pContext;
	m_hWnd = hWnd;
	m_Dpi = _GetDpi();

	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = SetMIDIINDeviceName(NULL);
	if (result != 0) goto EXIT;

	{
		unsigned long scaledFontSize = _GetScaledFontSize();

		result = m_Counter.Create(pDevice, pContext,
					MTDASHBOARDLIVE11_FONTNAME, scaledFontSize,
					MTDASHBOARDLIVE11_COUNTER_CHARS, MTDASHBOARDLIVE11_COUNTER_SIZE);
		if (result != 0) goto EXIT;
		m_Counter.SetColor(m_CaptionColor);
	}

	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;
	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;

	{
		float charH = 0.0f;
		m_Counter.GetDisplayCharSize(MTDASHBOARDLIVE11_DEFAULT_MAGRATE, NULL, &charH);
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		m_PosCounterX = MTDASHBOARDLIVE11_FRAMESIZE;
		m_PosCounterY = (float)(rect.bottom - rect.top) - charH - MTDASHBOARDLIVE11_FRAMESIZE;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTDashboardLive11::Release()
{
	m_Title.Release();
	m_Counter.Release();
}

//******************************************************************************
// Draw
//******************************************************************************
int MTDashboardLive11::Draw(
		ID3D11DeviceContext* pContext,
		unsigned int screenWidth,
		unsigned int screenHeight,
		MTSceneLayoutInfo* pLayoutInfo
	)
{
	int result = 0;
	WCHAR counter[100];

	if (!m_isEnable) goto EXIT;

	{
		float captionMag = MTDASHBOARDLIVE11_DEFAULT_MAGRATE;
		float displayW = 0.0f;
		m_Title.GetDisplaySize(captionMag, &displayW, NULL);
		float availableWidth = (float)screenWidth - MTDASHBOARDLIVE11_FRAMESIZE * 2.0f;
		if (displayW > availableWidth && displayW > 0.0f) {
			captionMag *= availableWidth / displayW;
		}

		float titleDispH = 0.0f;
		m_Title.GetDisplaySize(captionMag, NULL, &titleDispH);

		result = m_Title.Draw(pContext,
					MTDASHBOARDLIVE11_FRAMESIZE, MTDASHBOARDLIVE11_FRAMESIZE,
					captionMag, screenWidth, screenHeight);
		if (result != 0) goto EXIT;

		if (pLayoutInfo != NULL) {
			pLayoutInfo->titleAreaHeight = MTDASHBOARDLIVE11_FRAMESIZE + titleDispH;
		}
	}

	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;

	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;

	{
		float counterMag = MTDASHBOARDLIVE11_DEFAULT_MAGRATE;
		float charW = 0.0f, charH = 0.0f;
		m_Counter.GetDisplayCharSize(counterMag, &charW, &charH);
		float counterDrawnW = charW * (float)wcslen(counter);
		float availW = (float)screenWidth - MTDASHBOARDLIVE11_FRAMESIZE * 2.0f;
		if (counterDrawnW > availW && counterDrawnW > 0.0f) {
			float shrink = availW / counterDrawnW;
			counterMag *= shrink;
			charH *= shrink;
		}

		float counterY = (float)screenHeight - charH - MTDASHBOARDLIVE11_FRAMESIZE;

		result = m_Counter.Draw(pContext,
					m_PosCounterX, counterY,
					counterMag, screenWidth, screenHeight);
		if (result != 0) goto EXIT;

		if (pLayoutInfo != NULL) {
			pLayoutInfo->counterAreaHeight = charH + MTDASHBOARDLIVE11_FRAMESIZE;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get counter string
//******************************************************************************
int MTDashboardLive11::_GetCounterStr(WCHAR* pStr, unsigned long bufSize)
{
	const WCHAR* pStatus = m_isMonitoring ? L"" : L" [MONITORING OFF]";

	int eresult = swprintf_s(pStr, bufSize, L"NOTES:%08lu%s", m_NoteCount, pStatus);
	if (eresult < 0) {
		return YN_SET_ERR("Program error.", 0, 0);
	}
	return 0;
}

//******************************************************************************
// Load configuration
//******************************************************************************
int MTDashboardLive11::_LoadConfFile(const TCHAR* pSceneName)
{
	int result = 0;
	MTColorConf colorConf;
	MTColorPalette colorPalette;
	Color color;

	result = colorConf.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	colorConf.GetSelectedColorPalette(&colorPalette);
	colorPalette.GetCounterColor(&color);
	m_CaptionColor = color;

EXIT:;
	return result;
}

//******************************************************************************
// Various setters
//******************************************************************************
void MTDashboardLive11::SetMonitoringStatus(bool isMonitoring)
{
	m_isMonitoring = isMonitoring;
}

void MTDashboardLive11::OnNoteActivate(const NoteData& note, unsigned long index)
{
	if (index >= m_NoteCount) {
		m_NoteCount = index + 1;
	}
}

void MTDashboardLive11::OnNoteDeactivate(const NoteData& note, unsigned long index)
{
}

void MTDashboardLive11::OnReset()
{
	m_NoteCount = 0;
}

void MTDashboardLive11::Reset()
{
	m_isMonitoring = false;
	m_NoteCount = 0;
}

//******************************************************************************
// Window resize notification
//******************************************************************************
void MTDashboardLive11::OnWindowResize()
{
	UINT newDpi = _GetDpi();
	if (newDpi != m_Dpi) {
		m_Dpi = newDpi;
		_RecreateCaptions();
	}
}

//******************************************************************************
// MIDI IN device name
//******************************************************************************
int MTDashboardLive11::SetMIDIINDeviceName(const TCHAR* pName)
{
	int result = 0;
	WCHAR titleStr[300];

	WCHAR devNameW[256];
	if (pName != NULL && _tcslen(pName) > 0) {
		size_t converted = 0;
		mbstowcs_s(&converted, devNameW, 256, pName, _TRUNCATE);
		swprintf_s(titleStr, 300, L"MIDI IN: %s", devNameW);
	}
	else {
		swprintf_s(titleStr, 300, L"MIDI IN: (none)");
	}

	m_TitleText = titleStr;
	m_Title.Release();

	if (m_pDevice != NULL && m_pContext != NULL) {
		result = m_Title.Create(m_pDevice, m_pContext,
					MTDASHBOARDLIVE11_FONTNAME, _GetScaledFontSize(),
					titleStr);
		if (result != 0) goto EXIT;
		m_Title.SetColor(m_CaptionColor);
	}

EXIT:;
	return result;
}

//******************************************************************************
// DPI helpers
//******************************************************************************
UINT MTDashboardLive11::_GetDpi()
{
	if (m_hWnd != NULL) {
		return GetDpiForWindow(m_hWnd);
	}
	return USER_DEFAULT_SCREEN_DPI;
}

unsigned long MTDashboardLive11::_GetScaledFontSize()
{
	return MulDiv(MTDASHBOARDLIVE11_FONTSIZE, m_Dpi, USER_DEFAULT_SCREEN_DPI);
}

int MTDashboardLive11::_RecreateCaptions()
{
	int result = 0;
	unsigned long scaledFontSize = _GetScaledFontSize();

	m_Title.Release();
	result = m_Title.Create(m_pDevice, m_pContext,
				MTDASHBOARDLIVE11_FONTNAME, scaledFontSize,
				m_TitleText.c_str());
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

	m_Counter.Release();
	result = m_Counter.Create(m_pDevice, m_pContext,
				MTDASHBOARDLIVE11_FONTNAME, scaledFontSize,
				MTDASHBOARDLIVE11_COUNTER_CHARS, MTDASHBOARDLIVE11_COUNTER_SIZE);
	if (result != 0) goto EXIT;
	m_Counter.SetColor(m_CaptionColor);

EXIT:;
	return result;
}
