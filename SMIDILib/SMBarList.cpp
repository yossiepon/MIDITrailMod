//******************************************************************************
//
// Simple MIDI Library / SMBarList
//
// Bar (measure) list class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMBarList.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMBarList::SMBarList(void)
 : m_List(sizeof(unsigned long), 100)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
SMBarList::~SMBarList(void)
{
	Clear();
}

//******************************************************************************
// Clear
//******************************************************************************
void SMBarList::Clear(void)
{
	m_List.Clear();
}

//******************************************************************************
// Add bar info
//******************************************************************************
int SMBarList::AddBar(
		unsigned long tickTime
	)
{
	return m_List.AddItem(&tickTime);
}

//******************************************************************************
// Get bar info
//******************************************************************************
int SMBarList::GetBar(
		unsigned long index,
		unsigned long* pTickTime
	)
{
	return m_List.GetItem(index, pTickTime);
}

//******************************************************************************
// Get bar count
//******************************************************************************
unsigned long SMBarList::GetSize()
{
	return m_List.GetSize();
}

//******************************************************************************
// Copy list
//******************************************************************************
int SMBarList::CopyFrom(
		SMBarList* pSrcList
	)
{
	return m_List.CopyFrom(&(pSrcList->m_List));
}

} // end of namespace

