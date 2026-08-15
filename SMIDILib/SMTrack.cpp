//******************************************************************************
//
// Simple MIDI Library / SMTrack
//
// MIDI track class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMCommon.h"
#include "SMTrack.h"
#include "SMEventMIDI.h"
#include "SMEventMeta.h"
#include "SMFPUCtrl.h"
#include <vector>

#include "SMSeqData.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMTrack::SMTrack(void)
	: m_List(sizeof(SMDataSet), 1000), m_OverwritePortNo(-1), m_OverwriteChNo(-1)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
SMTrack::~SMTrack(void)
{
	Clear();
}

//******************************************************************************
// Clear data
//******************************************************************************
void SMTrack::Clear()
{
	SMExDataMap::iterator exdataitr;

	m_List.Clear();

	for (exdataitr = m_ExDataMap.begin(); exdataitr != m_ExDataMap.end(); exdataitr++) {
		delete [] (exdataitr->second);
	}
	m_ExDataMap.clear();

	m_OverwritePortNo = -1;

	return;
}

//******************************************************************************
// Add data set
//******************************************************************************
int SMTrack::AddDataSet(
		unsigned long deltaTime,
		SMEvent* pEvent,
		unsigned char portNo
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned char* pExData = NULL;
	SMDataSet dataSet;

	index = m_List.GetSize();

	// Create data set
	ZeroMemory(&dataSet, sizeof(SMDataSet));
	dataSet.deltaTime = deltaTime;
	dataSet.eventData.type   = pEvent->GetType();
	dataSet.eventData.status = pEvent->GetStatus();
	dataSet.eventData.meta   = pEvent->GetMetaType();
	dataSet.eventData.size   = pEvent->GetDataSize();
	dataSet.portNo = portNo;

	// If event data is 4 bytes or less, store it inside the structure
	if (pEvent->GetDataSize() <= 4) {
		memcpy(&(dataSet.eventData.data), pEvent->GetDataPtr(), pEvent->GetDataSize());
	}
	// Otherwise keep it on the heap separately and manage it in a map
	else {
		try {
			pExData = new unsigned char[pEvent->GetDataSize()];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", pEvent->GetDataSize(), 0);
			goto EXIT;
		}
		memcpy(pExData, pEvent->GetDataPtr(), pEvent->GetDataSize());
		m_ExDataMap.insert(SMExDataMapPair(index, pExData));
		pExData = NULL;
	}

	result = m_List.AddItem(&dataSet);
	if (result != 0) goto EXIT;

EXIT:;
	delete [] pExData;
	return result;
}

//******************************************************************************
// Get data set
//******************************************************************************
int SMTrack::GetDataSet(
		unsigned long index,
		unsigned long* pDeltaTime,
		SMEvent* pEvent,
		unsigned char* pPortNo
	)
{
	int result = 0;
	unsigned char* pEventData = NULL;
	SMDataSet dataSet;
	SMExDataMap::iterator exdataitr;

	result = m_List.GetItem(index, &dataSet);
	if (result != 0) goto EXIT;

	// Delta time
	if (pDeltaTime != NULL) {
		*pDeltaTime = dataSet.deltaTime;
	}

	// Event data position
	if (dataSet.eventData.size <= 4) {
		pEventData = dataSet.eventData.data;
	}
	else {
		exdataitr = m_ExDataMap.find(index);
		if (exdataitr == m_ExDataMap.end()) {
			result = YN_SET_ERR("Program error.", index, 0);
			goto EXIT;
		}
		pEventData = exdataitr->second;
	}

	// Event Register data
	if (pEvent != NULL) {
		result = pEvent->SetData(
						dataSet.eventData.type,
						dataSet.eventData.status,
						dataSet.eventData.meta,
						pEventData,
						dataSet.eventData.size
					);
		if (result != 0) goto EXIT;

		// If channel number overwrite is specified and this is a MIDI event
		if ((m_OverwriteChNo != -1) && (pEvent->GetType() == SMEvent::EventMIDI)) {
			// Overwrite the channel number
			unsigned char status = pEvent->GetStatus();
			status = (status & 0xf0) | (m_OverwriteChNo & 0x0f);
			pEvent->SetStatus(status);
		}
	}

	// Port number
	if (pPortNo != NULL) {
		if(m_OverwritePortNo == -1) {
			*pPortNo = dataSet.portNo;
		} else {
			*pPortNo = (unsigned char)m_OverwritePortNo;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get size
//******************************************************************************
unsigned long SMTrack::GetSize()
{
	return m_List.GetSize();
}

//******************************************************************************
// Copy
//******************************************************************************
int SMTrack::CopyFrom(
		SMTrack* pSrcTrack
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	unsigned char portNo = 0;

	//TODO: make the copy a bit more intelligent

	if (pSrcTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	// If the copy source is itself, do nothing
	if (pSrcTrack == this) {
		goto EXIT;
	}

	Clear();

	for (index = 0; index < pSrcTrack->GetSize(); index++) {
		result = pSrcTrack->GetDataSet(index, &deltaTime, &event, &portNo);
		if (result != 0) goto EXIT;

		result = AddDataSet(deltaTime, &event, portNo);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}


//******************************************************************************
// Specify port number overwrite
//******************************************************************************
int SMTrack::OverwritePortNo(short portNo)
{
	int result = 0;

	m_OverwritePortNo = portNo;

	return result;
}


//******************************************************************************
// Specify channel number overwrite
//******************************************************************************
int SMTrack::OverwriteChNo(short chNo)
{
	int result = 0;

	m_OverwriteChNo = chNo;

	return result;
}



//******************************************************************************
// Get note list
//******************************************************************************
int SMTrack::GetNoteList(
		SMNoteList* pNoteList
	)
{
	int result = 0;

	result = _GetNoteList(pNoteList, 0);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get note list
//******************************************************************************
int SMTrack::GetNoteListWithRealTime(
		SMNoteList* pNoteList,
		unsigned long timeDivision
	)
{
	int result = 0;

	if (timeDivision == 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = _GetNoteList(pNoteList, timeDivision);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get note list
//******************************************************************************
int SMTrack::_GetNoteList(
		SMNoteList* pNoteList,
		unsigned long timeDivision
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	unsigned long totalTime = 0;
	unsigned char portNo = 0;
	unsigned long key = 0;
	unsigned long tempo = SM_DEFAULT_TEMPO;
	double totalRealtime = 0;
	static const unsigned long ACTIVE_SENTINEL = 0xFFFFFFFF;
	static const unsigned long ACTIVE_SIZE = 256 * 16 * 128;
	std::vector<unsigned long> active(ACTIVE_SIZE, ACTIVE_SENTINEL);
	unsigned long noteIdx = 0;
	SMNote note;
	SMEvent event;
	SMEventMIDI midiEvent;
	SMEventMeta metaEvent;
	SMFPUCtrl fpuCtrl;

	// If timeDivision  = 0: startTime, endTime are set as tick time
	// If timeDivision != 0: startTime, endTime are set as real time (msec)

	if (pNoteList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	// Set floating-point arithmetic precision to double precision
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	// Note info is added to the note list in track registration order
	// i.e. the list is built so it is sorted by note start tick time
	pNoteList->Clear();

	for (index = 0; index < GetSize(); index++) {

		result = GetDataSet(index, &deltaTime, &event, &portNo);
		if (result != 0) goto EXIT;

		totalTime += deltaTime;
		totalRealtime += _ConvTick2TimeMsec(deltaTime, tempo, timeDivision);


		// META event
		if (event.GetType() == SMEvent::EventMeta) {

			metaEvent.Attach(&event);

			if (metaEvent.GetType() == 0x51) {
				// Set tempo
				tempo = metaEvent.GetTempo();
			} else if (metaEvent.GetType() == 0x05) {

				// If no note has been added yet, skip the lyric (nothing to link to)
				if (pNoteList->GetSize() == 0) continue;

				// Get the last note
				result = pNoteList->GetNote(pNoteList->GetSize() - 1, &note);
				if (result != 0) goto EXIT;

				// Get the lyric
				std::string lyric;

				result = metaEvent.GetText(&lyric);
				if (result != 0) goto EXIT;

				// Store the lyric if its first character is SPC (0x20) or later
				if( (lyric.length() > 0) && (((unsigned char)lyric.c_str()[0]) > 0x20) ) {

					std::wstring lyricW;

					result = SMSeqData::StringToWstring(&lyric, &lyricW);
					if (result != 0) goto EXIT;

					::wcsncpy_s(&note.lyric[0], std::size(note.lyric), lyricW.c_str(), _TRUNCATE);

					result = pNoteList->SetNote(pNoteList->GetSize() - 1, &note);
					if (result != 0) goto EXIT;
				}

			}
		}


		// Skip anything that isn't a MIDI event
		if (event.GetType() != SMEvent::EventMIDI) continue;

		midiEvent.Attach(&event);

		// Note on
		if (midiEvent.GetChMsg() == SMEventMIDI::NoteOn) {
			key = _GetNoteKey(portNo, midiEvent.GetChNo(), midiEvent.GetNoteNo());
			noteIdx = active[key];

			if (noteIdx == ACTIVE_SENTINEL) {
				// Not yet registered
				note.portNo = portNo;
				note.chNo = midiEvent.GetChNo();
				note.noteNo = midiEvent.GetNoteNo();
				note.velocity = midiEvent.GetVelocity();
				note.startTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				note.endTime = 0;
				note.lyric[0] = L'\0';
			}
			else {
				// Already registered — retrigger: cut and start new note
				result = pNoteList->GetNote(noteIdx, &note);
				if (result != 0) goto EXIT;

				note.endTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				result = pNoteList->SetNote(noteIdx, &note);
				if (result != 0) goto EXIT;

				note.velocity = midiEvent.GetVelocity();
				note.startTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				note.endTime = 0;
			}
			pNoteList->AddNote(note);
			active[key] = pNoteList->GetSize() - 1;
		}
		// Note off
		if (midiEvent.GetChMsg() == SMEventMIDI::NoteOff) {
			key = _GetNoteKey(portNo, midiEvent.GetChNo(), midiEvent.GetNoteNo());
			noteIdx = active[key];

			if (noteIdx != ACTIVE_SENTINEL) {
				result = pNoteList->GetNote(noteIdx, &note);
				if (result != 0) goto EXIT;

				note.endTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				result = pNoteList->SetNote(noteIdx, &note);
				if (result != 0) goto EXIT;

				active[key] = ACTIVE_SENTINEL;
			}
		}
	}

	// If a note is still on when processing ends, cut it
	for (unsigned long k = 0; k < ACTIVE_SIZE; k++) {
		if (active[k] == ACTIVE_SENTINEL) continue;

		result = pNoteList->GetNote(active[k], &note);
		if (result != 0) goto EXIT;

		note.endTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
		result = pNoteList->SetNote(active[k], &note);
		if (result != 0) goto EXIT;
	}

	result = fpuCtrl.End();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Convert tick time to real time (milliseconds)
//******************************************************************************
double SMTrack::_ConvTick2TimeMsec(
		unsigned long tickTime,
		unsigned long tempo,
		unsigned long timeDivision
	)
{
	double timeMsec = 0;

	//(1) resolution per quarter note: division
	//    e.g.: 48
	//(2) delta time of track data: delta
	//    time difference expressed using the resolution value
	//    if resolution is 48 and delta time is 24, that's an eighth note's worth of time
	//(3) tempo setting (microseconds): tempo
	//    real time interval of a quarter note
	//
	// Real time interval corresponding to the delta time (milliseconds)
	//  = (delta / division) * tempo / 1000
	//  = (delta * tempo) / (division * 1000)

	timeMsec = ((double)tickTime * (double)tempo) / (1000.0 * (double)timeDivision);

	return timeMsec;
}

//******************************************************************************
// Get note identification key
//******************************************************************************
unsigned long SMTrack::_GetNoteKey(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	if(m_OverwritePortNo != -1) {
		portNo = (unsigned char)m_OverwritePortNo;
	}

	return ((portNo << 16) | (chNo << 8) | noteNo);
}

} // end of namespace

