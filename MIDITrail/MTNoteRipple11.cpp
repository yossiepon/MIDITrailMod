//******************************************************************************
//
// MIDITrail / MTNoteRipple11
//
// DX11 note ripple effect (M3) - wraps the real MTNoteRippleMod
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTNoteRipple11.h"
#include "DXTexture11.h"
#include <stdlib.h>

using namespace YNBaseLib;
using namespace DirectX;

#define MTNR11_MAX_RIPPLE  (100)   // == MTNOTERIPPLE_MAX_RIPPLE_NUM


MTNoteRipple11::MTNoteRipple11()
{
	m_pSRV = NULL;
	m_pCpuBuf = NULL;
	m_VertCapacity = 0;
	m_LastMSec = 0;
	m_Ready = false;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pExtPitchBend = NULL;
	m_LiveMode = false;
	m_LiveBase = 0;
}

MTNoteRipple11::~MTNoteRipple11()
{
	Release();
}

void MTNoteRipple11::Release()
{
	m_Prim.Release();
	if (m_pSRV != NULL) { m_pSRV->Release(); m_pSRV = NULL; }
	if (m_pCpuBuf != NULL) { free(m_pCpuBuf); m_pCpuBuf = NULL; }
	m_VertCapacity = 0;
	m_LastMSec = 0;
	m_Ready = false;
}

//******************************************************************************
// backward seek -> rebuild the ripple's forward note window
//******************************************************************************
void MTNoteRipple11::SetPlayTimeMSec(unsigned long ms)
{
	// a real backward seek (or a new/shorter song) rewinds the forward note scan; a
	// few ms of jitter is ignored. Reset ONCE and restart tracking from the new time
	// - else m_LastMSec stays high and every later frame re-triggers the reset, so
	// the ripple never accumulates (notably after switching to the next MIDI).
	if (ms + 200 < m_LastMSec) {
		m_Ripple.Reset();
		m_LastMSec = ms;
	}
	if (m_LiveMode) return;   // live monitor self-clocks in DrawDX11 (timeGetTime)
	if (ms > m_LastMSec) m_LastMSec = ms;
	m_Ripple.SetPlayTimeMSec(ms);
}

//******************************************************************************
// live monitor: register a real-time note-on (timeGetTime-stamped) into the ripple
//******************************************************************************
void MTNoteRipple11::SetNoteOnLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo)
{
	if (!m_Ready || !m_LiveMode) return;
	m_Ripple.AddLiveNoteOn(portNo, chNo, noteNo, timeGetTime() - m_LiveBase);
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteRipple11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		bool ringMode
	)
{
	int result = 0;
	D3DXVECTOR3 mv;
	TCHAR texPath[_MAX_PATH] = { _T('\0') };
	unsigned int tw = 0, th = 0;
	unsigned long overwrite = 1;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	m_Ripple.SetRingMode(ringMode);   //M4.13: ring positions for the Ring scene
	//M4.23: ripple follows the bent note position; use the app's shared pitch
	//bend if provided (else the internal no-bend instance, matching old behavior).
	result = m_Ripple.InitForDX11(pSceneName, pSeqData,
			(m_pExtPitchBend != NULL) ? m_pExtPitchBend : &m_PitchBend);
	if (result != 0) goto EXIT;

	mv = m_Ripple.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	// 6 verts per ripple, multiplied by the overwrite count (additive build-up)
	overwrite = m_Ripple.GetOverwriteTimes();
	m_VertCapacity = 6 * MTNR11_MAX_RIPPLE * overwrite;

	result = m_Prim.CreateVertexBuffer(pDevice, m_VertCapacity);
	if (result != 0) goto EXIT;

	// identity index buffer (the ripple vertices are already a triangle list)
	{
		unsigned long* pi = NULL;
		unsigned long i = 0;
		result = m_Prim.CreateIndexBuffer(pDevice, m_VertCapacity);
		if (result != 0) goto EXIT;
		result = m_Prim.LockIndex(pContext, &pi);
		if (result != 0) goto EXIT;
		for (i = 0; i < m_VertCapacity; i++) pi[i] = i;
		m_Prim.UnlockIndex(pContext);
	}

	m_pCpuBuf = malloc((size_t)m_VertCapacity * sizeof(DXP11_VERTEX));
	if (m_pCpuBuf == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	if (m_Ripple.GetTexturePath(pSceneName, texPath, _MAX_PATH) == 0) {
		if (DXTexture11::LoadFromFile(pDevice, texPath, &m_pSRV, &tw, &th) == 0) {
			m_Prim.SetTexture(m_pSRV);
		}
	}

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);   // ripples are unlit (full color)
	m_Prim.SetAdditiveBlend(true);                 // [Ripple] config: SRCALPHA/ONE
	m_LastMSec = 0;
	// live monitor: no song -> real-time note-on driven, clocked by timeGetTime
	m_LiveMode = (pSeqData == NULL);
	m_LiveBase = m_LiveMode ? timeGetTime() : 0;
	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteRipple11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle,
		const XMFLOAT3& camPos
	)
{
	unsigned long activeNum = 0;
	DXP11_VERTEX* pv = NULL;

	if (!m_Ready) return 0;

	//live: advance the ripple envelope by wall-clock time (no playback timeline)
	if (m_LiveMode) m_Ripple.SetPlayTimeMSec(timeGetTime() - m_LiveBase);

	m_Ripple.UpdateCPU(D3DXVECTOR3(camPos.x, camPos.y, camPos.z), m_pCpuBuf, &activeNum);
	if (m_LiveMode) m_Ripple.RecycleLiveListIfIdle();
	if (activeNum == 0) return 0;
	if (activeNum * 6 > m_VertCapacity) activeNum = m_VertCapacity / 6;

	if (m_Prim.LockVertex(pContext, &pv) != 0) return 0;
	memcpy(pv, m_pCpuBuf, (size_t)activeNum * 6 * sizeof(DXP11_VERTEX));
	m_Prim.UnlockVertex(pContext);

	// world = RotX(roll) * Trans(worldMove)  (same as the note field)
	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
	               * XMMatrixTranslation(m_WorldMove.x, m_WorldMove.y, m_WorldMove.z);
	m_Prim.SetWorldMatrix(world);

	// the overwrite multiplier is already baked into the vertex count
	return m_Prim.Draw(pContext, viewProj, lightDir, (int)(2 * activeNum), 0);
}
