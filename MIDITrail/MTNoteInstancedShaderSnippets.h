//******************************************************************************
//
// MIDITrail / MTNoteInstancedShaderSnippets
//
// Shared HLSL function snippets for instanced note renderers.
// Used by MTNoteAABBInstanced11 and MTNoteCylindricalInstanced11.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign11.h"

#define MTNOTEINST_HLSL_SHARED_FUNCTIONS \
	"static const float DECAY_SATURATION = " MTNOTEDESIGN_STRINGIFY(MTNOTEDESIGN_DECAY_SATURATION_SMOOTH) ";\n" \
	\
	"float GetDecayCoeff(float rate) {\n" \
	"  float c;\n" \
	"  if (rate < 0.5) {\n" \
	"    c = (exp2((0.5 - rate) * 8.0) + 14.0) / DECAY_SATURATION;\n" \
	"  } else {\n" \
	"    c = (16.0 - exp2((rate - 0.5) * 8.0)) / DECAY_SATURATION;\n" \
	"  }\n" \
	"  return saturate(c);\n" \
	"}\n" \
	\
	"float CalcEnvelope(float playMs, float startMs, float endMs,\n" \
	"                   float decDur, float relDur, float decR, float susR) {\n" \
	"  float noteLen = endMs - startMs;\n" \
	"  float relR = 1.0 - decR - susR;\n" \
	"  if (noteLen < decDur) {\n" \
	"  } else if (noteLen < decDur + relDur) {\n" \
	"    relDur = noteLen - decDur;\n" \
	"    decR = 0.5; susR = 0.0; relR = 0.5;\n" \
	"  } else if (noteLen < (decDur + relDur) * 2.0) {\n" \
	"    float mid = (startMs + decDur + endMs - relDur) * 0.5;\n" \
	"    decDur = mid - startMs;\n" \
	"    relDur = endMs - mid;\n" \
	"    decR = 0.5; susR = 0.0; relR = 0.5;\n" \
	"  }\n" \
	"  float progress = playMs - startMs;\n" \
	"  if (progress < decDur) {\n" \
	"    return (decDur > 0.0) ? (decR * progress / decDur) : 0.0;\n" \
	"  }\n" \
	"  float sustainEnd = noteLen - relDur;\n" \
	"  if (progress <= sustainEnd) {\n" \
	"    float sLen = sustainEnd - decDur;\n" \
	"    return (sLen > 0.0) ? (decR + susR * (progress - decDur) / sLen) : (decR + susR);\n" \
	"  }\n" \
	"  float rProg = progress - sustainEnd;\n" \
	"  return (relDur > 0.0) ? (decR + susR + relR * rProg / relDur) : 1.0;\n" \
	"}\n"
