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

#include <windows.h>


namespace SMIDILib {

//******************************************************************************
// Progress callback type for file loading operations
//******************************************************************************
typedef void (*SMLoadProgressFunc)(unsigned long current, unsigned long total, const char* message, void* userData);

} // end of namespace
