//******************************************************************************
//
// Simple MIDI Library / SMPortList
//
// Port list class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
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
// Port list class
//******************************************************************************
class SMIDILIB_API SMPortList
{
public:

	//Constructor / Destructor
	SMPortList(void);
	virtual ~SMPortList(void);

	//Clear
	void Clear();

	//Register port
	int AddPort(unsigned char portNo);

	//Get port
	int GetPort(unsigned long index, unsigned char* pPortNo);

	//Get port count
	unsigned long GetSize();

	//Copy
	int CopyFrom(SMPortList* pSrcList);

private:

	SMSimpleList m_List;

	//Prohibit assignment and copy constructor
	void operator=(const SMPortList&);
	SMPortList(const SMPortList&);

};

} // end of namespace

