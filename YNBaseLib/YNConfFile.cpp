//******************************************************************************
//
// Simple Base Library / YNConfFile
//
// 設定ファイルクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNConfFile.h"
#include <stdio.h>
#include <stdlib.h>

namespace YNBaseLib {

//******************************************************************************
// パラメータ定義
//******************************************************************************
#define YNCONFFILE_NO_DATA  _T("*** NO DATA ***")

//******************************************************************************
// コンストラクタ
//******************************************************************************
YNConfFile::YNConfFile(void)
{
	m_FilePath[0] = _T('\0');
	m_Section[0] = _T('\0');
}

//******************************************************************************
// デストラクタ
//******************************************************************************
YNConfFile::~YNConfFile(void)
{
}

//******************************************************************************
// 初期化
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
// セクション設定
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
// 整数値取得
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
					m_Section,			//セクション名
					pKey,				//キー名
					YNCONFFILE_NO_DATA, //デフォルト文字列
					buf,				//バッファ位置
					20,					//バッファサイズ（TCHAR単位）
					m_FilePath			//ファイルパス
				);
	//戻り値のチェックはあきらめる

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
// 整数値登録
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
					m_Section,		//セクション名
					pKey,			//キー名
					buf,			//登録する文字列
					m_FilePath		//ファイルパス
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// 浮動小数値取得
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
					m_Section,			//セクション名
					pKey,				//キー名
					YNCONFFILE_NO_DATA, //デフォルト文字列
					buf,				//バッファ位置
					20,					//バッファサイズ（TCHAR単位）
					m_FilePath			//ファイルパス
				);
	//戻り値のチェックはあきらめる

	if (_tcscmp(buf, YNCONFFILE_NO_DATA) == 0) {
		*pVal = defaultVal;
	}
	else {
		//_tstofはdoubleを返す
		*pVal = (float)_tstof(buf);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// 浮動小数値登録
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
					m_Section,		//セクション名
					pKey,			//キー名
					buf,			//登録する文字列
					m_FilePath		//ファイルパス
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// 文字列取得
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
					m_Section,			//セクション名
					pKey,				//キー名
					pDefaultVal,		//デフォルト文字列
					pBuf,				//バッファ位置
					bufSize,			//バッファサイズ（TCHAR単位）
					m_FilePath			//ファイルパス
				);
	//戻り値のチェックはあきらめる

//EXIT:;
	return result;
}

//******************************************************************************
// 文字列登録
//******************************************************************************
int YNConfFile::SetStr(
		const TCHAR* pKey,
		const TCHAR* pStr
	)
{
	int result = 0;
	BOOL bresult = TRUE;

	bresult = WritePrivateProfileString(
					m_Section,		//セクション名
					pKey,			//キー名
					pStr,			//登録する文字列
					m_FilePath		//ファイルパス
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}

EXIT:;
	return result;
}

} // end of namespace

