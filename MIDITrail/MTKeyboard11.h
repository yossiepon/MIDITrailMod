//******************************************************************************
//
// MIDITrail / MTKeyboard11
//
// DX11 piano keyboard (M3 / M4.6): faithful port of the DX9 MTPianoKeyboardMod
// + MTPianoKeyboardCtrlMod. Reuses MTPianoKeyboard's exact key geometry (white
// C/F/D/G/A/E/B + black shapes with texture UVs) and the HDKeyboard.png texture,
// and the Mod world transform that maps the keyboard's local frame into the note
// world (pitch -> Y) at the playback line.
//
// Multi-keyboard: the Mod scene runs in multi mode (one keyboard per active
// MIDI port, stacked along Y, capped at KeyboardMaxDispNum). Each sub-keyboard
// shares the geometry/texture but has its own per-key press state (only its
// port's notes) and its own base transform (GetKeyboardBasePos(index)).
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardDesignMod.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "SMIDILib.h"

using namespace SMIDILib;


// max simultaneously displayed keyboards. The multi path is capped by
// KeyboardMaxDispNum (<=16); keep this small - the SubKbd array is by value in
// MIDITrailApp (a stack object), so SM_MAX_PORT_NUM (256) here overflows the stack.
#define MTKBD11_MAX_KEYBOARDS  (16)


class MTKeyboard11
{
public:
	MTKeyboard11();
	virtual ~MTKeyboard11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData, bool isSingleKeyboard = true);
	void Release();

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle,
			const DirectX::XMFLOAT3& camPos = DirectX::XMFLOAT3(0, 0, 0));

	// current playback tick: drives the keyboard's X follow position AND the
	// key-press animation (tick-based, so it is robust to dropped note on/off
	// messages under heavy load).
	void SetCurTickTime(unsigned long curTickTime);
	void Reset();

	// Live monitor: drive key presses directly from real-time MIDI (no note list /
	// tick). Lights the single keyboard's key, tinted with the note color, until
	// the matching note-off. m_CurTickTime stays 0 in live, so a key is "down"
	// while its keyMaxEndTick is non-zero (0xFFFFFFFF here).
	void SetNoteOnLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo);
	void SetNoteOffLive(unsigned char noteNo);
	void AllNoteOffLive();

	// per-channel pitch bend (not owned); the keyboard shifts in pitch by the
	// port's strongest channel bend, matching MTPianoKeyboardCtrlMod.
	void SetPitchBend(MTNotePitchBend* pPitchBend) { m_pPitchBend = pPitchBend; }

	// current song time scale (ticks per millisecond = TimeDivision * BPM / 60000),
	// fed per frame so the press envelope's KeyDown/UpDuration (ms) map to ticks at the
	// current tempo. Independent of play-speed: the eased display tick already carries
	// the speed, so the envelope stays correct at any playback speed (matches DX9).
	void SetSongTickPerMs(double tickPerMs) { if (tickPerMs > 0.0) m_TickPerMs = tickPerMs; }

	bool IsReady() { return m_Ready; }

private:
	// compact per-note record (~16 B vs the 32 B SMNote) with the color
	// precomputed, so the full note-list copy is dropped after Create.
	struct KbdNote {
		unsigned long startTime;
		unsigned long endTime;
		unsigned long color;     // D3DCOLOR 0xAARRGGBB (note color / track-channel color)
		unsigned char noteNo;
		unsigned char chNo;      // MIDI channel (for the [PianoKeyboard] ActiveKeyColor palette)
	};

	// one keyboard (per active MIDI port in multi mode)
	struct SubKbd {
		DXPrimitive11 prim;             // own VB/IB (VB mutated for key presses)
		void* pWorkVerts;               // current VB contents (CPU mirror)
		int keyboardIndex;              // design index -> base position
		int portNo;                     // MIDI port (for pitch-bend lookup)
		bool dirty;
		// playback notes grouped per key (pNotes laid out as [key 0 notes][key 1 ..],
		// each block sorted by start tick). keyOffset[k]..keyOffset[k+1] is key k's
		// block; keyCursor[k] skips notes whose release tail has fully passed. No fixed
		// per-key cap, so a hammered key (black MIDI) never loses its sounding note.
		KbdNote* pNotes;
		unsigned long noteCount;
		unsigned long keyOffset[SM_MAX_NOTE_NUM + 1];
		unsigned long keyCursor[SM_MAX_NOTE_NUM];
		unsigned long lastTick;
		bool keyDown[SM_MAX_NOTE_NUM];
		float keyRate[SM_MAX_NOTE_NUM];   // animated press depth 0(up)..1(down)
		unsigned long keyColor[SM_MAX_NOTE_NUM];        // live: current note colour
		unsigned long keyRenderedColor[SM_MAX_NOTE_NUM];
		unsigned long keyMaxEndTick[SM_MAX_NOTE_NUM];   // live monitor key-down tracking
	};

	MTPianoKeyboard m_Geom;                 // geometry generator (DX9 builders, device-free)
	MTPianoKeyboardDesignMod m_DesignMod;   // base position + resize ratio
	MTNoteDesign m_NoteDesign;              // playback position + world move
	ID3D11ShaderResourceView* m_pSRV;       // keyboard texture (owned, shared)
	MTNotePitchBend* m_pPitchBend;          // per-channel pitch bend (not owned)
	bool m_SingleKbd;                       // true = one merged keyboard; false = per port
	bool m_LiveMode;                        // true = live monitor (no song; wall-clock ease)
	bool m_Ready;
	unsigned long m_CurTickTime;
	DirectX::XMFLOAT3 m_WorldMove;

	// key-press animation (DX9 parity):
	//  - PLAYBACK: an anticipatory envelope keyed off the (eased) display tick. A key
	//    presses down over KeyDownDuration BEFORE the note so it is fully down exactly
	//    when the note sounds, holds at 1 while held, then releases over KeyUpDuration
	//    after note-off (matches MTPianoKeyboardCtrlMod). Durations (ms) -> ticks via
	//    m_TickPerMs (song tempo). No per-frame integration; rate = f(tick) directly.
	//  - LIVE: real-time note-ons can't be anticipated, so keys ease down/up over the
	//    same durations in wall-clock time (timeGetTime), from the note-on/off instant.
	unsigned long m_LastAnimMs;     // timeGetTime at the previous DrawDX11 (live ease)
	unsigned long m_KeyDownDurMs;   // ms for a full up->down press  ([Keyboard] KeyDownDuration)
	unsigned long m_KeyUpDurMs;     // ms for a full down->up release ([Keyboard] KeyUpDuration)
	double        m_TickPerMs;      // song ticks per ms at the current tempo (playback envelope)

	void* m_pBaseVerts;                     // unpressed geometry (shared CPU master copy)
	unsigned long m_VertexNum;

	SubKbd m_Subs[MTKBD11_MAX_KEYBOARDS];
	unsigned long m_NumKbd;

	// ced 20260629: infinite keyboard (NotLive box 2D/3D). The keyboard pattern is extended
	// below note 0 and above note 127. ced 20260703: the extension octaves are APPENDED into
	// each keyboard's OWN vertex/index buffer (their octave X-offset baked into the vertex
	// positions), in note order, so they are drawn by the SAME single Draw call, with the same
	// world matrix and the same primitive submission order as the original 0-127 keys. There is
	// no separate buffer and no second draw, so the extension is rendered by the identical path
	// as the main keyboard -> it looks exactly like the original on every GPU (a separate second
	// draw used to let some GPUs cover the extension's raised black keys with white keys). The
	// extension part is static (never animated); _ApplyKeyStates only touches notes 0-127.
	// Off by default ([PianoKeyboard] InfiniteKeyboard).
	bool           m_InfiniteKbd;           // enabled (conf flag) AND not live
	float          m_OctaveWidthX;          // local-X width of one octave (12 semitones)
	DXP11_VERTEX*  m_pExtVerts;             // extension vertices (X-offset baked), note order
	unsigned long* m_pExtIdx;               // extension indices, 0-based within the extension block
	unsigned long  m_ExtVertNum;
	unsigned long  m_ExtIdxNum;
	// m_pExtIdx is laid out [below-note-0 octaves][note 128+ octaves]. This is the index count of
	// the first (below-0) part, so Create() can submit the whole keyboard in true note order:
	// below-0 extension -> main 0-127 -> note-128+ extension. Consistent submission order makes
	// the semi-transparent keys blend the same at the seams as in the interior (fixes a GPU-
	// dependent boundary artifact that appeared only with alpha<255 key colors + depth write).
	unsigned long  m_ExtBottomIdxNum;
	int  _BuildExtCPU(const void* pCpuVB, const unsigned long* pCpuIB);

	int _ApplyKeyStates(ID3D11DeviceContext* pContext, SubKbd* pSub, unsigned long elapsedMs);
	void _AdvanceWindow(SubKbd* pSub, unsigned long tick);
	void _ReleaseSub(SubKbd* pSub);
};
