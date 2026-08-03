//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRing11
//
// DX11 Ring-scene time indicator (M4.13): a circle (128-seg LINELIST) on the
// YZ plane at the now-line, riding the playback position on X. Port of
// MTTimeIndicatorRing.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "DXPrimitive11.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


class MTTimeIndicatorRing11
{
public:
	MTTimeIndicatorRing11();
	virtual ~MTTimeIndicatorRing11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	void SetCurTickTime(unsigned long curTickTime) { m_CurTickTime = curTickTime; }
	void Reset() { m_CurTickTime = 0; }

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	bool IsReady() { return m_Ready; }

private:
	MTNoteDesignRing m_NoteDesign;
	DXPrimitive11 m_Prim;
	DirectX::XMFLOAT3 m_WorldMove;
	unsigned long m_CurTickTime;
	bool m_Ready;

	void operator=(const MTTimeIndicatorRing11&);
	MTTimeIndicatorRing11(const MTTimeIndicatorRing11&);
};
