//******************************************************************************
//
// YN Base Library / YNConfFile
//
// Configuration (INI) file access class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Class that wraps access to an INI file.

#pragma once

#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

#include <stdlib.h>

namespace YNBaseLib {

//******************************************************************************
// Configuration file class
//******************************************************************************
class YNBASELIB_API YNConfFile
{
public:

	//Constructor / Destructor
	YNConfFile(void);
	virtual ~YNConfFile(void);

	//Initialize
	int Initialize(const TCHAR* pConfFilePath);

	//Set the current section
	int SetCurSection(const TCHAR* pSection);

	//Get/set integer value
	int GetInt(const TCHAR* pKey, int* pVal, int defaultVal);
	int SetInt(const TCHAR* pKey, int val);

	//Get/set float value
	int GetFloat(const TCHAR* pKey, float* pVal, float defaultVal);
	int SetFloat(const TCHAR* pKey, float val);

	//Get/set string
	int GetStr(const TCHAR* pKey, TCHAR* pBuf, unsigned long bufSize, const TCHAR* pDefaultVal);
	int SetStr(const TCHAR* pKey, const TCHAR* pStr);

	//Get/set string (value only, as wide string)
	int GetWStr(const TCHAR* pKey, WCHAR* pBuf, unsigned long bufSize, const WCHAR* pDefaultVal);
	int SetWStr(const TCHAR* pKey, const WCHAR* pStr);

private:

	TCHAR m_FilePath[_MAX_PATH];
	TCHAR m_Section[_MAX_PATH];

	//Prohibit assignment and copy constructor
	void operator=(const YNConfFile&);
	YNConfFile(const YNConfFile&);

};


} // end of namespace

