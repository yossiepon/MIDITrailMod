//******************************************************************************
//
// Simple MIDI Library / SMTrack
//
// MIDI track class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2025 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// SysEx events and meta events are variable-length, so the simple list
// class cannot be used as-is for them. However, these events are far fewer
// than MIDI events, which always fit within 4 bytes, so individual "new"
// allocations are accepted and they are managed with a map instead.
//
// TODO:
// The SMEvent class should hold the delta time and port number.
// Because the event / delta time / port number are kept separate,
// handling by users of the SMTrack class is more cumbersome than it
// needs to be.

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMSimpleList.h"
#include "SMEvent.h"
#include "SMNoteList.h"
#include <map>

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// Track class
//******************************************************************************
class SMIDILIB_API SMTrack
{

public:

	//Constructor / Destructor
	SMTrack(void);
	virtual ~SMTrack(void);

	//Clear
	void Clear();

	//Register data set
	int AddDataSet(unsigned long deltaTime, SMEvent* pEvent, unsigned char portNo);

	//Get data set
	int GetDataSet(unsigned long index, unsigned long* pDeltaTime, SMEvent* pEvent, unsigned char* pPortNo);

	//Get data set count
	unsigned long GetSize();

	//Get note list: startTime, endTime are in tick time
	int GetNoteList(SMNoteList* pNoteList);

	//Get note list: startTime, endTime are in real time (msec)
	int GetNoteListWithRealTime(SMNoteList* pNoteList, unsigned long timeDivision);

	//Copy
	int CopyFrom(SMTrack* pSrcTrack);


	//Overwrite port number
	int OverwritePortNo(short portNo);

	//Overwrite channel number
	int OverwriteChNo(short chNo);


private:

	//Event data
	typedef struct {
		SMEvent::EventType type;
		unsigned char status;
		unsigned char meta;
		unsigned long size;
		unsigned char data[4];
	} SMEventData;

	//Data set
	typedef struct {
		unsigned long deltaTime;
		SMEventData eventData;
		unsigned char portNo;
	} SMDataSet;

	//Extended data map: index -> data position
	typedef std::map<unsigned long, unsigned char*> SMExDataMap;
	typedef std::pair<unsigned long, unsigned char*> SMExDataMapPair;

	//Note info map: note identification key -> note list index
	typedef std::map<unsigned long, unsigned long> SMNoteMap;
	typedef std::pair<unsigned long, unsigned long> SMNoteMapPair;

private:

	SMSimpleList m_List;
	SMExDataMap m_ExDataMap;

	short m_OverwritePortNo;
	short m_OverwriteChNo;

	unsigned long _GetNoteKey(unsigned char portNo, unsigned char chNo, unsigned char noteNo);
	int _GetNoteList(SMNoteList* pNoteList, unsigned long timeDivision);
	double _ConvTick2TimeMsec(unsigned long tickTime, unsigned long tempo, unsigned long timeDivision);

	//Prohibit assignment and copy constructor
	void operator=(const SMTrack&);
	SMTrack(const SMTrack&);

};

} // end of namespace

#pragma warning(default:4251)

