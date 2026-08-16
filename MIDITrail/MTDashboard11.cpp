//******************************************************************************
//
// MIDITrail / MTDashboard11
//
// Dashboard renderer.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "MTDashboard11.h"
#include <string>

using namespace YNBaseLib;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTDashboard11::MTDashboard11()
{
	m_hWnd = NULL;
	m_pDevice = NULL;
	m_pContext = NULL;
	m_Dpi = USER_DEFAULT_SCREEN_DPI;
	m_PosCounterX = 0.0f;
	m_PosCounterY = 0.0f;
	m_CounterMag = MTDASHBOARD11_DEFAULT_MAGRATE;

	m_PlayTimeMSec = 0;
	m_TotalPlayTimeMSec = 0;
	m_TempoBPM = 0;
	m_BeatNumerator = 0;
	m_BeatDenominator = 0;
	m_BarNo = 0;
	m_BarNum = 0;
	m_NoteCount = 0;
	m_NoteNum = 0;
	m_PlaySpeedRatio = 100;

	m_TempoBPMOnStart = 0;
	m_BeatNumeratorOnStart = 0;
	m_BeatDenominatorOnStart = 0;

	m_CaptionColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_isEnableFileName = false;
}

MTDashboard11::~MTDashboard11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTDashboard11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		HWND hWnd
	)
{
	int result = 0;
	std::wstring title;
	std::wstring fileName;

	m_pDevice = pDevice;
	m_pContext = pContext;
	WCHAR counter[100];
	// Declared here (before the first goto) for C++20 compatibility with the goto-based error handling below.
	unsigned long scaledFontSize = 0;

	Release();

	m_hWnd = hWnd;
	m_Dpi = _GetDpi();

	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	title = pSeqData->GetTitle();
	if (title.size() == 0) {
		title += L" ";
	}
	fileName = pSeqData->GetFileName();
	if (fileName.size() == 0) {
		fileName += L" ";
	}

	SetTotalPlayTimeSec(pSeqData->GetTotalPlayTime());
	SetTempoBPM(pSeqData->GetTempoBPM());
	m_TempoBPMOnStart = pSeqData->GetTempoBPM();
	SetBeat(pSeqData->GetBeatNumerator(), pSeqData->GetBeatDenominator());
	m_BeatNumeratorOnStart = pSeqData->GetBeatNumerator();
	m_BeatDenominatorOnStart = pSeqData->GetBeatDenominator();
	SetBarNo(1);
	SetBarNum(pSeqData->GetBarNum());

	// Note count is set later via SetNoteNum() after NoteTracker is built.
	// Previously called GetMergedTrack+GetNoteList here, which duplicated
	// heavy processing (2+ seconds for 6M+ notes) just to get the count.

	m_TitleText = title;
	m_FileNameText = fileName;

	scaledFontSize = _GetScaledFontSize();

	// Title caption
	result = m_Title.Create(pDevice, pContext,
				MTDASHBOARD11_FONTNAME, scaledFontSize,
				title.c_str());
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

	// File name caption
	result = m_FileName.Create(pDevice, pContext,
				MTDASHBOARD11_FONTNAME, scaledFontSize,
				fileName.c_str());
	if (result != 0) goto EXIT;
	m_FileName.SetColor(m_CaptionColor);

	// Counter caption
	result = m_Counter.Create(pDevice, pContext,
				MTDASHBOARD11_FONTNAME, scaledFontSize,
				MTDASHBOARD11_COUNTER_CHARS, MTDASHBOARD11_COUNTER_SIZE);
	if (result != 0) goto EXIT;
	m_Counter.SetColor(m_CaptionColor);

	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;
	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;

	result = _GetCounterPos(&m_PosCounterX, &m_PosCounterY);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTDashboard11::Release()
{
	m_Title.Release();
	m_FileName.Release();
	m_Counter.Release();
}

//******************************************************************************
// Draw
//******************************************************************************
int MTDashboard11::Draw(
		ID3D11DeviceContext* pContext,
		unsigned int screenWidth,
		unsigned int screenHeight
	)
{
	int result = 0;
	WCHAR counter[100];

	if (!m_isEnable) goto EXIT;

	{
		MTStaticCaption11& caption = m_isEnableFileName ? m_FileName : m_Title;
		unsigned long texH = 0, texW = 0;
		caption.GetTextureSize(&texH, &texW);

		float captionMag = MTDASHBOARD11_DEFAULT_MAGRATE;
		float availableWidth = (float)screenWidth - MTDASHBOARD11_FRAMESIZE * 2.0f;
		float captionWidth = (float)texW * captionMag;
		if (captionWidth > availableWidth && texW > 0) {
			captionMag = availableWidth / (float)texW;
		}

		result = caption.Draw(pContext,
					MTDASHBOARD11_FRAMESIZE, MTDASHBOARD11_FRAMESIZE,
					captionMag, screenWidth, screenHeight);
		if (result != 0) goto EXIT;
	}

	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;

	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;

	{
		unsigned long cTexH = 0, cTexW = 0;
		m_Counter.GetTextureSize(&cTexH, &cTexW);

		unsigned long charWidth = 0;
		if (wcslen(MTDASHBOARD11_COUNTER_CHARS) > 0) {
			charWidth = cTexW / (unsigned long)wcslen(MTDASHBOARD11_COUNTER_CHARS);
		}
		float counterMag = m_CounterMag;
		float counterDrawnW = (float)(charWidth * (unsigned long)wcslen(counter)) * counterMag;
		float availW = (float)screenWidth - MTDASHBOARD11_FRAMESIZE * 2.0f;
		if (counterDrawnW > availW && counterDrawnW > 0.0f) {
			counterMag *= availW / counterDrawnW;
		}

		float counterY = (float)screenHeight - (float)cTexH * counterMag - MTDASHBOARD11_FRAMESIZE;

		result = m_Counter.Draw(pContext,
					m_PosCounterX, counterY,
					counterMag, screenWidth, screenHeight);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get counter display position
//******************************************************************************
int MTDashboard11::_GetCounterPos(float* pX, float* pY)
{
	int result = 0;
	RECT rect;
	// Declared here (before the first goto) for C++20 compatibility with the goto-based error handling below.
	unsigned long cw = 0;
	unsigned long ch = 0;
	unsigned long th = 0, tw = 0;

	if (!GetClientRect(m_hWnd, &rect)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	cw = rect.right - rect.left;
	ch = rect.bottom - rect.top;

	m_Counter.GetTextureSize(&th, &tw);

	*pX = MTDASHBOARD11_FRAMESIZE;
	*pY = (float)ch - ((float)th * m_CounterMag) - MTDASHBOARD11_FRAMESIZE;

EXIT:;
	return result;
}

//******************************************************************************
// Get counter string
//******************************************************************************
int MTDashboard11::_GetCounterStr(WCHAR* pStr, unsigned long bufSize)
{
	int eresult = 0;
	WCHAR spdstr[16] = {0};

	eresult = swprintf_s(
				pStr, bufSize,
				L"TIME:%02lu:%02lu.%03lu/%02lu:%02lu.%03lu BPM:%03lu BEAT:%lu/%lu BAR:%03lu/%03lu NOTES:%05lu/%05lu",
				m_PlayTimeMSec / 60000,
				(m_PlayTimeMSec % 60000) / 1000,
				m_PlayTimeMSec % 1000,
				m_TotalPlayTimeMSec / 60000,
				(m_TotalPlayTimeMSec % 60000) / 1000,
				m_TotalPlayTimeMSec % 1000,
				m_TempoBPM,
				m_BeatNumerator,
				m_BeatDenominator,
				m_BarNo,
				m_BarNum,
				m_NoteCount,
				m_NoteNum);

	if (eresult < 0) {
		return YN_SET_ERR("Program error.", 0, 0);
	}

	if (m_PlaySpeedRatio != 100) {
		eresult = swprintf_s(spdstr, 16, L" SPEED:%03lu%%", m_PlaySpeedRatio);
		if (eresult < 0) {
			return YN_SET_ERR("Program error.", 0, 0);
		}
		wcscat_s(pStr, bufSize, spdstr);
	}

	return 0;
}

//******************************************************************************
// Load configuration
//******************************************************************************
int MTDashboard11::_LoadConfFile(const TCHAR* pSceneName)
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
int MTDashboard11::Update(const MTSceneUpdateContext& ctx) { m_PlayTimeMSec = ctx.playTimeMSec; return 0; }
void MTDashboard11::SetTotalPlayTimeSec(unsigned long t)       { m_TotalPlayTimeMSec = t; }
void MTDashboard11::SetTempoBPM(unsigned long bpm)             { m_TempoBPM = bpm; }
void MTDashboard11::SetBarNo(unsigned long barNo)              { m_BarNo = barNo; }
void MTDashboard11::SetBarNum(unsigned long barNum)            { m_BarNum = barNum; }
void MTDashboard11::SetBeat(unsigned long n, unsigned long d)  { m_BeatNumerator = n; m_BeatDenominator = d; }
void MTDashboard11::SetPlaySpeedRatio(unsigned long ratio)     { m_PlaySpeedRatio = ratio; }
void MTDashboard11::SetNotesCount(unsigned long c)             { m_NoteCount = c; }
void MTDashboard11::SetNoteNum(unsigned long noteNum)          { m_NoteNum = noteNum; }
unsigned long MTDashboard11::GetPlayTimeSec()                  { return m_PlayTimeMSec; }
void MTDashboard11::SetEnableFileName(bool e)                  { m_isEnableFileName = e; }

void MTDashboard11::OnNoteActivate(const NoteData& note, unsigned long index)
{
	if (index >= m_NoteCount) {
		m_NoteCount = index + 1;
	}
}

void MTDashboard11::OnNoteDeactivate(const NoteData& note, unsigned long index)
{
}

void MTDashboard11::OnReset()
{
	m_NoteCount = 0;
}

void MTDashboard11::Reset()
{
	m_PlayTimeMSec = 0;
	m_TempoBPM = m_TempoBPMOnStart;
	m_BeatNumerator = m_BeatNumeratorOnStart;
	m_BeatDenominator = m_BeatDenominatorOnStart;
	m_BarNo = 1;
	m_NoteCount = 0;
}

//******************************************************************************
// Window resize notification
//******************************************************************************
void MTDashboard11::OnWindowResize()
{
	UINT newDpi = _GetDpi();
	if (newDpi != m_Dpi) {
		m_Dpi = newDpi;
		_RecreateCaptions();
	}

	m_CounterMag = MTDASHBOARD11_DEFAULT_MAGRATE;
	_GetCounterPos(&m_PosCounterX, &m_PosCounterY);
}

//******************************************************************************
// DPI helpers
//******************************************************************************
UINT MTDashboard11::_GetDpi()
{
	if (m_hWnd != NULL) {
		return GetDpiForWindow(m_hWnd);
	}
	return USER_DEFAULT_SCREEN_DPI;
}

unsigned long MTDashboard11::_GetScaledFontSize()
{
	return MulDiv(MTDASHBOARD11_FONTSIZE, m_Dpi, USER_DEFAULT_SCREEN_DPI);
}

int MTDashboard11::_RecreateCaptions()
{
	int result = 0;
	unsigned long scaledFontSize = _GetScaledFontSize();

	m_Title.Release();
	result = m_Title.Create(m_pDevice, m_pContext,
				MTDASHBOARD11_FONTNAME, scaledFontSize,
				m_TitleText.c_str());
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

	m_FileName.Release();
	result = m_FileName.Create(m_pDevice, m_pContext,
				MTDASHBOARD11_FONTNAME, scaledFontSize,
				m_FileNameText.c_str());
	if (result != 0) goto EXIT;
	m_FileName.SetColor(m_CaptionColor);

	m_Counter.Release();
	result = m_Counter.Create(m_pDevice, m_pContext,
				MTDASHBOARD11_FONTNAME, scaledFontSize,
				MTDASHBOARD11_COUNTER_CHARS, MTDASHBOARD11_COUNTER_SIZE);
	if (result != 0) goto EXIT;
	m_Counter.SetColor(m_CaptionColor);

EXIT:;
	return result;
}
