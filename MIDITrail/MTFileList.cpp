//******************************************************************************
//
// MIDITrail / MTFileList
//
// File list manager.
//
// Copyright (C) 2021-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTFileList.h"
#include "shlwapi.h"

using namespace YNBaseLib;


//******************************************************************************
// Constructor
//******************************************************************************
MTFileList::MTFileList(void)
{
	m_TargetDirPath[0] = L'\0';
	m_CurFilePath[0] = L'\0';
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTFileList::~MTFileList(void)
{
	Clear();
}

//******************************************************************************
// Clear
//******************************************************************************
void MTFileList::Clear()
{
	m_TargetDirPath[0] = L'\0';
	m_CurFilePath[0] = L'\0';
	m_FileNameList.clear();
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// Create the file list under the directory
//******************************************************************************
int MTFileList::MakeFileListWithDirectory(
		const WCHAR* pTargetDirPath,
		SMRcpConv* pRcpConv
	)
{
	int result = 0;
	WCHAR findPath[_MAX_PATH] = { L'\0' };
	WIN32_FIND_DATAW findData;
	HANDLE hFind = NULL;
	BOOL isFind = true;
	bool isMIDIDataFile = false;
	
	if (pTargetDirPath == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	if (wcslen(pTargetDirPath) > (_MAX_PATH - 1)) {
		result = YN_SET_ERR("Directory path is too long.", wcslen(pTargetDirPath), 0);
		goto EXIT;
	}
	
	Clear();
	
	//Store the directory path
	wcscpy_s(m_TargetDirPath, _MAX_PATH, pTargetDirPath);
	if (pTargetDirPath[wcslen(pTargetDirPath) - 1] != L'\\') {
		wcscat_s(m_TargetDirPath, _MAX_PATH, L"\\");
	}
	
	//Build the search path
	findPath[0] = L'\0';
	wcscat_s(findPath, _MAX_PATH, m_TargetDirPath);
	wcscat_s(findPath, _MAX_PATH, L"*.*");

	//Search for files
	hFind = FindFirstFileW(findPath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		//No file found
		goto EXIT;
	}

	//Build the file name list
	while (isFind) {
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			//Ignore directories
		}
		else {
			//Check the file extension
			isMIDIDataFile = false;
			if (YNPathUtil::IsFileExtMatch(findData.cFileName, L".mid")) {
				isMIDIDataFile = true;
			}
			else if (pRcpConv->IsAvailable() && pRcpConv->IsSupportFileExt(findData.cFileName)) {
				isMIDIDataFile = true;
			}
			if (isMIDIDataFile) {
				//Add the file name to the list
				m_FileNameList.push_back(findData.cFileName);
			}
		}
		//Search for the next file
		isFind = FindNextFileW(hFind, &findData);
	}

	//Sort the file names
	m_FileNameList.sort();

EXIT:;
	if (hFind != NULL) FindClose(hFind);
	return result;
}

//******************************************************************************
// Get file count
//******************************************************************************
size_t MTFileList::GetFileCount()
{
	return m_FileNameList.size();
}

//******************************************************************************
// Get file path
//******************************************************************************
const WCHAR* MTFileList::GetFilePath(unsigned long index)
{
	WCHAR* pFilePath = NULL;
	MTFileNameList::iterator itr;

	if (m_FileNameList.size() <= index) {
		pFilePath = NULL;
	}
	else {
		itr = m_FileNameList.begin();
		advance(itr, index);
		m_CurFilePath[0] = L'\0';
		wcscat_s(m_CurFilePath, _MAX_PATH, m_TargetDirPath);
		wcscat_s(m_CurFilePath, _MAX_PATH, (*itr).c_str());
		pFilePath = &(m_CurFilePath[0]);
	}

	return pFilePath;
}

//******************************************************************************
// Get file name
//******************************************************************************
const WCHAR* MTFileList::GetFileName(unsigned long index)
{
	const WCHAR* pFilePath = NULL;
	const WCHAR* pFileName = NULL;

	pFilePath = GetFilePath(index);
	if (pFilePath != NULL) {
		pFileName = PathFindFileNameW(pFilePath);
	}

	return pFileName;
}

//******************************************************************************
// Register selected file
//******************************************************************************
int MTFileList::SetSelectedFileName(const WCHAR* pFileName)
{
	int result = 0;
	unsigned long index = 0;
	MTFileNameList::iterator itr;

	if (pFileName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_SelectedFileIndex = 0;

	//Search the file name list (case-insensitive)
	for (itr = m_FileNameList.begin(); itr != m_FileNameList.end(); itr++) {
		if (_wcsicmp((*itr).c_str(), pFileName) == 0) {
			m_SelectedFileIndex = index;
			break;
		}
		index++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Select first file
//******************************************************************************
void MTFileList::SelectFirstFile()
{
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// Select previous file
//******************************************************************************
void MTFileList::SelectPreviousFile(bool* pIsExist)
{
	bool isExist = false;

	//When there are no files
	if (m_FileNameList.size() == 0) {
		//Exit with no previous file
	}
	//When the first file in the list is currently selected
	else if (m_SelectedFileIndex == 0) {
		//Exit with no previous file
	}
	else {
		//Select the previous file
		m_SelectedFileIndex -= 1;
		isExist = true;
	}

	if (pIsExist != NULL) {
		*pIsExist = isExist;
	}

	return;
}

//******************************************************************************
// Select next file
//******************************************************************************
void MTFileList::SelectNextFile(bool* pIsExist)
{
	bool isExist = false;

	//When there are no files
	if (m_FileNameList.size() == 0) {
		//Exit with no next file
	}
	//When the last file in the list is currently selected
	else if (m_SelectedFileIndex >= (m_FileNameList.size() - 1)) {
		//Exit with no next file
	}
	else {
		//Select the next file
		m_SelectedFileIndex += 1;
		isExist = true;
	}
	
	if (pIsExist != NULL) {
		*pIsExist = isExist;
	}

	return;
}

//******************************************************************************
// Select first file
//******************************************************************************
unsigned long MTFileList::GetSelectedFileIndex()
{
	return m_SelectedFileIndex;
}


