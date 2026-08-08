//******************************************************************************
//
// MIDITrail / MTPianoKeyboardRoll11
//
// DX11 piano keyboard for PianoRoll scene (1ch).
// Inherits flat key generation from MTPianoKeyboardFlat11.
// Only Create differs (uses MTPianoKeyboardDesignMod).
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardFlat11.h"
#include "MTPianoKeyboardDesignMod.h"


//******************************************************************************
// DX11 piano keyboard for PianoRoll scene (1ch)
//******************************************************************************
class MTPianoKeyboardRoll11 : public MTPianoKeyboardFlat11
{
public:

	MTPianoKeyboardRoll11();
	virtual ~MTPianoKeyboardRoll11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				ID3D11ShaderResourceView* pSRV
			) override;

private:

	MTPianoKeyboardDesignMod m_DesignMod;
};
