//******************************************************************************
//
// MIDITrail / MTNoteTracker
//
// Note tracker class.
// Holds unified note list, performs ms-based forward scan,
// and notifies listeners of note activation events.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <vector>
#include "SMIDILib.h"

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
	virtual void OnReset() = 0;
};

//******************************************************************************
// Note tracker class
//******************************************************************************
class MTNoteTracker
{
public:

	MTNoteTracker();
	~MTNoteTracker();

	int Create(SMSeqData* pSeqData);
	void Release();

	void Update(unsigned long playTimeMSec);
	void Seek(unsigned long playTimeMSec);

	unsigned long GetNoteCount() const;
	const NoteData& GetNote(unsigned long index) const;

	void AddListener(
				IMTNoteTrackerListener* pListener,
				NoteEventType filter,
				unsigned long preMarginMs = 0,
				unsigned long postMarginMs = 0
			);
	void RemoveListener(IMTNoteTrackerListener* pListener);

private:

	struct ListenerEntry {
		IMTNoteTrackerListener* pListener;
		NoteEventType filter;
		unsigned long preMarginMs;
		unsigned long postMarginMs;
	};

	std::vector<NoteData> m_Notes;
	unsigned long m_CurNoteIndex;
	unsigned long m_PlayTimeMSec;
	std::vector<ListenerEntry> m_Listeners;
	unsigned long m_MaxPreMargin;

	void _UpdateMaxPreMargin();
};
