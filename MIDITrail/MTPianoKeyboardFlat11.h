//******************************************************************************
//
// MIDITrail / MTPianoKeyboardFlat11
//
// Piano keyboard renderer for flat scenes (single channel).
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
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

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				ID3D11ShaderResourceView* pSRV
			) override;

protected:

	MTPianoKeyboardDesign11 m_KeyboardDesign;

	int _CreateVertexOfKeyboard(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext
			) override;

	int _BuildKeyCPU(unsigned char noteNo, float rate,
				DirectX::SimpleMath::Color* pColor = NULL) override;
};
