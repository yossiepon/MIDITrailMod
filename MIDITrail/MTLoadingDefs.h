//******************************************************************************
//
// MIDITrail / MTLoadingDefs
//
// Loading progress definitions.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// Loading progress callback type (MIDITrail layer)
//******************************************************************************
typedef void (*MTLoadingProgressFunc)(
	unsigned long current,
	unsigned long total,
	const char* message,
	void* userData
);
