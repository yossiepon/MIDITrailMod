//******************************************************************************
//
// MIDITrail / MTNoteEffect
//
// Note effect base class.
// Common base for Ripple and Lyrics: slot-based active note management,
// envelope calculation via MTNoteDesignMod, m_KeyDownRate dedup array.
// Derived classes implement OnActivate/OnDeactivate/BuildVertices.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "MTNoteTracker.h"
#include "MTNoteDesignMod.h"

//******************************************************************************
// Constants
//******************************************************************************
#define NOTEEFFECT_MAX_SLOTS  (100)
#define NOTEEFFECT_MAX_PORT   (8)

//******************************************************************************
// Note effect status (per-slot state)
//******************************************************************************
struct NoteEffectStatus {
	bool isActive;
	MTKeyStatus keyStatus;
	float keyDownRate;
	unsigned long index;

	unsigned char portNo;
	unsigned char chNo;
	unsigned char noteNo;
	unsigned char velocity;
	unsigned long startTimeMs;
	unsigned long endTimeMs;
	WCHAR lyric[17];
};

//******************************************************************************
// Note effect base class
//******************************************************************************
class MTNoteEffect : public MTSceneComponent11, public IMTNoteTrackerListener
{
public:

	MTNoteEffect();
	virtual ~MTNoteEffect();

	virtual int Create(const TCHAR* pSceneName, SMSeqData* pSeqData);
	virtual void Release();

	// IMTNoteTrackerListener
	void OnNoteActivate(const NoteData& note, unsigned long index) override;
	void OnReset() override;

	int Update(const MTSceneUpdateContext& ctx) override;

	void SetSkipStatus(bool isSkipping) { m_isSkipping = isSkipping; }

protected:

	// Template Method hooks for derived classes
	virtual int OnActivate(NoteEffectStatus& status) = 0;
	virtual int OnDeactivate(NoteEffectStatus& status) = 0;
	virtual int BuildVertices(unsigned long playTimeMSec) = 0;

	NoteEffectStatus m_Status[NOTEEFFECT_MAX_SLOTS];
	float m_KeyDownRate[NOTEEFFECT_MAX_PORT][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];
	MTNoteDesignMod m_NoteDesign;
	unsigned long m_CurTickTime;
	unsigned long m_PlayTimeMSec;
	float m_RollAngle;
	bool m_isSkipping;
};
