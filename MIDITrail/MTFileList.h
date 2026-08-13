//******************************************************************************
//
// MIDITrail / MTFileList
//
// File list manager.
//
// Copyright (C) 2021-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "SMRcpConv.h"
#include <list>
#include <string>

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// File list class
//******************************************************************************
class MTFileList
{
public:

	//Constructor / Destructor
	MTFileList(void);
	virtual ~MTFileList(void);

	//Create the file list under the directory
	int MakeFileListWithDirectory(const WCHAR* pTargetDirPath, SMRcpConv* pRcpConv);

	//File count
	size_t GetFileCount();

	//Get file path
	const WCHAR* GetFilePath(unsigned long index);

	//Get file name
	const WCHAR* GetFileName(unsigned long index);

	//Clear
	void Clear();

	//Register selected file
	int SetSelectedFileName(const WCHAR* pFileName);

	//Select first file
	void SelectFirstFile();

	//Select previous file
	void SelectPreviousFile(bool* pExist);

	//Select next file
	void SelectNextFile(bool* pExist);

	//Get selected file index
	unsigned long GetSelectedFileIndex();

private:

	//Prohibit assignment and copy constructor
	void operator=(const MTFileList&);
	MTFileList(const MTFileList&);

	WCHAR m_TargetDirPath[_MAX_PATH];
	WCHAR m_CurFilePath[_MAX_PATH];

	typedef std::list<wstring> MTFileNameList;
	MTFileNameList m_FileNameList;

	unsigned long m_SelectedFileIndex;


};

