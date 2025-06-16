//******************************************************************************
//
// MIDITrail / MTFileList
//
// ファイルリストクラス
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
// コンストラクタ
//******************************************************************************
MTFileList::MTFileList(void)
{
	m_TargetDirPath[0] = L'\0';
	m_CurFilePath[0] = L'\0';
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTFileList::~MTFileList(void)
{
	Clear();
}

//******************************************************************************
// クリア
//******************************************************************************
void MTFileList::Clear()
{
	m_TargetDirPath[0] = L'\0';
	m_CurFilePath[0] = L'\0';
	m_FileNameList.clear();
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// ディレクトリ配下ファイルリスト作成
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
	
	//ディレクトリパスを保持する
	wcscpy_s(m_TargetDirPath, _MAX_PATH, pTargetDirPath);
	if (pTargetDirPath[wcslen(pTargetDirPath) - 1] != L'\\') {
		wcscat_s(m_TargetDirPath, _MAX_PATH, L"\\");
	}
	
	//ファイル検索用パス作成
	findPath[0] = L'\0';
	wcscat_s(findPath, _MAX_PATH, m_TargetDirPath);
	wcscat_s(findPath, _MAX_PATH, L"*.*");

	//ファイル検索
	hFind = FindFirstFileW(findPath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		//ファイルが見つからない
		goto EXIT;
	}

	//ファイル名リストを作成
	while (isFind) {
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			//ディレクトリは無視する
		}
		else {
			//ファイル拡張子を確認
			isMIDIDataFile = false;
			if (YNPathUtil::IsFileExtMatch(findData.cFileName, L".mid")) {
				isMIDIDataFile = true;
			}
			else if (pRcpConv->IsAvailable() && pRcpConv->IsSupportFileExt(findData.cFileName)) {
				isMIDIDataFile = true;
			}
			if (isMIDIDataFile) {
				//ファイル名をリストに追加
				m_FileNameList.push_back(findData.cFileName);
			}
		}
		//次のファイルを検索
		isFind = FindNextFileW(hFind, &findData);
	}

	//ファイル名ソート
	m_FileNameList.sort();

EXIT:;
	if (hFind != NULL) FindClose(hFind);
	return result;
}

//******************************************************************************
// ファイル数取得
//******************************************************************************
size_t MTFileList::GetFileCount()
{
	return m_FileNameList.size();
}

//******************************************************************************
// ファイルパス取得
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
// ファイル名取得
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
// 選択ファイル登録
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

	//ファイル名リストから検索（大文字小文字を区別しない）
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
// 先頭ファイル選択
//******************************************************************************
void MTFileList::SelectFirstFile()
{
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// 前ファイル選択
//******************************************************************************
void MTFileList::SelectPreviousFile(bool* pIsExist)
{
	bool isExist = false;

	//ファイルが存在しない場合
	if (m_FileNameList.size() == 0) {
		//前ファイルなしで終了
	}
	//ファイルリスト先頭を選択中の場合
	else if (m_SelectedFileIndex == 0) {
		//前ファイルなしで終了
	}
	else {
		//前ファイルを選択
		m_SelectedFileIndex -= 1;
		isExist = true;
	}

	if (pIsExist != NULL) {
		*pIsExist = isExist;
	}

	return;
}

//******************************************************************************
// 次ファイル選択
//******************************************************************************
void MTFileList::SelectNextFile(bool* pIsExist)
{
	bool isExist = false;

	//ファイルが存在しない場合
	if (m_FileNameList.size() == 0) {
		//次ファイルなしで終了
	}
	//ファイルリスト末尾を選択中の場合
	else if (m_SelectedFileIndex >= (m_FileNameList.size() - 1)) {
		//次ファイルなしで終了
	}
	else {
		//次ファイルを選択
		m_SelectedFileIndex += 1;
		isExist = true;
	}
	
	if (pIsExist != NULL) {
		*pIsExist = isExist;
	}

	return;
}

//******************************************************************************
// 先頭ファイル選択
//******************************************************************************
unsigned long MTFileList::GetSelectedFileIndex()
{
	return m_SelectedFileIndex;
}


