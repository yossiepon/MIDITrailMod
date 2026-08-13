//******************************************************************************
//
// MIDITrail / MTNoteDesignLive11
//
// Note design class (Live).
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteDesignLive11.h"


//******************************************************************************
// Initialize (pSeqData = NULL wrapper)
//******************************************************************************
int MTNoteDesignLive11::Initialize(
		const TCHAR* pSceneName
	)
{
	return MTNoteDesign11::Initialize(pSceneName, NULL);
}

//******************************************************************************
// CalcNoteEnvelope (3-phase for Live mode)
//******************************************************************************
MTNoteEnvelopeResult MTNoteDesignLive11::CalcNoteEnvelope(
		unsigned long playTimeMSec,
		unsigned long startTime,
		unsigned long endTime
	)
{
	return _CalcLiveEnvelope(
		playTimeMSec, startTime, endTime,
		GetEnvelopeConfig(), GetRippleDecayDuration(), GetRippleReleaseDuration());
}
