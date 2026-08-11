//******************************************************************************
//
// MIDITrail / MTNoteDesignRingLive11
//
// Ring note design class for Live mode.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
// CalcNoteEnvelope (linear decay for Live mode)
// Same logic as MTNoteDesignLive11 — uses RippleDecayDuration for timing.
//******************************************************************************
MTNoteEnvelopeResult MTNoteDesignRingLive11::CalcNoteEnvelope(
		unsigned long playTimeMSec,
		unsigned long startTime,
		unsigned long endTime
	)
{
	MTNoteEnvelopeResult result;
	result.keyDownRate = 0.0f;
	result.keyStatus = BeforeNoteON;

	if (playTimeMSec < startTime) {
		return result;
	}

	unsigned long elapsed = playTimeMSec - startTime;

	if (endTime == 0 || playTimeMSec <= endTime) {
		unsigned long decayDuration = GetRippleDecayDuration();
		if (decayDuration > 0 && elapsed < decayDuration) {
			result.keyDownRate = (float)elapsed / (float)decayDuration;
		}
		else {
			result.keyDownRate = 1.0f;
		}
		result.keyStatus = NoteON;
	}
	else {
		unsigned long releaseDuration = GetRippleReleaseDuration();
		unsigned long releaseElapsed = playTimeMSec - endTime;
		if (releaseDuration > 0 && releaseElapsed < releaseDuration) {
			result.keyDownRate = 1.0f - ((float)releaseElapsed / (float)releaseDuration);
		}
		else {
			result.keyDownRate = 0.0f;
		}
		result.keyStatus = AfterNoteOFF;
	}

	return result;
}
