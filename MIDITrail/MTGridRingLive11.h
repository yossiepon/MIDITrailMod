//******************************************************************************
//
// MIDITrail / MTGridRingLive11
//
// Grid ring renderer (Live).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridRingBase11.h"


//******************************************************************************
// Live grid ring renderer
//******************************************************************************
class MTGridRingLive11 : public MTGridRingBase11
{
public:

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName);

private:

	int _CreateVertices(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};
