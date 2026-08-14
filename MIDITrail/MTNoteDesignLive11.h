//******************************************************************************
//
// MIDITrail / MTNoteDesignLive11
//
// Note design class (Live).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign11.h"


//******************************************************************************
// Note design class (Live mode)
//******************************************************************************
class MTNoteDesignLive11 : public MTNoteDesign11
{
public:

	int Initialize(const TCHAR* pSceneName);

	MTNoteEnvelopeResult CalcNoteEnvelope(
				unsigned long playTimeMSec,
				unsigned long startTime,
				unsigned long endTime
			) override;
};
