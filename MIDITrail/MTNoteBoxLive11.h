//******************************************************************************
//
// MIDITrail / MTNoteBoxLive11
//
// DX11 live-monitor note-box renderer - port of MTNoteBoxLive.
//
// MEMO:
// Real-time visualization for the live monitor: notes are registered on MIDI
// note-on (SetNoteOn) and closed on note-off (SetNoteOff), each timestamped with
// timeGetTime(). Every frame the active notes are laid out via the note design's
// "live" placement (GetNoteBoxVirtexPosLive), so a held note grows from the now
// line and the whole field scrolls by elapsed time, then notes age out past the
// display-duration window. Self-contained (DXPrimitive11 + dynamic vertex buffer),
// mirroring MTNoteRipple11/MTNoteLyrics11.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "MTNoteDesign.h"
#include "MTNoteDesignRing.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNotePitchBend.h"


//maximum simultaneously displayed live notes (== DX9 MTNOTEBOX_MAX_LIVENOTE_NUM)
#define MTNOTEBOXLIVE11_MAX_NOTES  (2048)


class MTNoteBoxLive11
{
public:
	MTNoteBoxLive11();
	virtual ~MTNoteBoxLive11();

	// ringMode = true uses the circular (Ring scene) live note placement
	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData, bool ringMode = false);
	void Release();

	void SetPitchBend(MTNotePitchBend* p) { m_pExtPitchBend = p; }

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	void Reset();

	// MIDI input (timeGetTime() stamped internally)
	void SetNoteOn(unsigned char portNo, unsigned char chNo, unsigned char noteNo, unsigned char velocity);
	void SetNoteOff(unsigned char portNo, unsigned char chNo, unsigned char noteNo);
	void AllNoteOff();
	void AllNoteOffOnCh(unsigned char portNo, unsigned char chNo);

	bool IsReady() { return m_Ready; }

private:

	struct NoteStatus {
		bool isActive;
		unsigned char portNo;
		unsigned char chNo;
		unsigned char noteNo;
		unsigned long startTime;   // timeGetTime() at note-on
		unsigned long endTime;     // timeGetTime() at note-off; 0 = still held
	};

	MTNoteDesign m_NoteDesign;          // box (PianoRoll) placement
	MTNoteDesignRing m_NoteDesignRing;  // ring placement (used when ringMode)
	MTNoteDesign* m_pDesign;            // -> the active design (box or ring)
	MTNotePitchBend m_PitchBend;       // fallback (no bend)
	MTNotePitchBend* m_pExtPitchBend;  // app's shared bend (not owned; NULL = use m_PitchBend)

	DXPrimitive11 m_Prim;
	void* m_pCpuBuf;                   // 24 verts * MAX
	unsigned long m_VertCapacity;

	NoteStatus* m_pNoteStatus;
	unsigned long m_DisplayDuration;   // ms window (GetLiveMonitorDisplayDuration)
	unsigned long m_MinNoteElapsed;    // min note length (ms) so a just-pressed note is visible
	float m_NowLineOffsetX;            // shift so the note leading edge sits at the keyboard front
	DirectX::XMFLOAT3 m_WorldMove;
	bool m_Ready;

	MTNotePitchBend* _Bend() { return (m_pExtPitchBend != NULL) ? m_pExtPitchBend : &m_PitchBend; }
	void _ExpireOld(unsigned long curTime);
	void _ClearOldest(unsigned long* pIndex);
	unsigned long _BuildVertices(unsigned long curTime, DXP11_VERTEX* pBuf);
	void _BuildNoteBox(const NoteStatus& note, unsigned long curTime, DXP11_VERTEX* pv);
};
