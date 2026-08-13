//******************************************************************************
//
// MIDITrail / MTNoteDesignRingLive11
//
// Note design class for Ring (Live).
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteDesignRingLive11.h"


//******************************************************************************
// Initialize (pSeqData = NULL wrapper)
//******************************************************************************
int MTNoteDesignRingLive11::Initialize(
		const TCHAR* pSceneName
	)
{
	return MTNoteDesignRing11::Initialize(pSceneName, NULL);
}

//******************************************************************************
// CalcNoteEnvelope (3-phase for Live mode)
//******************************************************************************
MTNoteEnvelopeResult MTNoteDesignRingLive11::CalcNoteEnvelope(
		unsigned long playTimeMSec,
		unsigned long startTime,
		unsigned long endTime
	)
{
	return _CalcLiveEnvelope(
		playTimeMSec, startTime, endTime,
		GetEnvelopeConfig(), GetRippleDecayDuration(), GetRippleReleaseDuration());
}
