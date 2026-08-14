//******************************************************************************
//
// Simple MIDI Library / SMEventSysEx
//
// System exclusive (SysEx) event class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventSysEx.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMEventSysEx::SMEventSysEx()
{
	m_pEvent = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMEventSysEx::~SMEventSysEx(void)
{
}

//******************************************************************************
// Attach event
//******************************************************************************
void SMEventSysEx::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// Get MIDI output message (long)
//******************************************************************************
int SMEventSysEx::GetMIDIOutLongMsg(
		unsigned char** pPtrMsg,
		unsigned long* pSize
	)
{
	int result = 0;

	if (m_pEvent == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	*pSize = m_pEvent->GetDataSize();
	*pPtrMsg = m_pEvent->GetDataPtr();

EXIT:;
	return result;
}

} // end of namespace


