//******************************************************************************
//
// RTDiagLib / RTDiagLib
//
// RTDiagLib public header (DLL export definitions).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef RTDIAGLIB_EXPORTS
#define RTDIAGLIB_API __declspec(dllexport)
#else
#define RTDIAGLIB_API __declspec(dllimport)
#endif
