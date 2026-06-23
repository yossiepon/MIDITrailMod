//******************************************************************************
//
// MIDITrail / MTStars11
//
// DX11 starfield (port of MTStars): NumberOfStars points scattered on a
// sphere of radius 500 around the origin, rendered as a POINTLIST with random
// grayscale colors. The world matrix is a pure translation by the camera
// position so the stars follow the camera (an infinitely distant sky).
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXPrimitive11.h"


class MTStars11
{
public:
	MTStars11();
	virtual ~MTStars11();

	// numStars <= 0 -> not ready (off). Generates the point cloud once.
	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, int numStars);
	void Release();

	// draw the stars translated to camPos (behind everything)
	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT3& camPos);

	bool IsReady() { return m_Ready; }

private:
	DXPrimitive11 m_Prim;
	int m_NumStars;
	bool m_Ready;

	void operator=(const MTStars11&);
	MTStars11(const MTStars11&);
};
