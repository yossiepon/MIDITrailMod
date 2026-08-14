//******************************************************************************
//
// Simple MIDI Library / SMNoteList
//
// Note list class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMSimpleList.h"

namespace SMIDILib {


//******************************************************************************
// Note info structure
//******************************************************************************
//Note info
typedef struct {
	unsigned char portNo;
	unsigned char chNo;
	unsigned char noteNo;
	unsigned char velocity;
	unsigned long startTime;
	unsigned long endTime;
	WCHAR lyric[17];
} SMNote;

//******************************************************************************
// Note list class
//******************************************************************************
class SMIDILIB_API SMNoteList
{
public:

	//Constructor / Destructor
	SMNoteList(void);
	virtual ~SMNoteList(void);

	//Clear
	void Clear();

	//Add note info
	int AddNote(SMNote note);

	//Get note info
	int GetNote(unsigned long index, SMNote* pNote);

	//Set note info (overwrite)
	int SetNote(unsigned long index, SMNote* pNote);

	//Get note count
	unsigned long GetSize();

	//Copy
	int CopyFrom(SMNoteList* pSrcList);

private:

	SMSimpleList m_List;

	//Prohibit assignment and copy constructor
	void operator=(const SMNoteList&);
	SMNoteList(const SMNoteList&);

};

} // end of namespace

