//******************************************************************************
//
// MIDITrail / MTPianoKeyboardFlat11
//
// DX11 piano keyboard for flat (linear) scenes (1ch).
// Provides _CreateVertexOfKeyboard and _BuildKeyCPU implementations shared
// by Rain and Roll keyboards. Ring uses its own cylindrical generation.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboard11.h"
#include "MTPianoKeyboardDesign11.h"


//******************************************************************************
// DX11 piano keyboard for flat scenes (1ch)
//******************************************************************************
class MTPianoKeyboardFlat11 : public MTPianoKeyboard11
{
public:

	MTPianoKeyboardFlat11();
	virtual ~MTPianoKeyboardFlat11();

protected:

	MTPianoKeyboardDesign11 m_KeyboardDesign;

	int _CreateVertexOfKeyboard(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext
			) override;

	int _BuildKeyCPU(unsigned char noteNo, float rate,
				DirectX::SimpleMath::Color* pColor = NULL) override;
};
