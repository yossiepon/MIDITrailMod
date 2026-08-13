//******************************************************************************
//
// MIDITrail / MTConfFile
//
// Configuration file accessor.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"

using namespace YNBaseLib;


//******************************************************************************
// Configuration file class
//******************************************************************************
class MTConfFile : public YNConfFile
{
public:

	//Constructor / Destructor
	MTConfFile(void);
	virtual ~MTConfFile(void);

	//Initialize
	int Initialize(const TCHAR* pCategory);

};


