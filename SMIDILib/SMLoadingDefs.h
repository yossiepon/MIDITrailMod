//******************************************************************************
//
// Simple MIDI Library / SMLoadingDefs
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


namespace SMIDILib {

//******************************************************************************
// Progress callback type for file loading operations
//******************************************************************************
typedef void (*SMLoadProgressFunc)(unsigned long current, unsigned long total, void* userData);

//******************************************************************************
// Load log output (open, write, close per call)
//******************************************************************************
inline void SMLoadLog(const char* fmt, ...)
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

} // end of namespace
