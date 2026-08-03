//******************************************************************************
//
// MIDITrail / MTKeyboardRain11
//
// DX11 piano keyboard for the Rain scene (M4.7b): port of MTPianoKeyboardCtrl +
// MTPianoKeyboard (the NON-Mod path). Reuses the same key geometry/texture as
// MTKeyboard11 but with the Rain placement: one keyboard per active MIDI channel
// (base = MTPianoKeyboardDesign::GetKeyboardBasePos(0, chNo)), laid in the raw
// geometry frame (pitch = X, surface faces +Y), rising with playback in Y so it
// stays fixed on screen under the falling notes. Transform = Trans(base + (0,
// playPos, 0)) * RotY(roll); no scale / no RotX-RotZ chain (that's the box Mod).
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "SMIDILib.h"

using namespace SMIDILib;


#define MTKBDRAIN11_MAX_KEYBOARDS  (SM_MAX_CH_NUM)


class MTKeyboardRain11
{
public:
	MTKeyboardRain11();
	virtual ~MTKeyboardRain11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	void SetCurTickTime(unsigned long curTickTime);
	void Reset();

	// live monitor: light keys directly from real-time MIDI (no song timeline).
	// Multi-keyboard rain maps the channel to its stacked sub-keyboard.
	void SetNoteOnLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo);
	void SetNoteOffLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo);
	void AllNoteOffLive();

	void SetPitchBend(MTNotePitchBend* pPitchBend) { m_pPitchBend = pPitchBend; }

	bool IsReady() { return m_Ready; }

private:
	struct KbdNote {
		unsigned long startTime;
		unsigned long endTime;
		unsigned long color;
		unsigned char noteNo;
	};

	// per-key active-note color tracking (latest sounding note's color; reverts
	// as notes end). Bounded; down/up still uses keyMaxEndTick.
	static const int KBDR11_COLOR_CAP = 8;
	struct ActiveCol { unsigned long endTime; unsigned long color; };

	struct SubKbd {
		DXPrimitive11 prim;
		void* pWorkVerts;
		int chNo;                       // channel -> base position
		bool dirty;
		KbdNote* pNotes;
		unsigned long noteCount;
		unsigned long nextNoteIdx;
		unsigned long lastTick;
		bool keyDown[SM_MAX_NOTE_NUM];
		unsigned long keyColor[SM_MAX_NOTE_NUM];
		unsigned long keyRenderedColor[SM_MAX_NOTE_NUM];
		unsigned long keyMaxEndTick[SM_MAX_NOTE_NUM];
		ActiveCol activeCol[SM_MAX_NOTE_NUM][KBDR11_COLOR_CAP];
		unsigned char activeColNum[SM_MAX_NOTE_NUM];
	};

	MTPianoKeyboard m_Geom;
	MTPianoKeyboardDesign m_Design;       // non-Mod: GetKeyboardBasePos(port, ch)
	MTNoteDesign m_NoteDesign;            // GetPlayPosX (Y scroll) + note color
	ID3D11ShaderResourceView* m_pSRV;
	MTNotePitchBend* m_pPitchBend;        // per-channel pitch bend (not owned)
	bool m_Ready;
	unsigned long m_CurTickTime;

	void* m_pBaseVerts;
	unsigned long m_VertexNum;

	SubKbd m_Subs[MTKBDRAIN11_MAX_KEYBOARDS];
	unsigned long m_NumKbd;

	int _ApplyKeyStates(ID3D11DeviceContext* pContext, SubKbd* pSub);
	void _AdvanceWindow(SubKbd* pSub, unsigned long tick);
	void _ReleaseSub(SubKbd* pSub);
};
