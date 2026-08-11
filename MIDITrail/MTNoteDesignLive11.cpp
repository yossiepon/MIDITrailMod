//******************************************************************************
//
// MIDITrail / MTNoteDesignLive11
//
// Note design class for Live mode.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
// Decay: 0 → decayRatio over DecayDuration (fixed time)
// Sustain: hold at midpoint while note is held
// Release: fade from sustain level to 0 over ReleaseDuration (after NoteOff)
//******************************************************************************
MTNoteEnvelopeResult MTNoteDesignLive11::CalcNoteEnvelope(
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

	MTEnvelopeConfig envConfig = GetEnvelopeConfig();
	float decayRatio = envConfig.decayRatio;
	float sustainRatio = envConfig.sustainRatio;
	float sustainMid = decayRatio + sustainRatio * 0.5f;

	unsigned long decayDuration = GetRippleDecayDuration();
	unsigned long releaseDuration = GetRippleReleaseDuration();
	unsigned long elapsed = playTimeMSec - startTime;

	if (endTime == 0 || playTimeMSec <= endTime) {
		// Phase 1: Decay
		if (decayDuration > 0 && elapsed < decayDuration) {
			result.keyDownRate = decayRatio * (float)elapsed / (float)decayDuration;
			result.keyStatus = BeforeNoteON;
		}
		// Phase 2: Sustain (hold at midpoint)
		else {
			result.keyDownRate = sustainMid;
			result.keyStatus = NoteON;
		}
	}
	else {
		// Phase 3: Release (expand from sustain level to 1.0, then done)
		unsigned long releaseElapsed = playTimeMSec - endTime;
		float releaseRatio = 1.0f - sustainMid;
		if (releaseDuration > 0 && releaseElapsed < releaseDuration) {
			result.keyDownRate = sustainMid + releaseRatio * (float)releaseElapsed / (float)releaseDuration;
			result.keyStatus = AfterNoteOFF;
		}
		else {
			result.keyDownRate = 0.0f;
			result.keyStatus = AfterNoteOFF;
		}
	}

	return result;
}
