//******************************************************************************
//
// MIDITrail / MTNoteTrackerBase
//
// Note tracker base class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <vector>
#include <cstdint>
#include "SMIDILib.h"
#include "IMTSceneManagedComponent.h"

using namespace SMIDILib;


//******************************************************************************
// Note event type
//******************************************************************************
enum class NoteEventType {
	Note,
	Lyric
};

//******************************************************************************
// Unified note data (tick + ms)
//******************************************************************************
struct NoteData {
	unsigned char portNo;
	unsigned char chNo;
	unsigned char noteNo;
	unsigned char velocity;
	unsigned long startTimeTick;
	unsigned long endTimeTick;
	unsigned long startTimeMs;
	unsigned long endTimeMs;
	WCHAR lyric[17];
};

//******************************************************************************
// Note tracker listener interface
//******************************************************************************
class IMTNoteTrackerListener {
public:
	virtual ~IMTNoteTrackerListener() = default;
	virtual void OnNoteActivate(const NoteData& note, unsigned long index) = 0;
	virtual void OnNoteDeactivate(const NoteData& note, unsigned long index) = 0;
	virtual void OnReset() = 0;
};

//******************************************************************************
// Note tracker base class
//******************************************************************************
class MTNoteTrackerBase : public IMTSceneManagedComponent
{
public:

	MTNoteTrackerBase();
	virtual ~MTNoteTrackerBase();

	void AddListener(IMTNoteTrackerListener* pListener, NoteEventType filter);
	void RemoveListener(IMTNoteTrackerListener* pListener);

	bool HasActiveNotesOnPort(unsigned char portNo) const;
	bool HasActiveNotesOnChannel(unsigned char portNo, unsigned char chNo) const;
	unsigned short GetActiveChannelMask(unsigned char portNo) const;
	unsigned long GetTotalActiveNoteCount() const;

protected:

	struct ListenerEntry {
		IMTNoteTrackerListener* pListener;
		NoteEventType filter;
	};

	void DispatchActivate(const NoteData& note, unsigned long index);
	void DispatchDeactivate(const NoteData& note, unsigned long index);
	void DispatchReset();

	std::vector<ListenerEntry> m_Listeners;
	unsigned long m_ActiveNotesPerCh[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned long m_TotalActiveNotes;

	std::vector<uint8_t> m_ActivatedFlags;

	void SetNoteCount(unsigned long noteCount);
};
