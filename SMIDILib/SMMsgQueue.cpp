//******************************************************************************
//
// Simple MIDI Library / SMMsgQueue
//
// Message queue class.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "SMMsgQueue.h"

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMMsgQueue::SMMsgQueue(void)
 : m_List(sizeof(unsigned long)*2, 10000)
{
	InitializeCriticalSection(&m_CriticalSection);
	m_MaxMsgNum = 0;
	m_NextPostIndex = 0;
	m_NextReadIndex = 0;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMMsgQueue::~SMMsgQueue(void)
{
	DeleteCriticalSection(&m_CriticalSection);
}

//******************************************************************************
// Create buffer
//******************************************************************************
int SMMsgQueue::Initialize(
		unsigned long maxMsgNum
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long dummy[2] = {0, 0};
	
	//Do nothing if already created
	if (m_List.GetSize() > 0) goto EXIT;
	
	for (index = 0; index < maxMsgNum; index++) {
		result = m_List.AddItem(dummy);
		if (result != 0) goto EXIT;
	}
	m_MaxMsgNum = maxMsgNum;
	
EXIT:;
	return result;
}

//******************************************************************************
// Post message
//******************************************************************************
int SMMsgQueue::PostMessage(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	unsigned long params[2] = {0, 0};
	
	EnterCriticalSection(&m_CriticalSection);
	
	params[0] = param1;
	params[1] = param2;
	
	//Register parameters
	result = m_List.SetItem(m_NextPostIndex, params);
	if (result != 0) goto EXIT;
	
	//Update next post index
	m_NextPostIndex++;
	if (m_NextPostIndex == m_MaxMsgNum) {
		m_NextPostIndex = 0;
	}
	
	//When the oldest unread data was discarded by overwrite
	if (m_NextPostIndex == m_NextReadIndex) {
		//Advance the read index (ignore the discarded data)
		m_NextReadIndex++;
		if (m_NextReadIndex == m_MaxMsgNum) {
			m_NextReadIndex = 0;
		}
	}
	
EXIT:;
	LeaveCriticalSection(&m_CriticalSection);
	return result;
}

//******************************************************************************
// Get message
//******************************************************************************
int SMMsgQueue::GetMessage(
		bool* pIsExist,
		unsigned long* pParam1,
		unsigned long* pParam2
	)
{
	int result = 0;
	unsigned long params[2] = {0, 0};
	
	EnterCriticalSection(&m_CriticalSection);
	
	*pIsExist = false;
	
	//When the message queue is empty
	if (m_NextReadIndex == m_NextPostIndex) goto EXIT;
	
	//Get parameters
	result = m_List.GetItem(m_NextReadIndex, params);
	if (result != 0) goto EXIT;
	
	*pParam1 = params[0];
	*pParam2 = params[1];
	
	//Update next read index
	m_NextReadIndex++;
	if (m_NextReadIndex == m_MaxMsgNum) {
		m_NextReadIndex = 0;
	}
	
	*pIsExist = true;
	
EXIT:;
	LeaveCriticalSection(&m_CriticalSection);
	return result;
}

} // end of namespace

