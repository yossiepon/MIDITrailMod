//******************************************************************************
//
// MIDITrail / MTGridRingLive11
//
// Live monitor grid ring renderer (DX11).
// Fixed 2-ring wireframe (start + end of display window).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridRing11Base.h"


//******************************************************************************
// Live grid ring renderer
//******************************************************************************
class MTGridRingLive11 : public MTGridRing11Base
{
public:

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName);

private:

	int _CreateVertices(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};
