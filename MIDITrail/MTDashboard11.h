//******************************************************************************
//
// MIDITrail / MTDashboard11
//
// DX11 dashboard (M4): the original MIDITrail on-screen info, matching the DX9
// MTDashboard layout/content -- file name at the top-left, and a multi-line
// counter (Time cur/total, BPM, Beat, Bars cur/total, Notes cur/total) at the
// bottom-left. Rendered via the GDI font-to-bitmap path (MTFont2Bmp) uploaded
// to D3D11 textures and drawn as screen-space quads.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <tchar.h>
#include "DXPrimitive11.h"
#include "MTFont2Bmp.h"


// 1 file-name line (top) + up to 6 counter lines (bottom: Time/BPM/Beat/Bars/Notes/Speed)
#define MTDB11_MAX_LINES  (8)


class MTDashboard11
{
public:
	MTDashboard11();
	virtual ~MTDashboard11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Release();

	// on load
	void SetFileName(const char* pName);
	void SetTotals(unsigned long totalTimeMSec, unsigned long totalBars, unsigned long totalNotes);
	void Reset();

	// live monitor: show "MIDI IN: <dev>" (top) + "NOTES:<count>" (bottom) instead
	// of the playback time/bpm/beat/bar counter (matches DX9 MTDashboardLive).
	void SetMonitorMode(bool isMonitor, const char* pMidiInDevName);

	// during playback
	void SetPlayTimeMSec(unsigned long ms);
	void SetCurBar(long barNo);
	void SetCurNotes(unsigned long notes);
	void SetTempo(unsigned long bpm);
	void SetBeat(int numerator, int denominator);
	void SetPlaySpeedRatio(unsigned long ratio);

	// dashboard text color (DX9 read [Color] CaptionRGBA; default translucent grey).
	// Components are 0..1 floats (as parsed by DXColorUtil::MakeColorFromHexRGBA).
	void SetTextColor(float r, float g, float b, float a);

	int DrawDX11(ID3D11DeviceContext* pContext, unsigned int screenW, unsigned int screenH);

	bool IsReady() { return m_Ready; }

private:
	enum Anchor { AnchorTop, AnchorBottom };
	struct Line {
		char text[256];
		Anchor anchor;
		ID3D11ShaderResourceView* pSRV;
		DXPrimitive11 prim;
		unsigned long texW;
		unsigned long texH;
		bool dirty;
	};

	ID3D11Device* m_pDevice;
	MTFont2Bmp m_Font;
	Line m_Lines[MTDB11_MAX_LINES];
	bool m_Ready;

	// counter source values
	unsigned long m_PlayTimeMSec;
	unsigned long m_TotalTimeMSec;
	unsigned long m_TempoBPM;
	int m_BeatNum;
	int m_BeatDenom;
	long m_CurBar;
	unsigned long m_TotalBars;
	unsigned long m_CurNotes;
	unsigned long m_TotalNotes;
	unsigned long m_PlaySpeedRatio;

	// dashboard text color (DX9 [Color] CaptionRGBA). RGB packed 0x00RRGGBB; the
	// alpha modulates the glyph coverage. Defaults to the shipped AAAAAAAA (grey).
	unsigned long m_FontColorRGB;
	unsigned char m_FontAlpha;

	// live monitor display
	bool m_MonitorMode;
	char m_MIDIINDevName[256];

	void _SetLineText(int idx, Anchor anchor, const char* pText);
	void _RebuildCounter();
	int  _RegenLine(int idx);
};
