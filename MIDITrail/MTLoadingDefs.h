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

#include <cstdio>
#include <cstdarg>
#include <share.h>
#include <windows.h>
#include "SMLoadingDefs.h"


//******************************************************************************
// Load log output (open, write, close per call)
//******************************************************************************
inline void MTLoadLog(const char* fmt, ...)
{
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	WCHAR* s = wcsrchr(path, L'\\');
	if (s) *(s + 1) = L'\0';
	wcscat_s(path, MAX_PATH, L"load_log.txt");
	FILE* f = _wfsopen(path, L"a", _SH_DENYNO);
	if (f) {
		va_list args;
		va_start(args, fmt);
		vfprintf(f, fmt, args);
		va_end(args);
		fclose(f);
	}
}

inline void MTLoadLogCreate()
{
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	WCHAR* s = wcsrchr(path, L'\\');
	if (s) *(s + 1) = L'\0';
	wcscat_s(path, MAX_PATH, L"load_log.txt");
	FILE* f = _wfsopen(path, L"w", _SH_DENYNO);
	if (f) fclose(f);
}


//******************************************************************************
// Loading progress band allocation (single source of truth)
// Ratios are 0.0~1.0 within the overall progress bar.
//******************************************************************************
namespace MTLoadBand {
	// MIDITrailApp level: SMFileReader::Load vs Scene::Create
	// Measured with 6.28M notes: Load 16.5s (45%), Scene 20.3s (55%)
	static constexpr float PARSE_END       = 0.45f;  // 0% ~ 45%: SMFileReader::Load
	static constexpr float BUILD_END       = 0.97f;  // 45% ~ 97%: Scene::Create

	// Scene level (within BUILD band, as 0.0~1.0 ratios within Scene)
	// Measured: Grid+TI+Dash 1.3s (6%), Tracker 9.6s (47%), Ripple+Lyrics 0.6s (3%), AABB 6.5s (32%), Keyboard 1.3s (6%)
	static constexpr float TRACKER_START   = 0.06f;  // after Grid/TimeIndicator/Dashboard
	static constexpr float TRACKER_END     = 0.54f;  // NoteTracker
	static constexpr float INSTANCED_START = 0.57f;  // after Ripple/Lyrics
	static constexpr float INSTANCED_END   = 0.89f;  // NoteInstanced
}


//******************************************************************************
// Loading progress context (MIDITrail layer)
// Uses SMLoadProgressFunc directly (MTLoadingProgressFunc is deprecated).
//******************************************************************************
struct MTLoadProgressContext
{
	SMIDILib::SMLoadProgressFunc func;
	void* userData;

	MTLoadProgressContext() : func(NULL), userData(NULL) {}
	MTLoadProgressContext(SMIDILib::SMLoadProgressFunc f, void* u) : func(f), userData(u) {}

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
