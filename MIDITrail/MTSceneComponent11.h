//******************************************************************************
//
// MIDITrail / MTSceneComponent11
//
// Visual scene component base class.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
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
