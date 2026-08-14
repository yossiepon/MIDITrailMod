//******************************************************************************
//
// Simple MIDI Library / SMBarList
//
// Bar (measure) list class.
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
// Bar list class
//******************************************************************************
class SMIDILIB_API SMBarList
{
public:

	//Constructor / Destructor
	SMBarList(void);
	virtual ~SMBarList(void);

	//Clear
	void Clear();

	//Add bar
	int AddBar(unsigned long tickTime);

	//Get bar
	int GetBar(unsigned long index, unsigned long* pTickTime);

	//Get bar count
	unsigned long GetSize();

	//Copy
	int CopyFrom(SMBarList* pSrcList);

private:

	SMSimpleList m_List;

	//Prohibit assignment and copy constructor
	void operator=(const SMBarList&);
	SMBarList(const SMBarList&);

};

} // end of namespace

