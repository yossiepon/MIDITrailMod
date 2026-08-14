//******************************************************************************
//
// YN Base Library / YNConfFile
//
// Configuration (INI) file access class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNErrCtrl.h"
#include "YNConfFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <new>

namespace YNBaseLib {

//******************************************************************************
// Parameter definitions
//******************************************************************************
#define YNCONFFILE_NO_DATA  _T("*** NO DATA ***")

//******************************************************************************
// Constructor
//******************************************************************************
YNConfFile::YNConfFile(void)
{
	m_FilePath[0] = _T('\0');
	m_Section[0] = _T('\0');
}

//******************************************************************************
// Destructor
//******************************************************************************
YNConfFile::~YNConfFile(void)
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int YNConfFile::Initialize(
		const TCHAR* pConfFilePath
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = _tcscpy_s(m_FilePath, _MAX_PATH, pConfFilePath);
	if (eresult != 0) {
		result = -1;
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Set section
//******************************************************************************
int YNConfFile::SetCurSection(
		const TCHAR* pSection
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = _tcscpy_s(m_Section, _MAX_PATH, pSection);
	if (eresult != 0) {
		result = -1;
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Get integer value
//******************************************************************************
int YNConfFile::GetInt(
		const TCHAR* pKey,
		int* pVal,
		int defaultVal
	)
{
	int result = 0;
	DWORD apiresult = 0;
	TCHAR buf[20];

	apiresult = GetPrivateProfileString(
					m_Section,			//Section name
					pKey,				//Key name
					YNCONFFILE_NO_DATA, //Default string
					buf,				//Buffer address
					20,					//Buffer size (in TCHAR units)
					m_FilePath			//File path
				);
	//Give up on checking the return value

	if (_tcscmp(buf, YNCONFFILE_NO_DATA) == 0) {
		*pVal = defaultVal;
	}
	else {
		*pVal = _tstoi(buf);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// Set integer value
//******************************************************************************
int YNConfFile::SetInt(
		const TCHAR* pKey,
		int val
	)
{
	int result = 0;
	BOOL bresult = TRUE;
	TCHAR buf[20];

	_stprintf_s(buf, 20, _T("%d"), val);

	bresult = WritePrivateProfileString(
					m_Section,		//Section name
					pKey,			//Key name
					buf,			//String to write
					m_FilePath		//File path
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Get float value
//******************************************************************************
int YNConfFile::GetFloat(
		const TCHAR* pKey,
		float* pVal,
		float defaultVal
	)
{
	int result = 0;
	DWORD apiresult = 0;
	TCHAR buf[20];

	apiresult = GetPrivateProfileString(
					m_Section,			//Section name
					pKey,				//Key name
					YNCONFFILE_NO_DATA, //Default string
					buf,				//Buffer address
					20,					//Buffer size (in TCHAR units)
					m_FilePath			//File path
				);
	//Give up on checking the return value

	if (_tcscmp(buf, YNCONFFILE_NO_DATA) == 0) {
		*pVal = defaultVal;
	}
	else {
		//_tstof returns a double
		*pVal = (float)_tstof(buf);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// Set float value
//******************************************************************************
int YNConfFile::SetFloat(
		const TCHAR* pKey,
		float val
	)
{
	int result = 0;
	BOOL bresult = TRUE;
	TCHAR buf[20];

	_stprintf_s(buf, 20, _T("%f"), val);

	bresult = WritePrivateProfileString(
					m_Section,		//Section name
					pKey,			//Key name
					buf,			//String to write
					m_FilePath		//File path
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Get string
//******************************************************************************
int YNConfFile::GetStr(
		const TCHAR* pKey,
		TCHAR* pBuf,
		unsigned long bufSize, 
		const TCHAR* pDefaultVal
	)
{
	int result = 0;
	DWORD apiresult = 0;

	apiresult = GetPrivateProfileString(
					m_Section,			//Section name
					pKey,				//Key name
					pDefaultVal,		//Default string
					pBuf,				//Buffer address
					bufSize,			//Buffer size (in TCHAR units)
					m_FilePath			//File path
				);
	//Give up on checking the return value

//EXIT:;
	return result;
}

//******************************************************************************
// Set string
//******************************************************************************
int YNConfFile::SetStr(
		const TCHAR* pKey,
		const TCHAR* pStr
	)
{
	int result = 0;
	BOOL bresult = TRUE;
	TCHAR* pValue = NULL;
	size_t length = 0;

	//If a value ending in whitespace is written to the INI file,
	//the trailing whitespace is stripped when reading it back,
	//so wrap the value in single quotes when writing it.
	length = _tcslen(pStr) + 4;
	try {
		pValue = new TCHAR[length];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", length, 0);
		goto EXIT;
	}
	_stprintf_s(pValue, length, _T("'%s'"), pStr);

	bresult = WritePrivateProfileString(
					m_Section,		//Section name
					pKey,			//Key name
					pValue,			//String to write
					m_FilePath		//File path
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}

EXIT:;
	delete [] pValue;
	return result;
}

//******************************************************************************
// Get string (value only, as wide string)
//******************************************************************************
int YNConfFile::GetWStr(
		const TCHAR* pKey,
		WCHAR* pBuf,
		unsigned long bufSize,
		const WCHAR* pDefaultVal
	)
{
	int result = 0;
	unsigned long hexBufSize = 0;
	unsigned long hexLength = 0;
	unsigned long index = 0;
	unsigned long indexw = 0;
	TCHAR* pHexString = NULL;
	TCHAR hexChar[5];
	TCHAR* stopped = NULL;
	WCHAR wchar = 0;

	//Error if the buffer is too small to hold the default value
	if (bufSize < (wcslen(pDefaultVal) + 1)) {
		result = YN_SET_ERR("Program Error.", bufSize, 0);
		goto EXIT;
	}

	hexBufSize = (bufSize * 4) + (unsigned long)_tcslen(YNCONFFILE_NO_DATA) + 1;

	//Allocate memory to hold the hex string
	try {
		pHexString = new TCHAR[hexBufSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", hexBufSize, 0);
		goto EXIT;
	}
	memset(pHexString, 0, hexBufSize);

	//Get the hex string
	result = GetStr(pKey, pHexString, hexBufSize, YNCONFFILE_NO_DATA);
	if (result != 0) goto EXIT;
	
	//Return the default string if not set
	if (_tcscmp(pHexString, YNCONFFILE_NO_DATA) == 0) {
		wcscpy_s(pBuf, bufSize, pDefaultVal);
		goto EXIT;
	}

	hexLength = (unsigned long)_tcslen(pHexString);

	//If the string is empty
	if (hexLength == 0) {
		pBuf[0] = L'\0';
		goto EXIT;
	}

	//Convert the hex string to wide chars, 4 hex digits at a time
	//Truncate any trailing chars that don't form a complete group of 4
	while ((index + 4) <= hexLength) {
		hexChar[0] = pHexString[index + 0];
		hexChar[1] = pHexString[index + 1];
		hexChar[2] = pHexString[index + 2];
		hexChar[3] = pHexString[index + 3];
		hexChar[4] = '\0';
		pBuf[indexw] = (WCHAR)_tcstol(hexChar, &stopped, 16);

		//Stop converting if at the end of the buffer
		if ((indexw + 1) == bufSize) {
			break;
		}

		index += 4;
		indexw += 1;
	}
	pBuf[indexw] = L'\0';

EXIT:;
	delete [] pHexString;
	return result;
}

//******************************************************************************
// Set string (value only, as wide string)
//******************************************************************************
int YNConfFile::SetWStr(const TCHAR* pKey, const WCHAR* pStr)
{
	int result = 0;
	unsigned long length = 0;
	unsigned long bufSize = 0;
	unsigned long index = 0;
	TCHAR* pHexString = NULL;
	TCHAR hexChar[5];

	length = (unsigned long)wcslen(pStr);
	bufSize = (length + 1) * 4;

	//Allocate memory to hold the hex string
	try {
		pHexString = new TCHAR[bufSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", bufSize, 0);
		goto EXIT;
	}
	memset(pHexString, 0, bufSize);

	//Convert the wide string to hex, 4 digits per character (excluding the null terminator)
	for (index = 0; index < length; index ++) {
		_stprintf_s(hexChar, 5, _T("%04X"), pStr[index]);
		_tcscat_s(pHexString, bufSize, hexChar);
	}

	//Write the string
	result = SetStr(pKey, pHexString);
	if (result != 0) goto EXIT;

EXIT:;
	delete [] pHexString;
	return result;
}

} // end of namespace


