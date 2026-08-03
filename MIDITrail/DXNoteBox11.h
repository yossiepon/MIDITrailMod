//******************************************************************************
//
// MIDITrail / DXNoteBox11
//
// Direct3D 11 instanced note-box renderer (M3)
//
//******************************************************************************

// MEMO:
// Renders all note boxes of a song with one GPU-instanced draw call.
// Geometry uses the authentic MTNoteDesign frame: X = time (start..end),
// Y = pitch (note number height), Z = channel (depth). The note field is
// static world geometry; the camera scrolls along +X to follow playback.
// Per-note data is a compact instance record (min/max box corner + color);
// a vertex shader expands the unit box per instance (same idea as the DX9
// GPU-instancing path in MTNoteBox).

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


//******************************************************************************
// Instanced note-box renderer
//******************************************************************************
class DXNoteBox11
{
public:

	DXNoteBox11();
	virtual ~DXNoteBox11();

	// build the note field for a loaded song. collapsePorts = true merges every
	// port onto one row (port 0's pitch origin), to match the single keyboard;
	// false keeps the per-port stacked rows (multi keyboard).
	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData, bool collapsePorts = true);
	void Release();

	// loading-screen progress hook: called periodically while the (potentially huge)
	// instance buffer is being built, so the loading bar keeps moving for black MIDI.
	typedef void (*BuildProgressFunc)(unsigned long current, unsigned long total, void* user);
	static void SetBuildProgressCallback(BuildProgressFunc func, void* user);

	// draw the visible note range (viewProj from the scene camera; rollAngle in degrees)
	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj, float rollAngle);

	// current playback tick (drives draw culling)
	void SetCurTickTime(unsigned long curTickTime) { m_CurTickTime = curTickTime; }
	void Reset() { m_CurTickTime = 0; }

	// song time scale (ticks per ms = TimeDivision * BPM / 60000); lets the active-note
	// white flash / swell decay over a fixed time (DX9's ActiveNoteDuration), not the note's
	// length. 0 = unknown (stopped / no tempo): the flash falls back to onset-only.
	void SetSongTickPerMs(double tickPerMs) { m_SongTickPerMs = tickPerMs; }

	// per-(port,ch) pitch bend (not owned); shifts active notes in pitch (Y)
	void SetPitchBend(MTNotePitchBend* pPitchBend) { m_pPitchBend = pPitchBend; }

	// M4.22: bend the whole channel (all notes) instead of only sounding notes
	void SetPitchBendAllNotes(bool b) { m_BendAllNotes = b; }

	bool IsReady() { return m_Ready; }

	// total note count (for the dashboard)
	unsigned long GetNoteCount() { return m_AllNoteNum; }

	// M6: latest note-off tick in the song (end of musical content; 0 if empty)
	unsigned long GetMaxEndTick() {
		return ((m_pNoteMaxEndTime != NULL) && (m_AllNoteNum > 0)) ? m_pNoteMaxEndTime[m_AllNoteNum - 1] : 0;
	}

	// number of notes started by curTick (tick-based, message-drop immune)
	unsigned long GetPlayedNoteCount(unsigned long curTick);

	// shared pipeline (built once on the device)
	static int InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

private:

	// per-note instance record (matches the DX9 MTNOTE_INSTANCE layout)
	struct DXNB11_INSTANCE {
		float vmin[3];
		float vmax[3];
		unsigned long color;   // D3DCOLOR 0xAARRGGBB (RGB used; A byte = pitch-bend index)
		float hidden;
		float alpha;           // real note opacity 0..1 (from the colour's A byte) - ced 20260627
	};

	MTNoteDesign m_NoteDesign;
	SMNoteList m_NoteList;
	MTNotePitchBend* m_pPitchBend;   // per-(port,ch) pitch bend (not owned; NULL = none)
	bool m_BendAllNotes;             // M4.22: bend the whole channel (not just sounding)

	// ced 20260713: DX9 scene lighting. MTScenePianoRoll3D lights the note boxes with
	// two opposing directional lights (D3DLIGHT9 direction (1,-1,2) / (-1,1,-2), diffuse
	// 1.2, ambient 0.2 / 0.0) against the note material's ambient 0.5; MTScenePianoRoll2D
	// sets m_IsEnableLight = FALSE, so its notes are flat. Off unless the scene is 3D.
	bool m_LightEnable;
	DirectX::XMFLOAT3 m_LightDir;    // direction the light travels (D3DLIGHT9 convention)
	float m_LightDiffuse;            // light diffuse level (DX9: 1.2)
	float m_LightAmbient;            // material ambient * sum(light ambient) (DX9: 0.5 * 0.2)

	bool m_Ready;
	bool m_CollapsePorts;   // merge all ports onto port 0's row (single keyboard)
	unsigned long m_CurTickTime;
	double m_SongTickPerMs;   // ticks/ms for the active-note flash decay (0 = unknown)

	// world translation (MTNoteDesign::GetWorldMoveVector)
	DirectX::XMFLOAT3 m_WorldMove;

	// instance buffer + culling arrays
	ID3D11Buffer* m_pInstanceVB;
	unsigned long m_AllNoteNum;
	unsigned long* m_pNoteStartTime;
	unsigned long* m_pNoteMaxEndTime;
	unsigned char* m_pNoteTrackNo;   // per-note source track (track color mode only; NULL otherwise)

	// shared pipeline objects
	static ID3D11VertexShader* s_pVS;
	static ID3D11PixelShader* s_pPS;
	static ID3D11InputLayout* s_pLayout;
	static ID3D11Buffer* s_pConstBuf;
	static ID3D11Buffer* s_pTemplateVB;   // 24 box vertices (corner mask + face normal)
	static ID3D11Buffer* s_pBoxIB;        // 36 indices (12 triangles)
	static ID3D11RasterizerState* s_pRaster;
	static ID3D11BlendState* s_pBlend;
	static ID3D11DepthStencilState* s_pDepth;

	static BuildProgressFunc s_BuildProgressFunc;
	static void* s_BuildProgressUser;

	int _CreateInstanceBuffer(ID3D11Device* pDevice, SMNoteList* pNoteList, const unsigned char* pTrackNo);
	void _GetVisibleNoteRange(unsigned long* pLoNote, unsigned long* pHiNote);
	// instance index range whose [start, prefix-max-end] straddles [tickLow, tickHigh]
	void _RangeForTicks(unsigned long tickLow, unsigned long tickHigh, unsigned long* pLoNote, unsigned long* pHiNote);
};
