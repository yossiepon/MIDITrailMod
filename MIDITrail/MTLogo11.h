//******************************************************************************
//
// MIDITrail / MTLogo11
//
// DX11 startup / title logo (port of MTLogo + MTSceneTitle).
//
// MEMO:
//   The title "MIDITrail" is rendered to a single GDI font texture, then drawn
//   as a horizontal strip of MTLOGO11_TILE_NUM tiles. Each tile samples a
//   vertical slice of the texture and carries its own grey vertex color; the
//   color sweeps bright->dark across the tiles over time, producing the
//   shimmering gradation of the original title. A perspective camera slowly
//   recedes (z increasing) for a subtle drift, matching MTSceneTitle.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"
#include "DXCamera.h"
#include "MTFont2Bmp.h"


class MTLogo11
{
public:
	MTLogo11();
	virtual ~MTLogo11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Release();

	// draw the title using its own receding camera; aspect = screen w/h
	int DrawDX11(ID3D11DeviceContext* pContext, float aspect);

	bool IsReady() { return m_Ready; }

private:
	ID3D11Device* m_pDevice;
	MTFont2Bmp m_Font;
	ID3D11ShaderResourceView* m_pSRV;
	unsigned long m_TexW;
	unsigned long m_TexH;
	DXPrimitive11 m_Prim;
	DXP11_VERTEX* m_pVtx;     // CPU copy (positions/uv fixed; colors updated per frame)
	DXCamera m_Camera;
	float m_CamPosZ;
	unsigned long m_StartTime;
	bool m_Ready;

	int _CreateTexture();
	int _CreateVertex(ID3D11DeviceContext* pContext);
	void _SetGradationColor();

	void operator=(const MTLogo11&);
	MTLogo11(const MTLogo11&);
};
