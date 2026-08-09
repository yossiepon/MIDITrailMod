//******************************************************************************
//
// MIDITrail / MTNoteLiveBase11
//
// Live note renderer base class (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteLiveBase11.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteLiveBase11::MTNoteLiveBase11()
{
	m_NoteNum = 0;
	m_LiveMonitorDisplayDuration = 30000;
	m_LiveTimeMSec = 0;
	m_pNotePitchBend = NULL;
	_ResetNoteStatus();
}

MTNoteLiveBase11::~MTNoteLiveBase11()
{
}

//******************************************************************************
// Reset
//******************************************************************************
void MTNoteLiveBase11::Reset()
{
	m_NoteNum = 0;
	_ResetNoteStatus();
}

//******************************************************************************
// Note ON
//******************************************************************************
void MTNoteLiveBase11::SetNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	unsigned long clearedIndex = 0;
	bool isFind = false;
	unsigned long curTime = m_LiveTimeMSec;

	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			if ((m_NoteStatus[i].endTime != 0)
				&& ((curTime - m_NoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_NoteStatus[i].isActive = false;
				clearedIndex = i;
				isFind = true;
				break;
			}
		}
		else {
			clearedIndex = i;
			isFind = true;
			break;
		}
	}

	if (!isFind) {
		_ClearOldestNoteStatus(&clearedIndex);
	}

	m_NoteStatus[clearedIndex].isActive = true;
	m_NoteStatus[clearedIndex].portNo = portNo;
	m_NoteStatus[clearedIndex].chNo = chNo;
	m_NoteStatus[clearedIndex].noteNo = noteNo;
	m_NoteStatus[clearedIndex].startTime = m_LiveTimeMSec;
	m_NoteStatus[clearedIndex].endTime = 0;
}

//******************************************************************************
// Note OFF
//******************************************************************************
void MTNoteLiveBase11::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if ((m_NoteStatus[i].isActive)
			&& (m_NoteStatus[i].portNo == portNo)
			&& (m_NoteStatus[i].chNo == chNo)
			&& (m_NoteStatus[i].noteNo == noteNo)
			&& (m_NoteStatus[i].endTime == 0)) {
			m_NoteStatus[i].endTime = m_LiveTimeMSec;
			break;
		}
	}
}

//******************************************************************************
// All Note OFF
//******************************************************************************
void MTNoteLiveBase11::AllNoteOff()
{
	unsigned long curTime = m_LiveTimeMSec;
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if ((m_NoteStatus[i].isActive) && (m_NoteStatus[i].endTime == 0)) {
			m_NoteStatus[i].endTime = curTime;
		}
	}
}

//******************************************************************************
// All Note OFF (channel)
//******************************************************************************
void MTNoteLiveBase11::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	unsigned long curTime = m_LiveTimeMSec;
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if ((m_NoteStatus[i].isActive) && (m_NoteStatus[i].endTime == 0)
			&& (m_NoteStatus[i].portNo == portNo) && (m_NoteStatus[i].chNo == chNo)) {
			m_NoteStatus[i].endTime = curTime;
		}
	}
}

//******************************************************************************
// Update note status (expire old notes)
//******************************************************************************
void MTNoteLiveBase11::_UpdateStatusOfNotes(unsigned long curTimeMs)
{
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			if ((m_NoteStatus[i].endTime != 0)
				&& ((curTimeMs - m_NoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_NoteStatus[i].isActive = false;
				m_NoteStatus[i].startTime = 0;
				m_NoteStatus[i].endTime = 0;
			}
		}
	}
}

//******************************************************************************
// Clear oldest note status
//******************************************************************************
void MTNoteLiveBase11::_ClearOldestNoteStatus(unsigned long* pClearedIndex)
{
	unsigned long oldestIndex = 0;
	bool isFind = false;

	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			if (!isFind) {
				oldestIndex = i;
				isFind = true;
			}
			else if (m_NoteStatus[i].startTime < m_NoteStatus[oldestIndex].startTime) {
				oldestIndex = i;
			}
		}
	}

	m_NoteStatus[oldestIndex].isActive = false;
	*pClearedIndex = oldestIndex;
}

//******************************************************************************
// Reset note status array
//******************************************************************************
void MTNoteLiveBase11::_ResetNoteStatus()
{
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		m_NoteStatus[i].isActive = false;
		m_NoteStatus[i].portNo = 0;
		m_NoteStatus[i].chNo = 0;
		m_NoteStatus[i].noteNo = 0;
		m_NoteStatus[i].startTime = 0;
		m_NoteStatus[i].endTime = 0;
	}
}
