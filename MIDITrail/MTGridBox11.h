//******************************************************************************
//
// MIDITrail / MTGridBox11
//
// DX11 grid box (M3): the piano-roll grid lines (box outline + bar lines +
// port split lines). Reuses MTGridBox's exact line geometry, rendered as a
// LINELIST via DXPrimitive11. Static geometry; the camera scrolls past it.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "MTGridBox.h"
#include "SMIDILib.h"

using namespace SMIDILib;


class MTGridBox11
{
public:
	MTGridBox11();
	virtual ~MTGridBox11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	bool IsReady() { return m_Ready; }

private:
	MTGridBox m_Geom;
	DXPrimitive11 m_Prim;
	bool m_Ready;
	DirectX::XMFLOAT3 m_WorldMove;
};
