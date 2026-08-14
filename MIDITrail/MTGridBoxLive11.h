//******************************************************************************
//
// MIDITrail / MTGridBoxLive11
//
// Grid box renderer (Live).
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridBoxBase11.h"


//******************************************************************************
// Live grid box renderer
//******************************************************************************
class MTGridBoxLive11 : public MTGridBoxBase11
{
public:

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName);

private:

	int _CreateVertices(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};
