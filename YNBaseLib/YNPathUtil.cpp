//******************************************************************************
//
// YN Base Library / YNPathUtil
//
// Path utility class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNErrCtrl.h"
#include "YNPathUtil.h"
#include <stdlib.h>
#include <shlobj.h>
#include <stdio.h>

namespace YNBaseLib {


//******************************************************************************
// Constructor
//******************************************************************************
YNPathUtil::YNPathUtil(void)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
YNPathUtil::~YNPathUtil(void)
{
}

//******************************************************************************
// Get the process executable's directory path
//******************************************************************************
int YNPathUtil::GetModuleDirPath(
		TCHAR* pBuf,
		unsigned long bufSize
	)
{
	int result = 0;
	DWORD apiresult = 0;
	errno_t eresult = 0;
	TCHAR path[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	//Get the process executable's file path
	apiresult = GetModuleFileName(GetModuleHandle(NULL), path, _MAX_PATH);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Split the path into its elements
	eresult = _tsplitpath_s(
					path,		//Path
					drive,		//Drive string buffer
					_MAX_DRIVE,	//Buffer size
					dir,		//Directory string buffer
					_MAX_DIR,	//Buffer size
					fname,		//File name string buffer
					_MAX_FNAME,	//Buffer size
					ext,		//Extension string buffer
					_MAX_EXT	//Buffer size
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Build the path
	eresult = _tmakepath_s(
					pBuf,		//Destination buffer for the path
					bufSize,	//Buffer size
					drive,		//Drive string
					dir,		//Directory string
					NULL,		//File name string
					NULL		//Extension string
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get the application data directory path
//******************************************************************************
int YNPathUtil::GetAppDataDirPath(
		TCHAR* pBuf,
		unsigned long bufSize
	)
{
	int result = 0;
	HRESULT hresult = 0;
	errno_t eresult = 0;
	TCHAR path[MAX_PATH];

	hresult = SHGetFolderPath(
					NULL,				//Owner window
					CSIDL_APPDATA,		//Folder specifier
					NULL,				//Access token
					SHGFP_TYPE_CURRENT,	//Flag: current folder path
										//  the user may have relocated it
					path				//Destination buffer for the path
				);
	if (hresult != S_OK) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	eresult = _tcscpy_s(pBuf, bufSize, path);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	eresult = _tcscat_s(pBuf, bufSize, _T("\\"));
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Check file extension
//******************************************************************************
bool YNPathUtil::IsFileExtMatch(
		const WCHAR* pPath,
		const WCHAR* pExt
	)
{
	bool isMatch = false;
	errno_t eresult = 0;
	WCHAR ext[_MAX_EXT] = { L'\0' };

	//Split the path into its elements and get the extension
	eresult = _wsplitpath_s(
					pPath,			//Path
					NULL, 0,		//Drive string buffer and size
					NULL, 0,		//Directory string buffer and size
					NULL, 0,		//File name string buffer and size
					ext, _MAX_EXT	//Extension string buffer and size
				);
	if (eresult != 0) {
		//result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Compare the extension case-insensitively
	if (_wcsicmp(ext, pExt) == 0) {
		isMatch = true;
	}

EXIT:;
	return isMatch;
}

//******************************************************************************
// Get temporary file path
//******************************************************************************
int YNPathUtil::GetTempFilePath(
		WCHAR* pPathBuf,
		unsigned long bufSize,
		const WCHAR* pPrefix
	)
{
	int result = 0;
	DWORD apiresult = 0;
	WCHAR tempDir[_MAX_PATH] = { L'\0' };

	//Get the temporary directory path
	apiresult = GetTempPathW(_MAX_PATH, tempDir);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//GetTempFileName is an odd API that doesn't let you specify a buffer size
	//It's documented as requiring a buffer of at least MAX_PATH,
	//so check the size here
	if (bufSize < MAX_PATH) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Get the temporary file path
	//  File name: PREuuuu.TMP
	//    PRE : prefix
	//    uuuu: hex string generated from the system time
	apiresult = GetTempFileNameW(
						tempDir,	//Directory path
						pPrefix,	//Prefix (3 characters)
						0,			//Uniqueness: enabled
						pPathBuf	//The generated file path
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

} // end of namespace

