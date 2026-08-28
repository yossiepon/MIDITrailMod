//******************************************************************************
//
// MIDITrail / MTVersion
//
// Version definitions.
//
// Copyright (C) 2014-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// Base version (frozen at 1.4.1, pre-DX11)
//******************************************************************************
#define MIDITRAIL_VER_MAJOR   1
#define MIDITRAIL_VER_MINOR   4
#define MIDITRAIL_VER_PATCH   1
#define MIDITRAIL_VER_REV     65535

// Auto-derived from numeric macros (no manual sync required)
#define _MT_STRINGIFY(x) #x
#define _MT_TOSTRING(x) _MT_STRINGIFY(x)
#define _MT_WSTRINGIFY(x) L ## #x
#define _MT_TOWSTRING(x) _MT_WSTRINGIFY(x)

#define MIDITRAIL_VER_STRING    _MT_TOSTRING(MIDITRAIL_VER_MAJOR) "." _MT_TOSTRING(MIDITRAIL_VER_MINOR) "." _MT_TOSTRING(MIDITRAIL_VER_PATCH) "." _MT_TOSTRING(MIDITRAIL_VER_REV)
#define MIDITRAIL_VER_STRING_W  _MT_TOWSTRING(MIDITRAIL_VER_MAJOR) L"." _MT_TOWSTRING(MIDITRAIL_VER_MINOR) L"." _MT_TOWSTRING(MIDITRAIL_VER_PATCH) L"." _MT_TOWSTRING(MIDITRAIL_VER_REV)

//******************************************************************************
// Mod version
// Build script (build.sh) auto-updates MOD_DATE_STR and MOD_COPYRIGHT_YEARS.
//******************************************************************************
#define MIDITRAIL_MOD_AUTHOR              "yossiepon"
#define MIDITRAIL_MOD_DATE_STR            "2026-08-28"
#define MIDITRAIL_MOD_STRING              "Mod " MIDITRAIL_MOD_AUTHOR " " MIDITRAIL_MOD_DATE_STR
#define MIDITRAIL_MOD_STRING_W            L"Mod " L"" MIDITRAIL_MOD_AUTHOR L" " L"" MIDITRAIL_MOD_DATE_STR

#define MIDITRAIL_MOD_COPYRIGHT_YEAR_START  2012
#define MIDITRAIL_MOD_COPYRIGHT_YEARS       "2012-2026"

//******************************************************************************
// Display strings (derived from above)
//******************************************************************************
#ifdef _WIN64
#define MIDITRAIL_VERSION_STRING_ARCH  L"x64"
#else
#define MIDITRAIL_VERSION_STRING_ARCH  L"x86"
#endif

#define MIDITRAIL_VERSION_STRING  MIDITRAIL_VER_STRING_W L" (" MIDITRAIL_VERSION_STRING_ARCH L"), " MIDITRAIL_MOD_STRING_W

// Backward compatibility (existing code references X64/X86 variants)
#define MIDITRAIL_VERSION_STRING_X64  MIDITRAIL_VERSION_STRING
#define MIDITRAIL_VERSION_STRING_X86  MIDITRAIL_VERSION_STRING

//******************************************************************************
// Copyright
//******************************************************************************
#define MIDITRAIL_COPYRIGHT              "Copyright (C) 2010-2025 WADA Masashi"
#define MIDITRAIL_COPYRIGHT_W            L"Copyright (C) 2010-2025 WADA Masashi"
#define MIDITRAIL_MOD_COPYRIGHT          "Mod (C) " MIDITRAIL_MOD_COPYRIGHT_YEARS " yossiepon Oniichan"
#define MIDITRAIL_MOD_COPYRIGHT_W        L"Mod (C) " L"" MIDITRAIL_MOD_COPYRIGHT_YEARS L" yossiepon Oniichan"
#define MIDITRAIL_CED_COPYRIGHT          "Portions (C) 2026 Ced (MIDITrail Mod Mod)"
#define MIDITRAIL_CED_COPYRIGHT_W        L"Portions (C) 2026 Ced (MIDITrail Mod Mod)"
