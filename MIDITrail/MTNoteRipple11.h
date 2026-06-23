//******************************************************************************
//
// MIDITrail / MTNoteRipple11
//
// DX11 note ripple effect (M3): wraps the real MTNoteRippleMod (keyDownRate
// envelope driven, real-time note list) and renders its quads via DXPrimitive11
// with additive blend + the Ripple texture.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "MTNoteRippleMod.h"
#include "MTNotePitchBend.h"
#include "SMIDILib.h"

using namespace SMIDILib;


class MTNoteRipple11
{
public:
	MTNoteRipple11();
	virtual ~MTNoteRipple11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData, bool ringMode = false);
	void Release();

	// M4.23: use the app's shared pitch bend so ripples follow the bent note
	// position (DX9 does this). Call before Create. NULL = internal no-bend.
	void SetPitchBend(MTNotePitchBend* p) { m_pExtPitchBend = p; }

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle, const DirectX::XMFLOAT3& camPos);

	// per-frame playback position: tick (note box position) + msec (envelope)
	void SetCurTickTime(unsigned long t) { m_Ripple.SetCurTickTime(t); }
	void SetPlayTimeMSec(unsigned long ms);
	void Reset() { m_Ripple.Reset(); m_LastMSec = 0; }

	// live monitor (Create with pSeqData == NULL): drive ripples from real-time
	// note-ons; the envelope is clocked by timeGetTime, not the playback time.
	void SetNoteOnLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo);

	bool IsReady() { return m_Ready; }

private:
	MTNoteRippleMod m_Ripple;       // real Mod ripple (device-free path)
	bool m_LiveMode;                // real-time note-on driven (no song)
	unsigned long m_LiveBase;       // timeGetTime() at live start (clock origin)
	MTNotePitchBend m_PitchBend;    // fallback (no bend) if no external one is set
	MTNotePitchBend* m_pExtPitchBend;  // M4.23: app's shared bend (not owned; NULL = use m_PitchBend)
	DXPrimitive11 m_Prim;
	ID3D11ShaderResourceView* m_pSRV;
	void* m_pCpuBuf;
	unsigned long m_VertCapacity;
	unsigned long m_LastMSec;
	bool m_Ready;
	DirectX::XMFLOAT3 m_WorldMove;
};
