//******************************************************************************
//
// MIDITrail / MTNoteLyrics11
//
// DX11 note-lyrics renderer - port of MTNoteLyrics.
//
// MEMO:
// Lyric (0x05) meta text is attached to its note (SMNote::lyric) at parse time.
// As playback passes each lyric note's start time this draws the text - rasterized
// to a texture via MTFont2Bmp - as a camera-facing quad at the note position,
// shrinking/fading over the note's life (same keyDownRate envelope as the ripple).
// Self-contained (no DX9 base): owns a DXPrimitive11 quad renderer + per-active-
// lyric textures, mirroring MTNoteRipple11's structure.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "MTNoteDesignMod.h"
#include "MTNoteDesignRing.h"
#include "MTNotePitchBend.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//maximum simultaneously drawn lyrics (== DX9 MTNOTELYRICS_MAX_LYRICS_NUM)
#define MTNOTELYRICS11_MAX_LYRICS  (100)

//ports that get an independent keyDownRate de-dup table (== DX9 MTNOTELYRICS_MAX_PORT_NUM)
#define MTNOTELYRICS11_MAX_PORT    (8)


class MTNoteLyrics11
{
public:
	MTNoteLyrics11();
	virtual ~MTNoteLyrics11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	// use the app's shared pitch bend so lyrics follow the bent note (call before Create)
	void SetPitchBend(MTNotePitchBend* p) { m_pExtPitchBend = p; }

	// ring scene: position lyrics on the ring (MTNoteDesignRing) instead of the
	// planar layout. Call before Create. (timing/colour stay ini-shared via Mod)
	void SetRingMode(bool r) { m_RingMode = r; }

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle, const DirectX::XMFLOAT3& camPos);

	void SetCurTickTime(unsigned long t) { m_CurTickTime = t; }
	void SetPlayTimeMSec(unsigned long ms);
	void Reset();

	void SetEnable(bool b) { m_isEnable = b; }
	bool IsReady() { return m_Ready; }

private:

	enum KeyStatus { BeforeNoteON, NoteON, AfterNoteOFF };

	struct LyricStatus {
		bool isActive;
		KeyStatus keyStatus;
		unsigned long index;          // index into m_NoteListRT
		float keyDownRate;
		ID3D11ShaderResourceView* pSRV;  // rasterized text (owned)
		unsigned long texW;
		unsigned long texH;
	};

	ID3D11Device* m_pDevice;          // not owned (texture creation on activation)
	MTNoteDesignMod m_NoteDesign;
	MTNoteDesignRing m_NoteDesignRing; // ring-scene positioning (ring mode only)
	bool m_RingMode;                   // true = lay lyrics on the ring
	MTNotePitchBend m_PitchBend;      // fallback (no bend)
	MTNotePitchBend* m_pExtPitchBend; // app's shared bend (not owned; NULL = use m_PitchBend)

	SMNoteList m_NoteListRT;          // own copy: realtime (ms) note list + lyric
	DXPrimitive11 m_Prim;
	void* m_pCpuBuf;                  // 6 * MAX vertices
	unsigned long m_VertCapacity;

	LyricStatus* m_pStatus;
	// per-drawn-quad texture, aligned with the vertex buffer order (not owned;
	// points into m_pStatus[].pSRV). Filled by _BuildVertices, consumed by Draw.
	ID3D11ShaderResourceView* m_pDrawSRV[MTNOTELYRICS11_MAX_LYRICS];
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	unsigned long m_LastMSec;
	float m_KeyDownRate[MTNOTELYRICS11_MAX_PORT][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	DirectX::XMFLOAT3 m_WorldMove;
	bool m_isEnable;
	bool m_Ready;

	MTNotePitchBend* _Bend() { return (m_pExtPitchBend != NULL) ? m_pExtPitchBend : &m_PitchBend; }
	void _ClearStatus(LyricStatus* p);
	void _UpdateStatus();
	void _UpdateNoteStatus(unsigned long playTimeMSec, unsigned long decayDuration,
			unsigned long releaseDuration, const SMNote& note, LyricStatus* pStatus);
	unsigned long _BuildVertices(const DirectX::XMFLOAT3& camPos, DXP11_VERTEX* pBuf);
	int _CreateLyricTexture(const TCHAR* pStr, ID3D11ShaderResourceView** ppSRV,
			unsigned long* pW, unsigned long* pH);
};
