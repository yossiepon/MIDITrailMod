//******************************************************************************
//
// MIDITrail / MTNoteDesignLive11
//
// Note design class for Live mode.
// Wraps MTNoteDesign11 with pSeqData=NULL initialization,
// overrides CalcNoteEnvelope with linear decay for Live.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
