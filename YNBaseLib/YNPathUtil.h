//******************************************************************************
//
// YN Base Library / YNPathUtil
//
// Path utility class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

namespace YNBaseLib {

//******************************************************************************
// Path utility class
//******************************************************************************
class YNBASELIB_API YNPathUtil
{
public:

	//Get the process executable's directory path
	//  A trailing "\" is appended
	//  Example path: "C:\Program Files\AppName\"
	static int GetModuleDirPath(TCHAR* pBuf, unsigned long bufSize);

	//Get the application data directory path
	//  A trailing "\" is appended
	//  Example path on Windows 7: "C:\Users\UserName\AppData\Roaming\"
	static int GetAppDataDirPath(TCHAR* pBuf, unsigned long bufSize);

	//Check file extension
	//  Determines whether the file's extension matches the one specified
	//  Example extension: ".txt"
	static bool IsFileExtMatch(const WCHAR* pPath, const WCHAR* pExt);

	//Get temporary file path
	//  Creates a unique temporary file in the temp directory defined by
	//  the TMP or TEMP environment variable and returns its path
	//  The prefix can be up to 3 characters
	//  The created file is named PREuuuu.TMP
	static int GetTempFilePath(WCHAR* pPathBuf, unsigned long bufSize, const WCHAR* pPrefix);

private:

	//Constructor / Destructor
	YNPathUtil(void);
	virtual ~YNPathUtil(void);

	//Prohibit assignment and copy constructor
	void operator=(const YNPathUtil&);
	YNPathUtil(const YNPathUtil&);

};

} // end of namespace

