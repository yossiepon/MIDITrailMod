//******************************************************************************
//
// Simple Base Library / YNPathUtil
//
// パスユーティリティクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
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
// パスユーティリティクラス
//******************************************************************************
class YNBASELIB_API YNPathUtil
{
public:

	//プロセス実行ファイルディレクトリパス取得
	//  末尾に"\"を付与する
	//  取得パスの例："C:\Program Files\AppName\"
	static int GetModuleDirPath(TCHAR* pBuf, unsigned long bufSize);

	//アプリケーションデータディレクトリパス取得
	//  末尾に"\"を付与する
	//  取得パスの例：Widows7の場合 "C:\Users\UserName\AppData\Roaming\"
	static int GetAppDataDirPath(TCHAR* pBuf, unsigned long bufSize);

	//拡張子判定
	//  ファイルの拡張子が指定されたものであるか判定する
	//  指定する拡張子の例：".txt"
	static bool IsFileExtMatch(const TCHAR* pPath, const TCHAR* pExt);

	//テンポラリファイルパス取得
	//  環境変数(TMP or TEMP)で定義されたテンポラリディレクトリに
	//  一意のテンポラリファイルを作成してパスを返却する
	//  指定できるプレフィックスは3文字
	//  作成されるファイルの名称は PREuuuu.TMP
	static int GetTempFilePath(TCHAR* pPathBuf, unsigned long bufSize, const TCHAR* pPrefix);

private:

	//コンストラクタ／デストラクタ
	YNPathUtil(void);
	virtual ~YNPathUtil(void);

	//代入とコピーコンストラクタの禁止
	void operator=(const YNPathUtil&);
	YNPathUtil(const YNPathUtil&);

};

} // end of namespace

