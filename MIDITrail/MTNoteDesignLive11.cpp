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
// CalcNoteEnvelope (linear decay for Live mode)
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

	unsigned long elapsed = playTimeMSec - startTime;

	if (endTime == 0 || playTimeMSec <= endTime) {
		// Note is still sounding
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
		// Note has ended — linear fade from keyDownRate at release to 1.0
		unsigned long releaseDuration = GetRippleDecayDuration();
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
