//******************************************************************************
//
// MIDITrail / DXNoteBoxRing11
//
// Direct3D 11 instanced note renderer for the Ring scene (M4.9).
//
// MEMO:
// Port of MTNoteBoxRing / MTNoteDesignRing. Notes are laid on a ring around the
// X axis: X = time (start..end), and the note number selects an angle around the
// ring (angle = -((step*noteNo) + step/2), step = 360/128). The radius is the
// port/channel origin (RingRadius + ...). Each note is an 8-corner curved box
// (inner/outer radius x angle1/angle2, extruded over its time span). One
// GPU-instanced draw expands the box per note in the vertex shader (radius+angle
// -> y=cos*r, z=-sin*r), so big files stay in the memory budget. Notes are
// static; the camera scrolls +X (DirX) to follow playback.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "MTNoteDesignRing.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


class DXNoteBoxRing11
{
public:
	DXNoteBoxRing11();
	virtual ~DXNoteBoxRing11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj, float rollAngle);

	void SetCurTickTime(unsigned long curTickTime) { m_CurTickTime = curTickTime; }
	void Reset() { m_CurTickTime = 0; }

	// M4.21: per-(port,ch) pitch bend state (active notes shift in angle)
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
	// per-note instance (20 B): time span (xStart,xEnd), ring radius, angle0(deg), color
	struct DXNBR11_INSTANCE {
		float xStart;
		float xEnd;
		float radius;
		float angle0;
		unsigned long color;   // D3DCOLOR 0xAARRGGBB (read as B8G8R8A8_UNORM)
	};

	MTNoteDesignRing m_NoteDesign;
	SMNoteList m_NoteList;

	// M4.21: pitch bend (not owned); active notes rotate by the channel's bend
	MTNotePitchBend* m_pPitchBend;
	bool m_BendAllNotes;   // M4.22: bend the whole channel (not just sounding notes)

	bool m_Ready;
	unsigned long m_CurTickTime;
	DirectX::XMFLOAT3 m_WorldMove;
	float m_HalfWidth;       // box radial half-width (NoteBoxWidth/2)
	float m_HalfAngleDeg;    // box angular half-step (NoteAngleStep/2)

	ID3D11Buffer* m_pInstanceVB;
	unsigned long m_AllNoteNum;
	unsigned long* m_pNoteStartTime;
	unsigned long* m_pNoteMaxEndTime;
	unsigned char* m_pNoteTrackNo;   // per-note source track (track color mode only; NULL otherwise)

	static ID3D11VertexShader* s_pVS;
	static ID3D11PixelShader* s_pPS;
	static ID3D11InputLayout* s_pLayout;
	static ID3D11Buffer* s_pConstBuf;
	static ID3D11Buffer* s_pTemplateVB;   // 8 unit-box corners
	static ID3D11Buffer* s_pBoxIB;        // 36 indices
	static ID3D11RasterizerState* s_pRaster;
	static ID3D11BlendState* s_pBlend;
	static ID3D11DepthStencilState* s_pDepth;

	int _CreateInstanceBuffer(ID3D11Device* pDevice);
	void _GetVisibleNoteRange(unsigned long* pLoNote, unsigned long* pHiNote);
};
