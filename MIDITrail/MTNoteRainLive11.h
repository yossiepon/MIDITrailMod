//******************************************************************************
//
// MIDITrail / MTNoteRainLive11
//
// DX11 live-monitor falling-note (Rain) renderer - port of MTNoteRainLive.
//
// MEMO:
// Real-time Rain visualization: a note registered on note-on (timeGetTime
// stamped) is a quad at its key's X (black-key width), spanning the note-on edge
// (GetLivePosX(elapsed since on)) and the note-off edge (GetLivePosX(elapsed
// since off), 0 while held), so it grows from the keyboard line and ages out.
// Self-contained (DXPrimitive11 + dynamic vertex buffer), mirroring MTNoteBoxLive11.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "MTNoteDesign.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNotePitchBend.h"


#define MTNOTERAINLIVE11_MAX_NOTES  (2048)


class MTNoteRainLive11
{
public:
	MTNoteRainLive11();
	virtual ~MTNoteRainLive11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	void SetPitchBend(MTNotePitchBend* p) { m_pExtPitchBend = p; }

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	void Reset();

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
		unsigned long endTime;     // timeGetTime() at note-off; 0 = held
	};

	MTNoteDesign m_NoteDesign;
	MTPianoKeyboardDesign m_KeyboardDesign;
	MTNotePitchBend m_PitchBend;
	MTNotePitchBend* m_pExtPitchBend;

	DXPrimitive11 m_Prim;
	void* m_pCpuBuf;                   // 4 verts * MAX
	unsigned long m_VertCapacity;

	NoteStatus* m_pNoteStatus;
	unsigned long m_DisplayDuration;
	unsigned long m_MinNoteElapsed;
	bool m_Ready;

	MTNotePitchBend* _Bend() { return (m_pExtPitchBend != NULL) ? m_pExtPitchBend : &m_PitchBend; }
	void _ExpireOld(unsigned long curTime);
	void _ClearOldest(unsigned long* pIndex);
	unsigned long _BuildVertices(unsigned long curTime, DXP11_VERTEX* pBuf);
	void _BuildNote(const NoteStatus& note, unsigned long curTime, DXP11_VERTEX* pv);
};
