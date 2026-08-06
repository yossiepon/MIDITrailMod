//******************************************************************************
//
// MIDITrail / MTPianoKeyboardMod11
//
// DX11 piano keyboard Mod renderer (1ch).
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTPianoKeyboardMod11.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardMod11::MTPianoKeyboardMod11()
{
}

MTPianoKeyboardMod11::~MTPianoKeyboardMod11()
{
}

//******************************************************************************
// Create
//******************************************************************************
int MTPianoKeyboardMod11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		ID3D11ShaderResourceView* pSRV
	)
{
	int result = 0;

	result = MTPianoKeyboard11::Create(pDevice, pContext, pSceneName, pSeqData, pSRV);
	if (result != 0) goto EXIT;

	result = m_DesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Key rotation angle (from DesignMod)
//******************************************************************************
float MTPianoKeyboardMod11::_GetKeyRotateAngle()
{
	return m_DesignMod.GetKeyRotateAngle();
}
