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
	m_isMonitorMode = false;
	m_isMonitoring = false;
	m_MIDIINDevName[0] = L'\0';
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
	SMTrack track;
	SMNoteList noteList;
	WCHAR counter[100];

	Release();

	m_hWnd = hWnd;

	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	if (pSeqData != NULL) {
		// Playback mode: read title, filename, totals from sequence data
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

		result = pSeqData->GetMergedTrack(&track);
		if (result != 0) goto EXIT;
		result = track.GetNoteList(&noteList);
		if (result != 0) goto EXIT;
		m_NoteNum = noteList.GetSize();
	}
	else {
		// Live mode: default captions
		title = L" ";
		fileName = L" ";
	}

	// Title caption
	result = m_Title.Create(pDevice, pContext,
				MTDASHBOARD11_FONTNAME, MTDASHBOARD11_FONTSIZE,
				title.c_str());
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

	// File name caption
	result = m_FileName.Create(pDevice, pContext,
				MTDASHBOARD11_FONTNAME, MTDASHBOARD11_FONTSIZE,
				fileName.c_str());
	if (result != 0) goto EXIT;
	m_FileName.SetColor(m_CaptionColor);

	// Counter caption
	result = m_Counter.Create(pDevice, pContext,
				MTDASHBOARD11_FONTNAME, MTDASHBOARD11_FONTSIZE,
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

	if (m_isEnableFileName) {
		result = m_FileName.Draw(pContext,
					MTDASHBOARD11_FRAMESIZE, MTDASHBOARD11_FRAMESIZE,
					m_CounterMag, screenWidth, screenHeight);
		if (result != 0) goto EXIT;
	}
	else {
		result = m_Title.Draw(pContext,
					MTDASHBOARD11_FRAMESIZE, MTDASHBOARD11_FRAMESIZE,
					m_CounterMag, screenWidth, screenHeight);
		if (result != 0) goto EXIT;
	}

	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;

	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;

	result = m_Counter.Draw(pContext,
				m_PosCounterX, m_PosCounterY,
				m_CounterMag, screenWidth, screenHeight);
	if (result != 0) goto EXIT;

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

	if (!GetClientRect(m_hWnd, &rect)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	unsigned long cw = rect.right - rect.left;
	unsigned long ch = rect.bottom - rect.top;

	unsigned long th = 0, tw = 0;
	m_Counter.GetTextureSize(&th, &tw);

	unsigned long charWidth = 0;
	if (wcslen(MTDASHBOARD11_COUNTER_CHARS) > 0) {
		charWidth = tw / (unsigned long)wcslen(MTDASHBOARD11_COUNTER_CHARS);
	}
	unsigned long captionWidth = charWidth * MTDASHBOARD11_COUNTER_SIZE;

	if (((cw - (unsigned long)(MTDASHBOARD11_FRAMESIZE * 2)) < captionWidth) && (tw > 0)) {
		float newMag = (float)(cw - (unsigned long)(MTDASHBOARD11_FRAMESIZE * 2)) / (float)captionWidth;
		if (m_CounterMag > newMag) {
			m_CounterMag = newMag;
		}
	}

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

	if (m_isMonitorMode) {
		const WCHAR* statusStr = m_isMonitoring ? L"" : L" [MONITORING OFF]";
		eresult = swprintf_s(pStr, bufSize, L"NOTES:%08lu%s", m_NoteCount, statusStr);
		if (eresult < 0) {
			return YN_SET_ERR("Program error.", 0, 0);
		}
		return 0;
	}

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
	m_CounterMag = MTDASHBOARD11_DEFAULT_MAGRATE;
	_GetCounterPos(&m_PosCounterX, &m_PosCounterY);
}

//******************************************************************************
// Live monitor mode
//******************************************************************************
void MTDashboard11::SetMonitorMode(
		bool isMonitor,
		const TCHAR* pMIDIINDevName
	)
{
	m_isMonitorMode = isMonitor;
	m_isMonitoring = isMonitor;
	m_NoteCount = 0;
	SetMIDIINDeviceName(pMIDIINDevName);
}

void MTDashboard11::SetMonitoringStatus(bool isMonitoring)
{
	m_isMonitoring = isMonitoring;
}

int MTDashboard11::SetMIDIINDeviceName(const TCHAR* pName)
{
	int result = 0;

	if (pName != NULL && _tcslen(pName) > 0) {
		size_t converted = 0;
		mbstowcs_s(&converted, m_MIDIINDevName, 256, pName, _TRUNCATE);
	}
	else {
		m_MIDIINDevName[0] = L'\0';
	}

	m_Title.Release();

	WCHAR titleStr[300];
	if (m_MIDIINDevName[0] != L'\0') {
		swprintf_s(titleStr, 300, L"MIDI IN: %s", m_MIDIINDevName);
	}
	else {
		swprintf_s(titleStr, 300, L" ");
	}

	if (m_pDevice != NULL && m_pContext != NULL) {
		result = m_Title.Create(m_pDevice, m_pContext,
					MTDASHBOARD11_FONTNAME, MTDASHBOARD11_FONTSIZE,
					titleStr);
		if (result != 0) goto EXIT;
		m_Title.SetColor(m_CaptionColor);
	}

EXIT:;
	return result;
}
