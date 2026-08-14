//******************************************************************************
//
// Simple MIDI Library / SMPortList
//
// Port list class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMPortList.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMPortList::SMPortList(void)
 : m_List(sizeof(unsigned char), 10)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
SMPortList::~SMPortList(void)
{
	Clear();
}

//******************************************************************************
// Clear
//******************************************************************************
void SMPortList::Clear(void)
{
	m_List.Clear();
}

//******************************************************************************
// Add port info
//******************************************************************************
int SMPortList::AddPort(
		unsigned char portNo
	)
{
	return m_List.AddItem(&portNo);
}

//******************************************************************************
// Get port info
//******************************************************************************
int SMPortList::GetPort(
		unsigned long index,
		unsigned char* pPortNo
	)
{
	return m_List.GetItem(index, pPortNo);
}

//******************************************************************************
// Get port count
//******************************************************************************
unsigned long SMPortList::GetSize()
{
	return m_List.GetSize();
}

//******************************************************************************
// Copy list
//******************************************************************************
int SMPortList::CopyFrom(
		SMPortList* pSrcList
	)
{
	return m_List.CopyFrom(&(pSrcList->m_List));
}

} // end of namespace

