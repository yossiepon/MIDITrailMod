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


//******************************************************************************
// Progress band wrapper (maps local 0~count to a sub-range of the parent)
//******************************************************************************
struct MTProgressBand
{
	const MTLoadProgressContext* pParent;
	float startRatio;
	float endRatio;

	static void Callback(unsigned long current, unsigned long total,
	                      const char* message, void* userData)
	{
		auto* band = static_cast<MTProgressBand*>(userData);
		if (band == NULL || band->pParent == NULL) return;
		float local = (total > 0) ? (float)current / (float)total : 1.0f;
		float global = band->startRatio + local * (band->endRatio - band->startRatio);
		unsigned long globalCurrent = (unsigned long)(global * 10000.0f);
		band->pParent->Fire(globalCurrent, 10000, message);
	}

	MTLoadProgressContext ToContext()
	{
		return MTLoadProgressContext(&Callback, this);
	}
};
