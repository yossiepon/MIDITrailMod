//******************************************************************************
//
// Simple MIDI Library / SMEvent
//
// Base MIDI event class.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2025 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEvent.h"
#include <new>

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMEvent::SMEvent(void)
{
	m_pExData = NULL;
	Clear();
}

//******************************************************************************
// Destructor
//******************************************************************************
SMEvent::~SMEvent(void)
{
	delete [] m_pExData;
}

//******************************************************************************
// Register data
//******************************************************************************
int SMEvent::SetData(
		EventType type,
		unsigned char status,
		unsigned char meta,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	m_Type = type;
	m_Status = status;
	m_MetaType = meta;

	delete [] m_pExData;
	m_pExData = NULL;
	m_DataSize = 0;
	ZeroMemory(m_Data, SMEVENT_INTERNAL_DATA_SIZE);

	if (size == 0) {
		//do nothing
	}
	else if (size <= SMEVENT_INTERNAL_DATA_SIZE) {
		memcpy(m_Data, pData, size);
	}
	else {
		try {
			m_pExData = new unsigned char[size];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", size, 0);
			goto EXIT;
		}
		memcpy(m_pExData, pData, size);
	}

	m_DataSize = size;

EXIT:;
	return result;
}

//******************************************************************************
// Register MIDI event data
//******************************************************************************
int SMEvent::SetMIDIData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	result = SetData(EventMIDI, status, 0, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Register SysEx event data
//******************************************************************************
int SMEvent::SetSysExData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	unsigned char* pExData = NULL;

	//If status is 0xF0, it's the first packet
	// -> Send with 0xF0 prepended
	if (status == 0xF0) {
		try {
			pExData = new unsigned char[size + 1];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", size + 1, 0);
			goto EXIT;
		}
		pExData[0] = status;
		memcpy(&(pExData[1]), pData, size);
		result = SetData(EventSysEx, status, 0, pExData, size + 1);
		if (result != 0) goto EXIT;
	}
	//If status is 0xF7, it's a continuation packet
	// -> Send without prepending 0xF7
	else if (status == 0xF7) {
		result = SetData(EventSysEx, status, 0, pData, size);
		if (result != 0) goto EXIT;
	}
	//Otherwise, it's an error
	else {
		result = YN_SET_ERR("Program error.", status, 0);
		goto EXIT;
	}

EXIT:;
	delete [] pExData;
	return result;
}

//******************************************************************************
// Register SysMsg event data
//******************************************************************************
int SMEvent::SetSysMsgData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	
	result = SetData(EventSysMsg, status, 0, pData, size);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// Register meta event data
//******************************************************************************
int SMEvent::SetMetaData(
		unsigned char status,
		unsigned char type,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	result = SetData(EventMeta, status, type, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get event type
//******************************************************************************
SMEvent::EventType SMEvent::GetType()
{
	return m_Type;
}

//******************************************************************************
// Get status
//******************************************************************************
unsigned char SMEvent::GetStatus()
{
	return m_Status;
}


//******************************************************************************
// Set status
//******************************************************************************
void SMEvent::SetStatus(unsigned char status)
{
	m_Status = status;
}


//******************************************************************************
// Get meta event type
//******************************************************************************
unsigned char SMEvent::GetMetaType()
{
	return m_MetaType;
}

//******************************************************************************
// Get data size
//******************************************************************************
unsigned long SMEvent::GetDataSize()
{
	return m_DataSize;
}

//******************************************************************************
// Get data pointer
//******************************************************************************
unsigned char* SMEvent::GetDataPtr()
{
	unsigned char* pData = NULL;

	if (m_DataSize <= SMEVENT_INTERNAL_DATA_SIZE) {
		pData = m_Data;
	}
	else {
		pData = m_pExData;
	}

	return pData;
}

//******************************************************************************
// Clear
//******************************************************************************
void SMEvent::Clear()
{
	delete [] m_pExData;
	
	m_Type = EventNone;
	m_Status = 0;
	m_MetaType = 0;
	m_DataSize = 0;
	memset(m_Data, 0, SMEVENT_INTERNAL_DATA_SIZE);
	m_pExData = NULL;
}

//******************************************************************************
// Dump output
//******************************************************************************

void SMEvent::Dump()
{
#ifdef _DEBUG
	char buf[256];
	sprintf_s(buf, "Event %02X: Status %02X, Meta: %02X, DataSize: %d(%04X), Data: [", m_Type, m_Status, m_MetaType, m_DataSize, m_DataSize);
	if (m_DataSize <= SMEVENT_INTERNAL_DATA_SIZE)
	{
		for (int i = 0; i < m_DataSize; i++)
		{
			if (i > 0)
			{
				strcat_s(buf, "-");
			}
			char buf2[8];
			sprintf_s(buf2, "%02X", m_Data[i]);
			strcat_s(buf, buf2);
		}
	}
	else
	{
		strcat_s(buf, "...");
	}
	strcat_s(buf, "]\n");
	OutputDebugStringA(buf);
#endif
}

} // end of namespace
