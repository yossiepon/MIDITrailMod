//******************************************************************************
//
// MIDITrail / MTNoteDesignRingLive11
//
// Ring note design class for Live mode.
// Wraps MTNoteDesignRing11 with pSeqData=NULL initialization,
// overrides CalcNoteEnvelope with linear decay for Live.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
