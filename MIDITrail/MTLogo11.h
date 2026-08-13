//******************************************************************************
//
// MIDITrail / MTLogo11
//
// MIDITrail logo renderer with gradation animation.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include "DXPrimitive11.h"
#include "MTFontTexture11.h"

#define MTLOGO_TITLE           L"MIDITrail"
#define MTLOGO_POS_X           (20.0f)
#define MTLOGO_POS_Y           (-15.0f)
#define MTLOGO_MAG             (0.1f)
#define MTLOGO_TILE_NUM        (40)
#define MTLOGO_GRADATION_TIME  (1000)


//******************************************************************************
// MIDITrail logo class
//******************************************************************************
class MTLogo11
{
public:

	MTLogo11();
	virtual ~MTLogo11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	int Update(ID3D11DeviceContext* pContext);
	int Draw(ID3D11DeviceContext* pContext,
	         const DirectX::SimpleMath::Matrix& viewProj,
	         const DirectX::SimpleMath::Vector4& lightDir);
	void Release();

private:

	DXPrimitive11 m_Primitive;
	MTFontTexture11 m_FontTexture;
	DXPRIMITIVE11_VERTEX* m_pCpuVertex;
	unsigned long m_VertexCount;

	unsigned long m_StartTime;

	int _CreateTexture(ID3D11Device* pDevice);
	int _CreateVertex(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void _SetGradationColor();
};
