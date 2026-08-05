//******************************************************************************
//
// MIDITrail / MTSceneComponent11
//
// Common base for DX11 visual scene components.
// Extends IMTSceneManagedComponent with enable/disable state for
// visibility toggling (SetEffect).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "IMTSceneManagedComponent.h"


//******************************************************************************
// DX11 scene component base
//******************************************************************************
class MTSceneComponent11 : public IMTSceneManagedComponent
{
public:

	virtual ~MTSceneComponent11() = default;

	void SetEnable(bool isEnable) { m_isEnable = isEnable; }
	bool IsEnable() const { return m_isEnable; }

protected:

	bool m_isEnable = true;
};
