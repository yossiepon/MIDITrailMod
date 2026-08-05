//******************************************************************************
//
// MIDITrail / MTSceneComponent11
//
// Common base for DX11 scene components.
// Provides a standard Update interface and shared enable/disable state.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// DX11 scene component base
//******************************************************************************
class MTSceneComponent11
{
public:

	virtual ~MTSceneComponent11() = default;

	virtual void Update(unsigned long curTickTime, unsigned long playTimeMSec) {}

	void SetEnable(bool isEnable) { m_isEnable = isEnable; }
	bool IsEnable() const { return m_isEnable; }

protected:

	bool m_isEnable = true;
};
