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

#pragma once

#include "MTSceneComponent11.h"
#include "MTNoteTracker.h"
#include "MTStaticCaption11.h"
#include "MTDynamicCaption11.h"
#include <directxtk/SimpleMath.h>
#include <string>

#define MTDASHBOARDLIVE11_FONTNAME  L"MS Gothic"
#define MTDASHBOARDLIVE11_FONTSIZE  (40)
#define MTDASHBOARDLIVE11_COUNTER_CHARS  L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:/%[] "
#define MTDASHBOARDLIVE11_COUNTER_SIZE  (40)
#define MTDASHBOARDLIVE11_FRAMESIZE  (5.0f)
#define MTDASHBOARDLIVE11_DEFAULT_MAGRATE  (0.5f)


//******************************************************************************
// Live monitor dashboard renderer
//******************************************************************************
class MTDashboardLive11 : public MTSceneComponent11, public IMTNoteTrackerListener
{
public:

	MTDashboardLive11();
	virtual ~MTDashboardLive11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName, HWND hWnd);
	void Release();

	int Draw(ID3D11DeviceContext* pContext,
	         unsigned int screenWidth, unsigned int screenHeight);

	void SetMonitoringStatus(bool isMonitoring);
	int SetMIDIINDeviceName(const TCHAR* pName);
	void Reset() override;
	void OnWindowResize();

	// IMTNoteTrackerListener
	void OnNoteActivate(const NoteData& note, unsigned long index) override;
	void OnNoteDeactivate(const NoteData& note, unsigned long index) override;
	void OnReset() override;

private:

	HWND m_hWnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	MTStaticCaption11 m_Title;
	MTDynamicCaption11 m_Counter;
	UINT m_Dpi;
	std::wstring m_TitleText;

	float m_PosCounterX;
	float m_PosCounterY;

	bool m_isMonitoring;
	unsigned long m_NoteCount;

	DirectX::SimpleMath::Color m_CaptionColor;

	UINT _GetDpi();
	unsigned long _GetScaledFontSize();
	int _RecreateCaptions();
	int _GetCounterStr(WCHAR* pStr, unsigned long bufSize);
	int _LoadConfFile(const TCHAR* pSceneName);
};
