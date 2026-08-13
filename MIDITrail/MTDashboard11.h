//******************************************************************************
//
// MIDITrail / MTDashboard11
//
// Dashboard renderer.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "MTNoteTracker.h"
#include "SMIDILib.h"
#include "MTStaticCaption11.h"
#include "MTDynamicCaption11.h"
#include <directxtk/SimpleMath.h>

using namespace SMIDILib;

#define MTDASHBOARD11_FONTNAME  L"MS Gothic"
#define MTDASHBOARD11_FONTSIZE  (40)
#define MTDASHBOARD11_COUNTER_CHARS  L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:./% []"
#define MTDASHBOARD11_COUNTER_SIZE  (90)
#define MTDASHBOARD11_FRAMESIZE  (5.0f)
#define MTDASHBOARD11_DEFAULT_MAGRATE  (0.5f)


//******************************************************************************
// DX11 dashboard renderer
//******************************************************************************
class MTDashboard11 : public MTSceneComponent11, public IMTNoteTrackerListener
{
public:

	MTDashboard11();
	virtual ~MTDashboard11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName, SMSeqData* pSeqData, HWND hWnd);
	void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(ID3D11DeviceContext* pContext,
	         unsigned int screenWidth, unsigned int screenHeight);

	void SetTotalPlayTimeSec(unsigned long totalPlayTimeSec);
	void SetTempoBPM(unsigned long bpm);
	void SetBarNo(unsigned long barNo);
	void SetBarNum(unsigned long barNum);
	void SetBeat(unsigned long numerator, unsigned long denominator);
	void SetPlaySpeedRatio(unsigned long ratio);
	void SetNotesCount(unsigned long notesCount);
	void Reset() override;
	void OnWindowResize();

	// IMTNoteTrackerListener
	void OnNoteActivate(const NoteData& note, unsigned long index) override;
	void OnNoteDeactivate(const NoteData& note, unsigned long index) override;
	void OnReset() override;

	unsigned long GetPlayTimeSec();
	void SetEnableFileName(bool isEnable);

	void SetMonitorMode(bool isMonitor, const TCHAR* pMIDIINDevName);
	void SetMonitoringStatus(bool isMonitoring);
	int SetMIDIINDeviceName(const TCHAR* pName);

private:

	HWND m_hWnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	MTStaticCaption11 m_Title;
	MTStaticCaption11 m_FileName;
	MTDynamicCaption11 m_Counter;

	float m_PosCounterX;
	float m_PosCounterY;
	float m_CounterMag;

	unsigned long m_PlayTimeMSec;
	unsigned long m_TotalPlayTimeMSec;
	unsigned long m_TempoBPM;
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;
	unsigned long m_BarNo;
	unsigned long m_BarNum;
	unsigned long m_NoteCount;
	unsigned long m_NoteNum;
	unsigned long m_PlaySpeedRatio;

	unsigned long m_TempoBPMOnStart;
	unsigned long m_BeatNumeratorOnStart;
	unsigned long m_BeatDenominatorOnStart;

	DirectX::SimpleMath::Color m_CaptionColor;
	bool m_isEnableFileName;
	bool m_isMonitorMode;
	bool m_isMonitoring;
	WCHAR m_MIDIINDevName[256];

	int _GetCounterPos(float* pX, float* pY);
	int _GetCounterStr(WCHAR* pStr, unsigned long bufSize);
	int _LoadConfFile(const TCHAR* pSceneName);
};
