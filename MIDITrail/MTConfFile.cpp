//******************************************************************************
//
// MIDITrail / MTConfFile
//
// Configuration file accessor.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTParam.h"
#include "MTConfFile.h"


//******************************************************************************
// Constructor
//******************************************************************************
MTConfFile::MTConfFile(void)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
MTConfFile::~MTConfFile(void)
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTConfFile::Initialize(
		const TCHAR* pCategory
	)
{
	int result = 0;
	TCHAR confFilePath[_MAX_PATH] = {_T('\0')};
	YNConfFile confFile;

	//Get the process executable directory path
	result = YNPathUtil::GetModuleDirPath(confFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//Build the config file path
	_tcscat_s(confFilePath, _MAX_PATH, MT_CONFFILE_DIR);
	_tcscat_s(confFilePath, _MAX_PATH, pCategory);
	_tcscat_s(confFilePath, _MAX_PATH, _T(".ini"));

	//Initialize
	result = YNConfFile::Initialize(confFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


