//******************************************************************************
//
// MIDITrail / MTGridRing11
//
// DX11 Ring-scene grid (M4.13): two concentric circles (LINELIST) on the YZ
// plane - inner at time 0, outer at the song end - giving the cylindrical grid.
// Port of MTGridRing. Static; the camera scrolls +X.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "DXPrimitive11.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


class MTGridRing11
{
public:
	MTGridRing11();
	virtual ~MTGridRing11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	bool IsReady() { return m_Ready; }

private:
	MTNoteDesignRing m_NoteDesign;
	DXPrimitive11 m_Prim;
	DirectX::XMFLOAT3 m_WorldMove;
	bool m_Ready;

	void operator=(const MTGridRing11&);
	MTGridRing11(const MTGridRing11&);
};
