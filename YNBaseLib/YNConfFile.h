//******************************************************************************
//
// Simple Base Library / YNConfFile
//
// 設定ファイルクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// INIファイルへのアクセスをラップするクラス。

#pragma once

#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

#include <stdlib.h>

namespace YNBaseLib {

//******************************************************************************
// 設定ファイルクラス
//******************************************************************************
class YNBASELIB_API YNConfFile
{
public:

	//コンストラクタ／デストラクタ
	YNConfFile(void);
	virtual ~YNConfFile(void);

	//初期化
	int Initialize(const TCHAR* pConfFilePath);

	//カレントセクション設定
	int SetCurSection(const TCHAR* pSection);

	//整数値取得／登録
	int GetInt(const TCHAR* pKey, int* pVal, int defaultVal);
	int SetInt(const TCHAR* pKey, int val);

	//浮動小数値取得／登録
	int GetFloat(const TCHAR* pKey, float* pVal, float defaultVal);
	int SetFloat(const TCHAR* pKey, float val);

	//文字列取得／登録
	int GetStr(const TCHAR* pKey, TCHAR* pBuf, unsigned long bufSize, const TCHAR* pDefaultVal);
	int SetStr(const TCHAR* pKey, const TCHAR* pStr);

private:

	TCHAR m_FilePath[_MAX_PATH];
	TCHAR m_Section[_MAX_PATH];

	//代入とコピーコンストラクタの禁止
	void operator=(const YNConfFile&);
	YNConfFile(const YNConfFile&);

};


} // end of namespace

