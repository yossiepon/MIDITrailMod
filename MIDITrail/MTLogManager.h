//******************************************************************************
//
// MIDITrail / MTLogManager
//
// Log manager class.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <string>

//******************************************************************************
// Log manager class
//******************************************************************************
class MTLogManager
{
public:

	static int Initialize(const WCHAR* pLogLevelOverride = nullptr);
	static void Terminate();

private:

	MTLogManager() = delete;

	static int _DetermineLogDir(WCHAR* pLogDirPath, unsigned long bufSize);
	static int _GetExeBaseName(WCHAR* pBaseName, unsigned long bufSize);

};
