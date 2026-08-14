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

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

namespace SMIDILib {

//******************************************************************************
// Parameter definitions
//******************************************************************************
#define SMEVENT_INTERNAL_DATA_SIZE  (16)


//******************************************************************************
// Event class
//******************************************************************************
class SMIDILIB_API SMEvent
{
public:

	//Event type
	enum EventType {
		EventNone,
		EventMIDI,
		EventSysEx,
		EventSysMsg,
		EventMeta
	};

	//Constructor / Destructor
	SMEvent(void);
	virtual ~SMEvent(void);

	//Register data
	int SetData(EventType type, unsigned char status, unsigned char meta, unsigned char* pData, unsigned long size);

	//Register MIDI event
	int SetMIDIData(unsigned char status, unsigned char* pData, unsigned long size);

	//Register SysEx event
	int SetSysExData(unsigned char status, unsigned char* pData, unsigned long size);

	//Register SysMsg event
	int SetSysMsgData(unsigned char status, unsigned char* pData, unsigned long size);

	//Register meta event
	int SetMetaData(unsigned char status, unsigned char type, unsigned char* pData, unsigned long size);

	//Get event type
	EventType GetType();

	//Get status
	unsigned char GetStatus();


	//Set status
	void SetStatus(unsigned char status);


	//Get meta type
	unsigned char GetMetaType();

	//Get data size
	unsigned long GetDataSize();

	//Get data pointer
	unsigned char* GetDataPtr();

	//Clear
	void Clear();


	//Dump output
	void Dump();


private:

	EventType m_Type;
	unsigned char m_Status;
	unsigned char m_MetaType;
	unsigned long m_DataSize;
	unsigned char m_Data[SMEVENT_INTERNAL_DATA_SIZE];
	unsigned char* m_pExData;

	//Prohibit assignment and copy constructor
	void operator=(const SMEvent&);
	SMEvent(const SMEvent&);

};

} // end of namespace

