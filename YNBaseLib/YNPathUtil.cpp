//******************************************************************************
//
// Simple Base Library / YNPathUtil
//
// パスユーティリティクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
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
// コンストラクタ
//******************************************************************************
YNPathUtil::YNPathUtil(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
YNPathUtil::~YNPathUtil(void)
{
}

//******************************************************************************
// プロセス実行ファイルディレクトリパス取得
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

	//プロセス実行ファイルパス取得
	apiresult = GetModuleFileName(GetModuleHandle(NULL), path, _MAX_PATH);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//パス要素の分割
	eresult = _tsplitpath_s(
					path,		//パス
					drive,		//ドライブ文字列バッファ
					_MAX_DRIVE,	//バッファサイズ
					dir,		//ディレクトリ文字列バッファ
					_MAX_DIR,	//バッファサイズ
					fname,		//ファイル名文字列バッファ
					_MAX_FNAME,	//バッファサイズ
					ext,		//拡張子文字列バッファ
					_MAX_EXT	//バッファサイズ
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//パス作成
	eresult = _tmakepath_s(
					pBuf,		//パス格納先バッファ
					bufSize,	//バッファサイズ
					drive,		//ドライブ文字列
					dir,		//ディレクトリ文字列
					NULL,		//ファイル名文字列
					NULL		//拡張子文字列
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// アプリケーションデータディレクトリパス取得
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
					NULL,				//オーナーウィンドウ
					CSIDL_APPDATA,		//フォルダ指定
					NULL,				//アクセストークン
					SHGFP_TYPE_CURRENT,	//フラグ：現在のフォルダパス
										//  ユーザが変更している可能性がある
					path				//パス格納先バッファ
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
// 拡張子判定
//******************************************************************************
bool YNPathUtil::IsFileExtMatch(
		const TCHAR* pPath,
		const TCHAR* pExt
	)
{
	bool isMatch = false;
	errno_t eresult = 0;
	TCHAR ext[_MAX_EXT] = {_T('\0')};

	//パス要素を分割して拡張子を取得
	eresult = _tsplitpath_s(
					pPath,			//パス
					NULL, 0,		//ドライブ文字列バッファとサイズ
					NULL, 0,		//ディレクトリ文字列バッファとサイズ
					NULL, 0,		//ファイル名文字列バッファとサイズ
					ext, _MAX_EXT	//拡張子文字列バッファとサイズ
				);
	if (eresult != 0) {
		//result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//大文字と小文字を区別せずに拡張子を比較する
	if (_tcsicmp(ext, pExt) == 0) {
		isMatch = true;
	}

EXIT:;
	return isMatch;
}

//******************************************************************************
// テンポラリファイルパス取得
//******************************************************************************
int YNPathUtil::GetTempFilePath(
		TCHAR* pPathBuf,
		unsigned long bufSize,
		const TCHAR* pPrefix
	)
{
	int result = 0;
	DWORD apiresult = 0;
	TCHAR tempDir[_MAX_PATH] = {_T('\0')};

	//テンポラリディレクトリパスを取得
	apiresult = GetTempPath(_MAX_PATH, tempDir);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//GetTmpFileNameはバッファサイズを指定できない奇妙なAPIである
	//「バッファサイズはMAX_PATH以上にせよ」と定義されているので
	//サイズチェックを行う
	if (bufSize < MAX_PATH) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//テンポラリファイルパスを取得
	//  ファイル名：PREuuuu.TMP
	//    PRE ：プレフィックス
	//    uuuu：システム時刻に基づいて生成された16進文字列
	apiresult = GetTempFileName(
						tempDir,	//ディレクトリパス
						pPrefix,	//プレフィックス（3文字）
						0,			//一意性：有効
						pPathBuf	//生成されたファイルパス
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

} // end of namespace

