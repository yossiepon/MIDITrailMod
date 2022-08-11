//******************************************************************************
//
// MIDITrail / MTFileList
//
// ファイルリストクラス
//
// Copyright (C) 2021 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "SMRcpConv.h"
#include <list>
#include <string>

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// ファイルリストクラス
//******************************************************************************
class MTFileList
{
public:

	//コンストラクタ／デストラクタ
	MTFileList(void);
	virtual ~MTFileList(void);

	//ディレクトリ配下ファイルリスト作成
	int MakeFileListWithDirectory(const TCHAR* pTargetDirPath, SMRcpConv* pRcpConv);

	//ファイル数
	size_t GetFileCount();

	//ファイルパス取得
	const TCHAR* GetFilePath(unsigned long index);

	//ファイル名取得
	const TCHAR* GetFileName(unsigned long index);

	//クリア
	void Clear();

	//選択ファイル登録
	int SetSelectedFileName(const TCHAR* pFileName);

	//先頭ファイル選択
	void SelectFirstFile();

	//前ファイル選択
	void SelectPreviousFile(bool* pExist);

	//次ファイル選択
	void SelectNextFile(bool* pExist);

	//選択ファイルインデックス取得
	unsigned long GetSelectedFileIndex();

private:

	//代入とコピーコンストラクタの禁止
	void operator=(const MTFileList&);
	MTFileList(const MTFileList&);

	TCHAR m_TargetDirPath[_MAX_PATH];
	TCHAR m_CurFilePath[_MAX_PATH];

#ifdef _UNICODE
	typedef std::list<wsting> MTFileNameList;
#else
	typedef std::list<string> MTFileNameList;
#endif

	MTFileNameList m_FileNameList;

	unsigned long m_SelectedFileIndex;


};

