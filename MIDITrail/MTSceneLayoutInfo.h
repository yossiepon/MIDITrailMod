//******************************************************************************
//
// MIDITrail / MTSceneLayoutInfo
//
// Layout information shared between UI overlay components.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// Scene layout info (written by Dashboard, read by DiagOverlay)
//******************************************************************************
struct MTSceneLayoutInfo
{
	float titleAreaHeight;
	float counterAreaHeight;

	MTSceneLayoutInfo()
		: titleAreaHeight(0.0f)
		, counterAreaHeight(0.0f)
	{
	}
};
