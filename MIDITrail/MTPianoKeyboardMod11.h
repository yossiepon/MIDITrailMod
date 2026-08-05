//******************************************************************************
//
// MIDITrail / MTPianoKeyboardMod11
//
// DX11 piano keyboard Mod renderer (1ch).
// Overrides key rotation angle from MTPianoKeyboardDesignMod.
// World matrix computation (orientation, scale) is CtrlMod's responsibility.
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboard11.h"
#include "MTPianoKeyboardDesignMod.h"


//******************************************************************************
// DX11 piano keyboard Mod renderer (1ch)
//******************************************************************************
class MTPianoKeyboardMod11 : public MTPianoKeyboard11
{
public:

	MTPianoKeyboardMod11();
	virtual ~MTPianoKeyboardMod11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				ID3D11ShaderResourceView* pSRV
			) override;

protected:

	float _GetKeyRotateAngle() override;

private:

	MTPianoKeyboardDesignMod m_DesignMod;
};
