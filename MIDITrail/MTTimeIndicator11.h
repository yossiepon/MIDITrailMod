//******************************************************************************
//
// MIDITrail / MTTimeIndicator11
//
// DX11 time indicator (M4.4): the translucent "now playing" playback section,
// a quad spanning the port area at the now-line on X. Port of MTTimeIndicator.
// Degenerates to a vertical line when viewed head-on (quad becomes invisible).
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "DXPrimitive11.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


class MTTimeIndicator11
{
public:
	MTTimeIndicator11();
	virtual ~MTTimeIndicator11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	void SetCurTickTime(unsigned long curTickTime) { m_CurTickTime = curTickTime; }
	void SetEnable(bool isEnable) { m_Enable = isEnable; }
	void Reset() { m_CurTickTime = 0; }

	// world = RotX(roll) * Trans(worldMove + (playPosX,0,0))
	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	bool IsReady() { return m_Ready; }

private:
	MTNoteDesign m_NoteDesign;
	DXPrimitive11 m_Prim;       // quad (triangle list)
	DXPrimitive11 m_PrimLine;   // degenerate line
	DirectX::XMFLOAT3 m_WorldMove;
	unsigned long m_CurTickTime;
	bool m_EnableLine;
	bool m_Enable;
	bool m_Ready;

	void operator=(const MTTimeIndicator11&);
	MTTimeIndicator11(const MTTimeIndicator11&);
};
