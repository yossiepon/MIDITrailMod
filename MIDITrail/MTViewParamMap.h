//******************************************************************************
//
// MIDITrail / MTViewParamMap
//
// View parameter map type definition.
// Separated from IMTScene11.h so that camera and other classes can use it
// without pulling in the full scene interface.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <string>
#include <map>

typedef std::map<std::string, float>  MTViewParamMap;
typedef std::pair<std::string, float> MTViewParamMapPair;
