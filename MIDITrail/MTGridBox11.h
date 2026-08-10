//******************************************************************************
//
// MIDITrail / MTGridBox11
//
// DX11 grid box renderer (Playback mode).
// Draws wireframe grid, bar lines, and port separators from SMSeqData.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTGridBoxBase11.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// DX11 grid box renderer (Playback)
//******************************************************************************
class MTGridBox11 : public MTGridBoxBase11
{
public:

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName, SMSeqData* pSeqData);

private:

	int _CreateVertices(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			SMSeqData* pSeqData);
};
