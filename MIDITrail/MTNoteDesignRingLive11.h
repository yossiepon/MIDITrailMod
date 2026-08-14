//******************************************************************************
//
// MIDITrail / MTNoteDesignRingLive11
//
// Note design class for Ring (Live).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesignRing11.h"


//******************************************************************************
// Ring note design class (Live mode)
//******************************************************************************
class MTNoteDesignRingLive11 : public MTNoteDesignRing11
{
public:

	int Initialize(const TCHAR* pSceneName);

	MTNoteEnvelopeResult CalcNoteEnvelope(
				unsigned long playTimeMSec,
				unsigned long startTime,
				unsigned long endTime
			) override;
};
