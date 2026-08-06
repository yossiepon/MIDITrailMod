//******************************************************************************
//
// MIDITrail / MTPianoKeyboardRoll11
//
// DX11 piano keyboard for PianoRoll scene (1ch).
// Generates key geometry in Rain coordinates (same as Rain11).
// World matrix orientation is handled by MTPianoKeyboardCtrlRoll11.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboard11.h"
#include "MTPianoKeyboardDesignMod.h"


//******************************************************************************
// DX11 piano keyboard for PianoRoll scene (1ch)
//******************************************************************************
class MTPianoKeyboardRoll11 : public MTPianoKeyboard11
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

protected:

	int _CreateVertexOfKeyboard(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext
			) override;

	int _BuildKeyCPU(unsigned char noteNo, float rate,
				DirectX::SimpleMath::Color* pColor = NULL) override;

private:

	MTPianoKeyboardDesignMod m_DesignMod;
};
