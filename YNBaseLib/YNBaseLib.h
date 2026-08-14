//******************************************************************************
//
// YN Base Library / YNBaseLib
//
// Base utility library public header.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

#include "YNErrInfo.h"
#include "YNErrCtrl.h"
#include "YNConfFile.h"
#include "YNPathUtil.h"

