//******************************************************************************
//
// MIDITrail / MTGridBoxLive11
//
// Live monitor grid box renderer (DX11).
// Fixed time-window wireframe cuboid (no bar lines, no port separators).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
