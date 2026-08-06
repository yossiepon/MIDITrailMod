//******************************************************************************
//
// MIDITrail / MTPianoKeyboardRain11
//
// DX11 piano keyboard for Rain scene (1ch).
// Generates key geometry in Rain native coordinates (X=pitch, Y=height, Z=depth).
// No model orientation transform applied.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboard11.h"
#include "MTPianoKeyboardDesign.h"


//******************************************************************************
// DX11 piano keyboard for Rain scene (1ch)
//******************************************************************************
class MTPianoKeyboardRain11 : public MTPianoKeyboard11
{
public:

	MTPianoKeyboardRain11();
	virtual ~MTPianoKeyboardRain11();

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

	MTPianoKeyboardDesign m_KeyboardDesign;
};
