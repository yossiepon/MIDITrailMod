//******************************************************************************
//
// MIDITrail / MTGridRing11
//
// DX11 grid ring renderer (Playback mode).
// Draws 128-segment ring lines at each bar position.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridRingBase11.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// DX11 grid ring renderer (Playback)
//******************************************************************************
class MTGridRing11 : public MTGridRingBase11
{
public:

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName, SMSeqData* pSeqData);

private:

	int _CreateVertexOfGrid(
			DXPRIMITIVE11_VERTEX* pVertex,
			unsigned long* pIndex,
			unsigned long totalTickTime,
			SMBarList* pBarList);
};
