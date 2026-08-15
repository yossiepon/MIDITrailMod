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


//******************************************************************************
// Loading progress context (MIDITrail layer)
//******************************************************************************
struct MTLoadProgressContext
{
	MTLoadingProgressFunc func;
	void* userData;

	MTLoadProgressContext() : func(NULL), userData(NULL) {}
	MTLoadProgressContext(MTLoadingProgressFunc f, void* u) : func(f), userData(u) {}

	void Fire(unsigned long current, unsigned long total, const char* message = NULL) const
	{
		if (func != NULL) {
			func(current, total, message, userData);
		}
	}
};
