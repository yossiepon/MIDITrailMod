//******************************************************************************
//
// MIDITrail / MTDashboard11
//
// DX11 dashboard (M4) - matches the original MTDashboard layout/content
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTDashboard11.h"
#include <stdlib.h>
#include <stdio.h>

using namespace YNBaseLib;
using namespace DirectX;

#define MTDB11_FONT_RGB   (0x00FFFFFF)   // white text
#define MTDB11_FONT_SIZE  (40)           // original MTDASHBOARD_FONTSIZE (rendered hi-res, scaled by mag)
#define MTDB11_DEF_MAG    (0.5f)         // original MTDASHBOARD_DEFAULT_MAGRATE
#define MTDB11_MARGIN     (4)            // screen pixels from the frame
#define MTDB11_LINE_GAP   (1)

// line layout: 0 = file name (top); 1..6 = counter (bottom)
#define MTDB11_LINE_NAME    (0)
#define MTDB11_LINE_TIME    (1)
#define MTDB11_LINE_BPM     (2)
#define MTDB11_LINE_BEAT    (3)
#define MTDB11_LINE_BARS    (4)
#define MTDB11_LINE_NOTES   (5)
#define MTDB11_LINE_SPEED   (6)


MTDashboard11::MTDashboard11()
{
	m_pDevice = NULL;
	m_Ready = false;
	m_PlayTimeMSec = 0;
	m_TotalTimeMSec = 0;
	m_TempoBPM = 0;
	m_BeatNum = 0;
	m_BeatDenom = 0;
	m_CurBar = 0;
	m_TotalBars = 0;
	m_CurNotes = 0;
	m_TotalNotes = 0;
	m_PlaySpeedRatio = 100;
	m_FontColorRGB = MTDB11_FONT_RGB;   // overridden per-scene from [Color] CaptionRGBA
	m_FontAlpha = 0xFF;
	m_MonitorMode = false;
	m_MIDIINDevName[0] = '\0';
	for (int i = 0; i < MTDB11_MAX_LINES; i++) {
		m_Lines[i].text[0] = '\0';
		m_Lines[i].anchor = AnchorTop;
		m_Lines[i].pSRV = NULL;
		m_Lines[i].texW = 0;
		m_Lines[i].texH = 0;
		m_Lines[i].dirty = false;
	}
}

MTDashboard11::~MTDashboard11()
{
	Release();
}

void MTDashboard11::Release()
{
	for (int i = 0; i < MTDB11_MAX_LINES; i++) {
		if (m_Lines[i].pSRV != NULL) { m_Lines[i].pSRV->Release(); m_Lines[i].pSRV = NULL; }
		m_Lines[i].prim.Release();
		m_Lines[i].texW = 0;
		m_Lines[i].texH = 0;
		m_Lines[i].dirty = false;
		m_Lines[i].text[0] = '\0';
	}
	m_Ready = false;
}

void MTDashboard11::Reset()
{
	m_PlayTimeMSec = 0; m_TempoBPM = 0; m_BeatNum = 0; m_BeatDenom = 0;
	m_CurBar = 0; m_CurNotes = 0; m_PlaySpeedRatio = 100;
	_RebuildCounter();
}

int MTDashboard11::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	int result = 0;
	int i = 0;

	Release();
	m_pDevice = pDevice;

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_Font.SetFont(_T("MS Gothic"), MTDB11_FONT_SIZE, false);
	if (result != 0) goto EXIT;

	for (i = 0; i < MTDB11_MAX_LINES; i++) {
		unsigned long* pi = NULL;
		result = m_Lines[i].prim.CreateVertexBuffer(pDevice, 4);
		if (result != 0) goto EXIT;
		result = m_Lines[i].prim.CreateIndexBuffer(pDevice, 6);
		if (result != 0) goto EXIT;
		result = m_Lines[i].prim.LockIndex(pContext, &pi);
		if (result != 0) goto EXIT;
		pi[0] = 0; pi[1] = 1; pi[2] = 2; pi[3] = 2; pi[4] = 1; pi[5] = 3;
		m_Lines[i].prim.UnlockIndex(pContext);
		m_Lines[i].prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);
	}

	m_Ready = true;

EXIT:;
	return result;
}

void MTDashboard11::_SetLineText(int idx, Anchor anchor, const char* pText)
{
	if ((idx < 0) || (idx >= MTDB11_MAX_LINES)) return;
	if (pText == NULL) pText = "";
	m_Lines[idx].anchor = anchor;
	if (strcmp(m_Lines[idx].text, pText) == 0) return;
	strncpy_s(m_Lines[idx].text, sizeof(m_Lines[idx].text), pText, _TRUNCATE);
	m_Lines[idx].dirty = true;
}

void MTDashboard11::SetFileName(const char* pName)
{
	_SetLineText(MTDB11_LINE_NAME, AnchorTop, pName);
}

void MTDashboard11::SetTotals(unsigned long totalTimeMSec, unsigned long totalBars, unsigned long totalNotes)
{
	m_TotalTimeMSec = totalTimeMSec;
	m_TotalBars = totalBars;
	m_TotalNotes = totalNotes;
	_RebuildCounter();
}

void MTDashboard11::_RebuildCounter()
{
	char buf[256];

	// live monitor: just the played-note count (DX9 MTDashboardLive). The
	// playback time/bpm/beat/bar are meaningless with no song.
	if (m_MonitorMode) {
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "NOTES:%08lu", m_CurNotes);
		_SetLineText(MTDB11_LINE_TIME, AnchorBottom, buf);
		return;
	}

	// single-line counter, matching the original MTDashboard format exactly
	_snprintf_s(buf, sizeof(buf), _TRUNCATE,
			"TIME:%02lu:%02lu.%03lu/%02lu:%02lu.%03lu BPM:%03lu BEAT:%d/%d BAR:%03ld/%03lu NOTES:%05lu/%05lu",
			m_PlayTimeMSec / 60000, (m_PlayTimeMSec % 60000) / 1000, m_PlayTimeMSec % 1000,
			m_TotalTimeMSec / 60000, (m_TotalTimeMSec % 60000) / 1000, m_TotalTimeMSec % 1000,
			m_TempoBPM, m_BeatNum, m_BeatDenom, m_CurBar, m_TotalBars, m_CurNotes, m_TotalNotes);

	if (m_PlaySpeedRatio != 100) {
		char sp[24];
		_snprintf_s(sp, sizeof(sp), _TRUNCATE, " SPEED:%03lu%%", m_PlaySpeedRatio);
		strncat_s(buf, sizeof(buf), sp, _TRUNCATE);
	}

	_SetLineText(MTDB11_LINE_TIME, AnchorBottom, buf);   // line 1 = the whole counter
}

//******************************************************************************
// live monitor mode: top line = "MIDI IN: <dev>", counter = note count
//******************************************************************************
void MTDashboard11::SetMonitorMode(bool isMonitor, const char* pMidiInDevName)
{
	m_MonitorMode = isMonitor;
	if (pMidiInDevName == NULL) pMidiInDevName = "";
	strncpy_s(m_MIDIINDevName, sizeof(m_MIDIINDevName), pMidiInDevName, _TRUNCATE);

	if (isMonitor) {
		char top[300];
		_snprintf_s(top, sizeof(top), _TRUNCATE, "MIDI IN: %s", m_MIDIINDevName);
		_SetLineText(MTDB11_LINE_NAME, AnchorTop, top);
	}
	_RebuildCounter();
}

void MTDashboard11::SetPlayTimeMSec(unsigned long ms)      { m_PlayTimeMSec = ms; _RebuildCounter(); }
void MTDashboard11::SetCurBar(long barNo)                  { m_CurBar = barNo;    _RebuildCounter(); }
void MTDashboard11::SetCurNotes(unsigned long notes)       { m_CurNotes = notes;  _RebuildCounter(); }
void MTDashboard11::SetTempo(unsigned long bpm)            { m_TempoBPM = bpm;    _RebuildCounter(); }
void MTDashboard11::SetBeat(int num, int denom)            { m_BeatNum = num; m_BeatDenom = denom; _RebuildCounter(); }
void MTDashboard11::SetPlaySpeedRatio(unsigned long ratio) { m_PlaySpeedRatio = ratio; _RebuildCounter(); }

void MTDashboard11::SetTextColor(float r, float g, float b, float a)
{
	// DX9 parity: the original dashboard took the text COLOUR from CaptionRGBA's RGB
	// (vertex diffuse) but its ALPHA from the glyph coverage only - the caption's own
	// alpha byte was ignored (D3DTSS_ALPHAARG1 = D3DTA_TEXTURE). So we apply the RGB
	// and keep the glyphs fully opaque; this also avoids confs with a 00 alpha byte
	// (e.g. "AAAAAA00") rendering the text invisible.
	(void)a;
	#define MTDB11_CLAMP01(v) (((v) < 0.0f) ? 0.0f : (((v) > 1.0f) ? 1.0f : (v)))
	unsigned long cr = (unsigned long)(MTDB11_CLAMP01(r) * 255.0f + 0.5f);
	unsigned long cg = (unsigned long)(MTDB11_CLAMP01(g) * 255.0f + 0.5f);
	unsigned long cb = (unsigned long)(MTDB11_CLAMP01(b) * 255.0f + 0.5f);
	#undef MTDB11_CLAMP01
	unsigned long rgb = (cr << 16) | (cg << 8) | cb;
	if ((rgb == m_FontColorRGB) && (m_FontAlpha == 0xFF)) return;   // unchanged
	m_FontColorRGB = rgb;
	m_FontAlpha = 0xFF;   // glyph coverage stays fully opaque (DX9 ignores caption alpha)
	// existing line textures bake in the old color; force a regen on the next draw
	for (int i = 0; i < MTDB11_MAX_LINES; i++) m_Lines[i].dirty = true;
}

//******************************************************************************
// regenerate a line's font texture (GDI bitmap -> D3D11 texture)
//******************************************************************************
int MTDashboard11::_RegenLine(int idx)
{
	int result = 0;
	Line& line = m_Lines[idx];
	unsigned long h = 0, w = 0, x = 0, y = 0;
	DWORD* pBuf = NULL;
	ID3D11Texture2D* pTex = NULL;
	HRESULT hr = S_OK;

	if (line.pSRV != NULL) { line.pSRV->Release(); line.pSRV = NULL; }
	line.texW = 0; line.texH = 0;

	if (line.text[0] == '\0') return 0;

	result = m_Font.CreateBmp(line.text);
	if (result != 0) return result;
	m_Font.GetBmpSize(&h, &w);
	if ((w == 0) || (h == 0)) return 0;

	pBuf = (DWORD*)malloc((size_t)w * h * sizeof(DWORD));
	if (pBuf == NULL) return YN_SET_ERR("Could not allocate memory.", 0, 0);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			BYTE p = m_Font.GetBmpPixcel(x, y);
			// glyph coverage (p = 0..16) scaled by the caption alpha (DX9 CaptionRGBA.a)
			DWORD a = ((DWORD)m_FontAlpha * p) / 16;
			pBuf[y * w + x] = (p == 0) ? 0 : ((a << 24) | (m_FontColorRGB & 0x00FFFFFF));
		}
	}

	{
		D3D11_TEXTURE2D_DESC td;
		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&td, sizeof(td));
		td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_IMMUTABLE;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = pBuf;
		sd.SysMemPitch = w * sizeof(DWORD);
		hr = m_pDevice->CreateTexture2D(&td, &sd, &pTex);
		if (SUCCEEDED(hr)) {
			hr = m_pDevice->CreateShaderResourceView(pTex, NULL, &line.pSRV);
			pTex->Release();
		}
		if (FAILED(hr)) { free(pBuf); return YN_SET_ERR("DirectX API error.", hr, 0); }
	}

	line.texW = w;
	line.texH = h;
	free(pBuf);
	return 0;
}

//******************************************************************************
// Draw: file name top-left, counter block bottom-left (screen-space quads)
//******************************************************************************
int MTDashboard11::DrawDX11(ID3D11DeviceContext* pContext, unsigned int screenW, unsigned int screenH)
{
	int i = 0;
	XMMATRIX ident = XMMatrixIdentity();
	XMFLOAT4 light(0.0f, 0.0f, 1.0f, 0.0f);
	float topY = (float)MTDB11_MARGIN;
	float bottomBlockH = 0.0f;
	float maxW = (float)screenW - 2.0f * (float)MTDB11_MARGIN;
	float mag = MTDB11_DEF_MAG;   // start at the default magnification, shrink to fit width

	if (!m_Ready || (screenW == 0) || (screenH == 0)) return 0;

	// regen dirty lines first; derive the global magnification from the counter
	// width so it fits the screen (matches the DX9 m_CounterMag applied to both
	// the file name and the counter).
	for (i = 0; i < MTDB11_MAX_LINES; i++) {
		if (m_Lines[i].dirty) { _RegenLine(i); m_Lines[i].dirty = false; }
		if ((m_Lines[i].anchor == AnchorBottom) && (m_Lines[i].pSRV != NULL)
		 && (m_Lines[i].texW > 0) && ((float)m_Lines[i].texW * mag > maxW)) {
			mag = maxW / (float)m_Lines[i].texW;
		}
	}
	if (mag <= 0.0f) mag = 1.0f;

	for (i = 0; i < MTDB11_MAX_LINES; i++) {
		if ((m_Lines[i].anchor == AnchorBottom) && (m_Lines[i].pSRV != NULL))
			bottomBlockH += (float)m_Lines[i].texH * mag + (float)MTDB11_LINE_GAP;
	}

	float bottomY = (float)screenH - bottomBlockH - (float)MTDB11_MARGIN;

	for (i = 0; i < MTDB11_MAX_LINES; i++) {
		Line& line = m_Lines[i];
		if ((line.pSRV == NULL) || (line.texW == 0) || (line.texH == 0)) continue;

		float w = (float)line.texW * mag;
		float h = (float)line.texH * mag;
		float px = (float)MTDB11_MARGIN;
		float py;
		if (line.anchor == AnchorTop) {
			py = topY;
			topY += h + (float)MTDB11_LINE_GAP;
		} else {
			py = bottomY;
			bottomY += h + (float)MTDB11_LINE_GAP;
		}

		float x0 = -1.0f + 2.0f * (px) / (float)screenW;
		float x1 = -1.0f + 2.0f * (px + w) / (float)screenW;
		float y0 = 1.0f - 2.0f * (py) / (float)screenH;
		float y1 = 1.0f - 2.0f * (py + h) / (float)screenH;

		DXP11_VERTEX* pv = NULL;
		if (line.prim.LockVertex(pContext, &pv) == 0) {
			pv[0].pos[0]=x0; pv[0].pos[1]=y0; pv[0].pos[2]=0.0f; pv[0].uv[0]=0; pv[0].uv[1]=0;
			pv[1].pos[0]=x1; pv[1].pos[1]=y0; pv[1].pos[2]=0.0f; pv[1].uv[0]=1; pv[1].uv[1]=0;
			pv[2].pos[0]=x0; pv[2].pos[1]=y1; pv[2].pos[2]=0.0f; pv[2].uv[0]=0; pv[2].uv[1]=1;
			pv[3].pos[0]=x1; pv[3].pos[1]=y1; pv[3].pos[2]=0.0f; pv[3].uv[0]=1; pv[3].uv[1]=1;
			for (int k = 0; k < 4; k++) {
				pv[k].normal[0]=0; pv[k].normal[1]=0; pv[k].normal[2]=-1;
				pv[k].color = 0xFFFFFFFF;
			}
			line.prim.UnlockVertex(pContext);
		}

		line.prim.SetTexture(line.pSRV);
		line.prim.SetWorldMatrix(ident);
		line.prim.Draw(pContext, ident, light, 2, 0);
	}

	return 0;
}
