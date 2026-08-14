//******************************************************************************
//
// Simple MIDI Library / SMNoteList
//
// Note list class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMNoteList.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMNoteList::SMNoteList(void)
 : m_List(sizeof(SMNote), 1000)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
SMNoteList::~SMNoteList(void)
{
	Clear();
}

//******************************************************************************
// Clear
//******************************************************************************
void SMNoteList::Clear(void)
{
	m_List.Clear();
}

//******************************************************************************
// Add note info
//******************************************************************************
int SMNoteList::AddNote(
		SMNote note
	)
{
	return m_List.AddItem(&note);
}

//******************************************************************************
// Get note info
//******************************************************************************
int SMNoteList::GetNote(
		unsigned long index,
		SMNote* pNote
	)
{
	return m_List.GetItem(index, pNote);
}

//******************************************************************************
// Set note info (overwrite)
//******************************************************************************
int SMNoteList::SetNote(
		unsigned long index,
		SMNote* pNote
	)
{
	return m_List.SetItem(index, pNote);
}

//******************************************************************************
// Get note count
//******************************************************************************
unsigned long SMNoteList::GetSize()
{
	return m_List.GetSize();
}

//******************************************************************************
// Copy list
//******************************************************************************
int SMNoteList::CopyFrom(
		SMNoteList* pSrcList
	)
{
	return m_List.CopyFrom(&(pSrcList->m_List));
}

} // end of namespace

