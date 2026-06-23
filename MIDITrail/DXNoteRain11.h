//******************************************************************************
//
// MIDITrail / DXNoteRain11
//
// Direct3D 11 instanced falling-note renderer (M4.7, Rain scene).
//
// MEMO:
// Port of MTNoteRain. Each note is a flat quad in the X-Y plane: X = pitch
// (keyboard key center), Y = time (start..end), Z = key drop depth. The field
// is static; the world scrolls -Y with playback (world = Trans(0,-playPos,0) *
// RotY(roll)). One GPU-instanced draw expands a unit quad per note (same idea
// as DXNoteBox11), so 4M-note files stay within the memory budget. The quad
// fades from alpha 1.0 at the note-on edge to 0.5 at the note-off edge.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "MTNoteDesign.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


class DXNoteRain11
{
public:
	DXNoteRain11();
	virtual ~DXNoteRain11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	// draw the visible note range (viewProj from the scene camera; roll in degrees)
	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj, float rollAngle);

	void SetCurTickTime(unsigned long curTickTime) { m_CurTickTime = curTickTime; }
	void Reset() { m_CurTickTime = 0; }

	// M4.21: per-(port,ch) pitch bend state (active notes shift in pitch / X)
	void SetPitchBend(MTNotePitchBend* pPitchBend) { m_pPitchBend = pPitchBend; }

	// M4.22: bend the whole channel (all notes) instead of only sounding notes
	void SetPitchBendAllNotes(bool b) { m_BendAllNotes = b; }

	bool IsReady() { return m_Ready; }
	unsigned long GetNoteCount() { return m_AllNoteNum; }
	unsigned long GetPlayedNoteCount(unsigned long curTick);

	// M6: latest note-off tick in the song (end of musical content; 0 if empty)
	unsigned long GetMaxEndTick() {
		return ((m_pNoteMaxEndTime != NULL) && (m_AllNoteNum > 0)) ? m_pNoteMaxEndTime[m_AllNoteNum - 1] : 0;
	}

	static int InitPipeline(ID3D11Device* pDevice);
	static void ReleasePipeline();

private:

	// per-note instance record (24 B): box = (cx, yStart, yEnd, z), halfW, color
	struct DXNR11_INSTANCE {
		float cx;
		float yStart;
		float yEnd;
		float z;
		float halfW;
		unsigned long color;   // D3DCOLOR 0xAARRGGBB (read as B8G8R8A8_UNORM)
	};

	MTNoteDesign m_NoteDesign;
	MTPianoKeyboardDesign m_KeyboardDesign;
	SMNoteList m_NoteList;

	bool m_Ready;
	unsigned long m_CurTickTime;

	// M4.21: pitch bend (not owned) + cached per-(port&0xF,ch) Y offset of the
	// keyboard now-line, used to decide which notes are currently sounding.
	MTNotePitchBend* m_pPitchBend;
	bool m_BendAllNotes;   // M4.22: bend the whole channel (not just sounding notes)
	float m_BaseY[256];

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
	static ID3D11Buffer* s_pTemplateVB;   // 4 unit-quad corners
	static ID3D11Buffer* s_pQuadIB;       // 6 indices (2 triangles)
	static ID3D11RasterizerState* s_pRaster;
	static ID3D11BlendState* s_pBlend;
	static ID3D11DepthStencilState* s_pDepth;

	int _CreateInstanceBuffer(ID3D11Device* pDevice);
	void _GetVisibleNoteRange(unsigned long* pLoNote, unsigned long* pHiNote);
};
