//******************************************************************************
//
// MIDITrail / MTPianoKeyboardRain11
//
// DX11 piano keyboard for Rain scene (1ch).
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTPianoKeyboardRain11.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardRain11::MTPianoKeyboardRain11()
{
}

MTPianoKeyboardRain11::~MTPianoKeyboardRain11()
{
}

//******************************************************************************
// Create
//******************************************************************************
int MTPianoKeyboardRain11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		ID3D11ShaderResourceView* pSRV
	)
{
	int result = 0;

	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	m_pKeyboardDesign = &m_KeyboardDesign;

	result = MTPianoKeyboard11::Create(pDevice, pContext, pSceneName, pSeqData, pSRV);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}
