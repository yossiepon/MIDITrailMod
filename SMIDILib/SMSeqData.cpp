//******************************************************************************
//
// Simple MIDI Library / SMSeqData
//
// Sequence data class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
// Copyright (C) 2026 Ced (MIDITrail Mod Mod). All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventMeta.h"
#include "SMSeqData.h"
#include "SMFPUCtrl.h"
#include <mbctype.h>
#include <vector>
#include <queue>

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMSeqData::SMSeqData()
{
	m_pMergedTrack = NULL;
	Clear();
}

//******************************************************************************
// Destructor
//******************************************************************************
SMSeqData::~SMSeqData(void)
{
	Clear();
}

//******************************************************************************
// Register SMF format
//******************************************************************************
void SMSeqData::SetSMFFormat(
		unsigned long smfFormat
	)
{
	m_SMFFormat = smfFormat;
}

//******************************************************************************
// Register time division
//******************************************************************************
void SMSeqData::SetTimeDivision(
		unsigned long timeDivision
	)
{
	m_TimeDivision = timeDivision;
}

//******************************************************************************
// Register track
//******************************************************************************
int SMSeqData::AddTrack(
		SMTrack* pTrack
	)
{
	m_TrackList.push_back(pTrack);
	return 0;
}

//******************************************************************************
// Track registration complete
//******************************************************************************
int SMSeqData::CloseTrack(
		SMLoadProgressFunc progressFunc,
		void* progressUserData,
		unsigned long progressOffset,
		unsigned long progressTotal
	)
{
	int result = 0;

	//Merge tracks process
	result = _MergeTracks(progressFunc, progressUserData,
						  progressOffset, progressTotal);
	if (result != 0) goto EXIT;

	//Calculate total playback time
	result = _CalcTotalTime();
	if (result != 0) goto EXIT;

	//Get tempo
	result = _GetTempo(&m_Tempo);
	if (result != 0) goto EXIT;

	//Get time signature
	result = _GetBeat(&m_BeatNumerator, &m_BeatDenominator);
	if (result != 0) goto EXIT;

	//Get bar count
	result = _GetBarNum(&m_BarNum);
	if (result != 0) goto EXIT;

	//Get text information
	result = _SearchText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Merge tracks process (k-way merge via min-heap)
//******************************************************************************

namespace {

struct MergeHeapItem {
	unsigned long long absTime;
	unsigned long trackIdx;
};

struct MergeHeapCmp {
	bool operator()(const MergeHeapItem& a, const MergeHeapItem& b) const {
		if (a.absTime != b.absTime) return a.absTime > b.absTime;
		return a.trackIdx > b.trackIdx;
	}
};

} // anonymous namespace

int SMSeqData::_MergeTracks(
		SMLoadProgressFunc progressFunc,
		void* progressUserData,
		unsigned long progressOffset,
		unsigned long progressTotal
	)
{
	int result = 0;
	unsigned char portNo = 0;
	SMEvent event;
	SMTrack* pMergedTrack = NULL;

	delete m_pMergedTrack;
	m_pMergedTrack = NULL;

	try {
		pMergedTrack = new SMTrack();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	{
		// Build indexed track array from std::list
		std::vector<SMTrack*> tracks(m_TrackList.begin(), m_TrackList.end());
		std::vector<unsigned long> idx(tracks.size(), 0);

		// Count total events for progress reporting
		unsigned long totalEvents = 0;
		for (unsigned long t = 0; t < tracks.size(); t++) {
			totalEvents += tracks[t]->GetSize();
		}

		// Initialize min-heap with each track's first event
		std::priority_queue<MergeHeapItem, std::vector<MergeHeapItem>, MergeHeapCmp> heap;

		for (unsigned long t = 0; t < tracks.size(); t++) {
			if (tracks[t]->GetSize() == 0) continue;

			unsigned long deltaTime = 0;
			result = tracks[t]->GetDataSet(0, &deltaTime, NULL, NULL);
			if (result != 0) goto EXIT;

			MergeHeapItem item;
			item.absTime = deltaTime;
			item.trackIdx = t;
			heap.push(item);
			idx[t] = 0;
		}

		unsigned long long prevAbsTime = 0;
		unsigned long mergedCount = 0;
		unsigned long mergeRange = (progressTotal > progressOffset)
			? (progressTotal - progressOffset) : 0;

		while (!heap.empty()) {
			MergeHeapItem top = heap.top();
			heap.pop();

			unsigned long t = top.trackIdx;
			unsigned long deltaTime = (unsigned long)(top.absTime - prevAbsTime);
			prevAbsTime = top.absTime;

			result = tracks[t]->GetDataSet(idx[t], NULL, &event, &portNo);
			if (result != 0) goto EXIT;

			result = pMergedTrack->AddDataSet(deltaTime, &event, portNo);
			if (result != 0) goto EXIT;

			mergedCount++;
			if (progressFunc != NULL && (mergedCount & 0x3FFF) == 0 && totalEvents > 0) {
				unsigned long current = progressOffset
					+ (unsigned long)((unsigned long long)mergedCount * mergeRange / totalEvents);
				char msg[80];
				snprintf(msg, sizeof(msg), "Merging tracks...  (%lu / %lu events)",
				         mergedCount, totalEvents);
				progressFunc(current, progressTotal, msg, progressUserData);
			}

			// Push next event from the same track
			idx[t] += 1;
			if (idx[t] < tracks[t]->GetSize()) {
				unsigned long nextDelta = 0;
				result = tracks[t]->GetDataSet(idx[t], &nextDelta, NULL, NULL);
				if (result != 0) goto EXIT;

				MergeHeapItem next;
				next.absTime = top.absTime + nextDelta;
				next.trackIdx = t;
				heap.push(next);
			}
		}
	}

	m_pMergedTrack = pMergedTrack;

EXIT:;
	if (result != 0) {
		delete pMergedTrack;
		pMergedTrack = NULL;
	}
	return result;
}

//******************************************************************************
// Clear data
//******************************************************************************
void SMSeqData::Clear()
{
	SMTrackListItr itr;

	m_SMFFormat = 0;
	m_TimeDivision = 0;
	m_TotalTickTime = 0;
	m_TotalPlayTime = 0;
	m_Tempo = SM_DEFAULT_TEMPO;
	m_BeatNumerator = SM_DEFAULT_TIME_SIGNATURE_NUMERATOR;
	m_BeatDenominator = SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;
	m_BarNum = 0;
	m_CopyRight = L"";
	m_Title = L"";
	m_FileName = L"";

	delete m_pMergedTrack;
	m_pMergedTrack = NULL;

	for (itr = m_TrackList.begin(); itr != m_TrackList.end(); itr++) {
		delete *itr;
		*itr = NULL;
	}
	m_TrackList.clear();

	return;
}


//******************************************************************************
// Add sequence
//******************************************************************************
void SMSeqData::AddSequence(SMSeqData &other, short portNo, short chNo)
{
	SMTrackListItr itr = other.m_TrackList.begin();
	std::advance(itr, 1);

	for (; itr != other.m_TrackList.end(); itr++) {

		(*itr)->OverwritePortNo(portNo);
		(*itr)->OverwriteChNo(chNo);

		m_TrackList.push_back(*itr);
	}

	other.m_TrackList.clear();

	CloseTrack();

	return;
}


//******************************************************************************
// Get SMF format
//******************************************************************************
unsigned long SMSeqData::GetSMFFormat()
{
	return m_SMFFormat;
}

//******************************************************************************
// Get time division
//******************************************************************************
unsigned long SMSeqData::GetTimeDivision()
{
	return m_TimeDivision;
}

//******************************************************************************
// Get track count
//******************************************************************************
unsigned long SMSeqData::GetTrackNum()
{
	return (unsigned long)m_TrackList.size();
}

//******************************************************************************
// Get track
//******************************************************************************
int SMSeqData::GetTrack(
		unsigned long index,
		SMTrack* pTrack
	)
{
	int result = 0;
	SMTrackListItr itr;
	SMTrack *pSrcTrack;

	if (pTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (index >= GetTrackNum()) {
		result = YN_SET_ERR("Program error.", index, GetTrackNum());
		goto EXIT;
	}

	itr = m_TrackList.begin();
	advance(itr, index);
	pSrcTrack = *itr;

	result = pTrack->CopyFrom(pSrcTrack);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get merged track
//******************************************************************************
int SMSeqData::GetMergedTrack(
		SMTrack* pMergedTrack
	)
{
	int result = 0;

	if (pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = pMergedTrack->CopyFrom(m_pMergedTrack);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get total tick time
//******************************************************************************
unsigned long SMSeqData::GetTotalTickTime()
{
	return m_TotalTickTime;
}

//******************************************************************************
// Get total playback time (msec.)
//******************************************************************************
unsigned long SMSeqData::GetTotalPlayTime()
{
	return m_TotalPlayTime;
}

//******************************************************************************
// Get tempo(μsec.)
//******************************************************************************
unsigned long SMSeqData::GetTempo()
{
	return m_Tempo;
}

//******************************************************************************
// Get tempo(BPM)
//******************************************************************************
unsigned long SMSeqData::GetTempoBPM()
{
	return ((60 * 1000 * 1000) / m_Tempo);
}

//******************************************************************************
// Get time signature: numerator
//******************************************************************************
unsigned long SMSeqData::GetBeatNumerator()
{
	return m_BeatNumerator;
}

//******************************************************************************
// Get time signature: denominator
//******************************************************************************
unsigned long SMSeqData::GetBeatDenominator()
{
	return m_BeatDenominator;
}

//******************************************************************************
// Get bar count
//******************************************************************************
unsigned long SMSeqData::GetBarNum()
{
	return m_BarNum;
}

//******************************************************************************
// Get copyright text
//******************************************************************************
const WCHAR* SMSeqData::GetCopyRight()
{
	return m_CopyRight.c_str();
}

//******************************************************************************
// Get title text
//******************************************************************************
const WCHAR* SMSeqData::GetTitle()
{
	if (m_Title.length() == 0) {
		return m_FileName.c_str();
	}

	return m_Title.c_str();
}

//******************************************************************************
// Calculate total playback time
//******************************************************************************
int SMSeqData::_CalcTotalTime()
{
	int result = 0;	
	unsigned long tempo = 0;
	unsigned long deltaTime = 0;
	unsigned long index = 0;
	double totalPlayTime = 0.0f;
	SMEvent event;
	SMEventMeta metaEvent;
	SMFPUCtrl fpuCtrl;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Set floating-point precision to double
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	tempo = SM_DEFAULT_TEMPO;
	m_TotalTickTime = 0;
	m_TotalPlayTime = 0;

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		//Get data set from track
		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		//Convert delta time to real time and add to playback time
		//  Truncating below 1msec accumulates error, so accumulate using double
		m_TotalTickTime += deltaTime;
		totalPlayTime += _GetDeltaTimeMsec(tempo, deltaTime);

		//Check for tempo update when a meta event appears
		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			if (metaEvent.GetType() == 0x51) {
				tempo = metaEvent.GetTempo();
			}
		}
	}

	m_TotalPlayTime = (unsigned long)totalPlayTime;

	result = fpuCtrl.End();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get delta time (milliseconds)
//******************************************************************************
double SMSeqData::_GetDeltaTimeMsec(
		unsigned long tempo,
		unsigned long deltaTime
	)
{
	double deltaTimeMsec = 0;

	//(1) Division: resolution per quarter note
	//    e.g. 48
	//(2) delta: track data's delta time
	//    Time difference expressed using the division value
	//    If division is 48 and delta time is 24, that's an eighth note's worth of time
	//(3) tempo: tempo setting (microseconds)
	//    Real-time interval of a quarter note
	//
	// Real-time interval corresponding to delta time (milliseconds)
	//  = (delta / division) * tempo / 1000
	//  = (delta * tempo) / (division * 1000)

	deltaTimeMsec = ((double)deltaTime * (double)tempo) / (1000.0 * (double)m_TimeDivision);

	return deltaTimeMsec;
}

//******************************************************************************
// Get tempo
//******************************************************************************
int SMSeqData::_GetTempo(
		unsigned long* pTempo
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	SMEventMeta metaEvent;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Per the MIDI spec, the default tempo is BPM120 = 500msec = 500,000usec
	*pTempo = SM_DEFAULT_TEMPO;

	//Search for tempo from the beginning of the sequence (delta time zero)
	//If not found, the default value is used
	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (deltaTime != 0) break;

		//Ignore anything other than meta events
		if (event.GetType() != SMEvent::EventMeta) continue;

		//Get tempo
		metaEvent.Attach(&event);
		if (metaEvent.GetType() == 0x51) {
			*pTempo = metaEvent.GetTempo();
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get time signature
//******************************************************************************
int SMSeqData::_GetBeat(
		unsigned long* pNumerator,
		unsigned long* pDenominator
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	SMEventMeta metaEvent;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Per the MIDI spec, the default time signature is 4/4
	*pNumerator   = SM_DEFAULT_TIME_SIGNATURE_NUMERATOR;
	*pDenominator = SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;

	//Search for time signature from the beginning of the sequence (delta time zero)
	//If not found, the default value is used
	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (deltaTime != 0) break;

		//Ignore anything other than meta events
		if (event.GetType() != SMEvent::EventMeta) continue;

		//Get time signature
		metaEvent.Attach(&event);
		if (metaEvent.GetType() == 0x58) {
			metaEvent.GetTimeSignature(pNumerator, pDenominator);
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get bar count
//******************************************************************************
int SMSeqData::_GetBarNum(
		unsigned long* pBarNum
	)
{
	int result = 0;
	SMBarList barList;

	result = GetBarList(&barList);
	if (result != 0) goto EXIT;

	*pBarNum = barList.GetSize();

EXIT:;
	return result;
}

//******************************************************************************
// Search for text information
//******************************************************************************
int SMSeqData::_SearchText()
{
	int result = 0;	
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	bool isFoundText = false;
	SMTrackListItr itr;
	SMTrack* pTrack = NULL;
	SMEvent event;
	SMEventMeta metaEvent;
	std::string copyRight;
	std::string title;

	//Do nothing if no tracks exist
	if (m_TrackList.size() == 0) goto EXIT;

	//Reference the first track (Conductor Track)
	itr = m_TrackList.begin();
	pTrack = *itr;

	//Search for copyright notice
	for (index = 0; index < pTrack->GetSize(); index++) {

		result = pTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		//Copyright notice is recorded at delta time zero
		if (deltaTime != 0) break;

		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			if (metaEvent.GetType() == 0x02) {
				result = metaEvent.GetText(&copyRight);
				if (result != 0) goto EXIT;
				break;
			}
		}
	}
	//Convert to wide string
	if (copyRight.length() > 0) {
		result = _StringToWstring(&copyRight, &m_CopyRight);
		if (result != 0) goto EXIT;
	}

	//Search for sequence name
	for (index = 0; index < pTrack->GetSize(); index++) {

		result = pTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			//Arbitrary text
			if ((metaEvent.GetType() == 0x01) && (!isFoundText)) {
				result = metaEvent.GetText(&title);
				if (result != 0) goto EXIT;

				//Sequence name takes priority, so continue searching
				isFoundText = true;
			}
			//Sequence name
			if (metaEvent.GetType() == 0x03) {
				result = metaEvent.GetText(&title);
				if (result != 0) goto EXIT;
				break;
			}
		}
	}
	//Convert to wide string
	if (title.length() > 0) {
		result = _StringToWstring(&title, &m_Title);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get bar list
//******************************************************************************
int SMSeqData::GetBarList(
		SMBarList* pBarList
	)
{
	int result = 0;	
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	unsigned long prevBarTime = 0;
	unsigned long nextBarTime = 0;
	unsigned long totalTickTime = 0;
	unsigned long numerator = 0;
	unsigned long denominator = 0;
	unsigned long tickTimeOfBar = 0;
	SMEvent event;

	if (pBarList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pBarList->Clear();

	//Tick time per bar
	tickTimeOfBar = (SM_DEFAULT_TIME_SIGNATURE_NUMERATOR * m_TimeDivision * 4) / SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;

	//Register as the start point of the first bar
	totalTickTime = 0;
	prevBarTime = totalTickTime;
	result = pBarList->AddBar(totalTickTime);
	if (result != 0) goto EXIT;

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {
		SMEventMeta metaEvent;

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		totalTickTime += deltaTime;

		//Find and register bar boundaries within the elapsed time
		while(true) {
			nextBarTime = prevBarTime + tickTimeOfBar;
			if (nextBarTime <= totalTickTime) {
				pBarList->AddBar(nextBarTime);
				prevBarTime = nextBarTime;
			}
			else {
				break;
			}
		}

		//From here, handle the case when a time signature appears

		//Ignore anything other than meta events
		if (event.GetType() != SMEvent::EventMeta) continue;

		//Ignore anything other than time signature
		metaEvent.Attach(&event);
		if (metaEvent.GetType() != 0x58) continue;

		//Get time signature
		metaEvent.GetTimeSignature(&numerator, &denominator);
		if (denominator == 0) {
			//Data error
			result = YN_SET_ERR("Invalid data found.", index, numerator);
			goto EXIT;
		}

		//Update tick time per bar
		tickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

		//Register as the start point of the first bar due to time signature update
		if (prevBarTime != totalTickTime) {
			prevBarTime = totalTickTime;
			result = pBarList->AddBar(totalTickTime);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get port list
//******************************************************************************
int SMSeqData::GetPortList(
		SMPortList* pPortList
	)
{
	int result = 0;	
	unsigned long index = 0;
	unsigned char portNo = 0;
	unsigned char port[256];
	SMEvent event;

	if (pPortList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pPortList->Clear();

	for (index = 0; index < 256; index++) {
		port[index] = 0;
	}

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {
		result = m_pMergedTrack->GetDataSet(index, NULL, &event, &portNo);
		if (result != 0) goto EXIT;

		port[portNo] = 1;
	}

	for (index = 0; index < 256; index++) {
		if (port[index] != 0) {
			pPortList->AddPort((unsigned char)index);
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Register file name
//******************************************************************************
void SMSeqData::SetFileName(
		const WCHAR* pFileName
	)
{
	m_FileName = pFileName;
	return;
}

//******************************************************************************
// Get file name
//******************************************************************************
const WCHAR* SMSeqData::GetFileName()
{
	return m_FileName.c_str();
}

//******************************************************************************
// Convert to wide string
//******************************************************************************
int SMSeqData::_StringToWstring(std::string* pStr, std::wstring* pWstr)
{
	int result = 0;
	int apiresult = 0;
	int buffSize = 0;
	WCHAR* wstrBuff = NULL;

	//No conversion if the string is empty
	if (pStr->length() == 0) {
		*pWstr = L"";
		goto EXIT;
	}

	//Buffer size accounting for surrogate pairs and null terminator
	buffSize = (int)(pStr->length()) * 2 + 1;

	try {
		wstrBuff = new WCHAR[buffSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", buffSize, 0);
		goto EXIT;
	}

	memset(wstrBuff, 0, sizeof(WCHAR) * buffSize);

	apiresult = MultiByteToWideChar(
						_getmbcp(),			//Code page
						MB_PRECOMPOSED,		//Flags:
						pStr->c_str(),		//Source multibyte string
						(int)(pStr->length()),	//Source multibyte string byte count
						wstrBuff,			//Destination wide string buffer
						buffSize - 1		//Buffer size (in wide characters)
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	*pWstr = wstrBuff;

EXIT:;
	delete [] wstrBuff;
	return result;
}

int SMSeqData::StringToWstring(std::string* pStr, std::wstring* pWstr)
{
	return _StringToWstring(pStr, pWstr);
}

} // end of namespace

