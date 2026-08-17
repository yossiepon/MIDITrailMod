//******************************************************************************
//
// MIDITrail / MTNoteEffect
//
// Note effect base class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <unordered_map>
#include <vector>
#include "MTSceneComponent11.h"
#include "MTNoteTrackerBase.h"
#include "MTNoteDesign11.h"

//******************************************************************************
// Constants
//******************************************************************************
#define NOTEEFFECT_MAX_SLOTS  (16384)
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

	virtual int Create(const TCHAR* pSceneName, SMSeqData* pSeqData,
	                   MTNoteDesign11* pNoteDesign = NULL);
	virtual void Release();

	// IMTNoteTrackerListener
	void OnNoteActivate(const NoteData& note, unsigned long index) override;
	void OnNoteDeactivate(const NoteData& note, unsigned long index) override;
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
	MTNoteDesign11 m_NoteDesignLocal;
	MTNoteDesign11* m_pNoteDesign;
	unsigned long m_CurTickTime;
	unsigned long m_PlayTimeMSec;
	float m_RollAngle;
	bool m_isSkipping;

	int m_FreeStack[NOTEEFFECT_MAX_SLOTS];
	int m_FreeCount;
	std::unordered_map<unsigned long, int> m_IndexToSlot;
	std::vector<int> m_ActiveSlots;
	int m_SlotToActivePos[NOTEEFFECT_MAX_SLOTS];

	void _InitFreeList();
	void _ReleaseSlot(int slotIndex);
};
