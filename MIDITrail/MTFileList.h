//******************************************************************************
//
// MIDITrail / MTFileList
//
// ファイルリストクラス
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
// ファイルリストクラス
//******************************************************************************
class MTFileList
{
public:

	//コンストラクタ／デストラクタ
	MTFileList(void);
	virtual ~MTFileList(void);

	//ディレクトリ配下ファイルリスト作成
	int MakeFileListWithDirectory(const WCHAR* pTargetDirPath, SMRcpConv* pRcpConv);

	//ファイル数
	size_t GetFileCount();

	//ファイルパス取得
	const WCHAR* GetFilePath(unsigned long index);

	//ファイル名取得
	const WCHAR* GetFileName(unsigned long index);

	//クリア
	void Clear();

	//選択ファイル登録
	int SetSelectedFileName(const WCHAR* pFileName);

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

	WCHAR m_TargetDirPath[_MAX_PATH];
	WCHAR m_CurFilePath[_MAX_PATH];

	typedef std::list<wstring> MTFileNameList;
	MTFileNameList m_FileNameList;

	unsigned long m_SelectedFileIndex;


};

