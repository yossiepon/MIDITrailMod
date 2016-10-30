//******************************************************************************
//
// MIDITrail / MTConfFile
//
// 設定ファイルクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTParam.h"
#include "MTConfFile.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTConfFile::MTConfFile(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTConfFile::~MTConfFile(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTConfFile::Initialize(
		const TCHAR* pCategory
	)
{
	int result = 0;
	TCHAR confFilePath[_MAX_PATH] = {_T('\0')};
	YNConfFile confFile;

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPath(confFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//設定ファイルパス登録
	_tcscat_s(confFilePath, _MAX_PATH, MT_CONFFILE_DIR);
	_tcscat_s(confFilePath, _MAX_PATH, pCategory);
	_tcscat_s(confFilePath, _MAX_PATH, _T(".ini"));

	//初期化
	result = YNConfFile::Initialize(confFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


