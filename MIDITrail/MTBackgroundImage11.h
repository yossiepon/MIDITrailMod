//******************************************************************************
//
// MIDITrail / MTBackgroundImage11
//
// DX11 background image (M4.15): a full-window, aspect-preserved (letterboxed)
// textured quad drawn behind the scene. Port of MTBackgroundImage. The image
// path comes from Graphic.ini [Background-image] ImageFilePath (empty = off).
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <tchar.h>
#include "DXPrimitive11.h"


class MTBackgroundImage11
{
public:
	MTBackgroundImage11();
	virtual ~MTBackgroundImage11();

	// pImgFilePath: absolute path; NULL/empty or load failure -> not ready (off)
	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TCHAR* pImgFilePath);
	void Release();

	// draw the letterboxed quad for the current window size (screen space, behind all)
	int DrawDX11(ID3D11DeviceContext* pContext, unsigned int screenW, unsigned int screenH);

	bool IsReady() { return m_Ready; }

private:
	DXPrimitive11 m_Prim;
	ID3D11ShaderResourceView* m_pSRV;
	unsigned int m_ImgW;
	unsigned int m_ImgH;
	bool m_Ready;

	void operator=(const MTBackgroundImage11&);
	MTBackgroundImage11(const MTBackgroundImage11&);
};
