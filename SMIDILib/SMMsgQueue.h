//******************************************************************************
//
// Simple MIDI Library / SMMsgQueue
//
// Message queue class.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
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
// Message queue class
//******************************************************************************
class SMIDILIB_API SMMsgQueue
{
public:
	
	//Constructor / Destructor
	SMMsgQueue(void);
	virtual ~SMMsgQueue(void);
	
	//Initialize
	int Initialize(unsigned long maxMsgNum);
	
	//Post message
	int PostMessage(unsigned long param1, unsigned long param2);
	
	//Get message
	int GetMessage(bool* pIsExist, unsigned long* pParam1, unsigned long* pParam2);
	
private:
	
	CRITICAL_SECTION m_CriticalSection;
	
	SMSimpleList m_List;
	unsigned long m_MaxMsgNum;
	unsigned long m_NextPostIndex;
	unsigned long m_NextReadIndex;

};

} // end of namespace

