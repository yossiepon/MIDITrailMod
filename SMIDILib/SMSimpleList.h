//******************************************************************************
//
// Simple MIDI Library / SMSimpleList
//
// Fixed-size block-allocated simple list class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// A simple list class that only adds/references fixed-size items.
// Allocates memory in block units to reduce the number of new calls,
// prioritizing performance at the cost of wasted memory.

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <map>

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// Simple list class
//******************************************************************************
class SMIDILIB_API SMSimpleList
{
public:

	//Constructor / Destructor
	SMSimpleList(unsigned long itemSize, unsigned long unitNum);
	virtual ~SMSimpleList(void);

	//Clear
	virtual void Clear();

	//Add item
	virtual int AddItem(void* pItem);

	//Get item
	virtual int GetItem(unsigned long index, void* pItem);

	//Set item (overwrite)
	virtual int SetItem(unsigned long index, void* pItem);

	//Get item count
	virtual unsigned long GetSize();

	//Copy
	virtual int CopyFrom(SMSimpleList* pSrcList);

private:

	typedef std::map<unsigned long, unsigned char*> SMMemBlockMap;
	typedef std::pair<unsigned long, unsigned char*> SMMemBlockMapPair;

private:

	unsigned long m_ItemSize;
	unsigned long m_UnitNum;
	unsigned long m_DataNum;

	SMMemBlockMap m_MemBlockMap;

	unsigned long _GetBlockNo(unsigned long index);
	unsigned long _GetBlockIndex(unsigned long index);

	//Prohibit assignment and copy constructor
	void operator=(const SMSimpleList&);
	SMSimpleList(const SMSimpleList&);

};

} // end of namespace

#pragma warning(default:4251)


