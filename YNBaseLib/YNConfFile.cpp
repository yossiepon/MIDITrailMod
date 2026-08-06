//******************************************************************************
//
// Simple Base Library / YNConfFile
//
// 設定ファイルクラス
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNErrCtrl.h"
#include "YNConfFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <new>

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
	TCHAR* pValue = NULL;
	size_t length = 0;

	//INIファイルに末尾が空白文字の値を登録すると
	//値を取得するときに空白文字を削除されてしまうため
	//シングルクォートで囲んで登録する
	length = _tcslen(pStr) + 4;
	try {
		pValue = new TCHAR[length];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", length, 0);
		goto EXIT;
	}
	_stprintf_s(pValue, length, _T("'%s'"), pStr);

	bresult = WritePrivateProfileString(
					m_Section,		//セクション名
					pKey,			//キー名
					pValue,			//登録する文字列
					m_FilePath		//ファイルパス
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}

EXIT:;
	delete [] pValue;
	return result;
}

//******************************************************************************
// 文字列取得（値のみワイド文字列）
//******************************************************************************
int YNConfFile::GetWStr(
		const TCHAR* pKey,
		WCHAR* pBuf,
		unsigned long bufSize,
		const WCHAR* pDefaultVal
	)
{
	int result = 0;
	unsigned long hexBufSize = 0;
	unsigned long hexLength = 0;
	unsigned long index = 0;
	unsigned long indexw = 0;
	TCHAR* pHexString = NULL;
	TCHAR hexChar[5];
	TCHAR* stopped = NULL;
	WCHAR wchar = 0;

	//バッファサイズがデフォルト値を格納できなければエラー
	if (bufSize < (wcslen(pDefaultVal) + 1)) {
		result = YN_SET_ERR("Program Error.", bufSize, 0);
		goto EXIT;
	}

	hexBufSize = (bufSize * 4) + (unsigned long)_tcslen(YNCONFFILE_NO_DATA) + 1;

	//16進数文字列を格納するメモリを確保
	try {
		pHexString = new TCHAR[hexBufSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", hexBufSize, 0);
		goto EXIT;
	}
	memset(pHexString, 0, hexBufSize);

	//16進数文字列を取得
	result = GetStr(pKey, pHexString, hexBufSize, YNCONFFILE_NO_DATA);
	if (result != 0) goto EXIT;
	
	//未登録の場合はデフォルト文字列を返す
	if (_tcscmp(pHexString, YNCONFFILE_NO_DATA) == 0) {
		wcscpy_s(pBuf, bufSize, pDefaultVal);
		goto EXIT;
	}

	hexLength = (unsigned long)_tcslen(pHexString);

	//空文字列の場合
	if (hexLength == 0) {
		pBuf[0] = L'\0';
		goto EXIT;
	}

	//16進数文字列を4文字ずつワイド文字に変換
	//末尾が4文字単位でなければ切り捨てる
	while ((index + 4) <= hexLength) {
		hexChar[0] = pHexString[index + 0];
		hexChar[1] = pHexString[index + 1];
		hexChar[2] = pHexString[index + 2];
		hexChar[3] = pHexString[index + 3];
		hexChar[4] = '\0';
		pBuf[indexw] = (WCHAR)_tcstol(hexChar, &stopped, 16);

		//バッファ終端であれば変換を終了
		if ((indexw + 1) == bufSize) {
			break;
		}

		index += 4;
		indexw += 1;
	}
	pBuf[indexw] = L'\0';

EXIT:;
	delete [] pHexString;
	return result;
}

//******************************************************************************
// 文字列登録（値のみワイド文字列）
//******************************************************************************
int YNConfFile::SetWStr(const TCHAR* pKey, const WCHAR* pStr)
{
	int result = 0;
	unsigned long length = 0;
	unsigned long bufSize = 0;
	unsigned long index = 0;
	TCHAR* pHexString = NULL;
	TCHAR hexChar[5];

	length = (unsigned long)wcslen(pStr);
	bufSize = (length + 1) * 4;

	//16進数文字列を格納するメモリを確保
	try {
		pHexString = new TCHAR[bufSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", bufSize, 0);
		goto EXIT;
	}
	memset(pHexString, 0, bufSize);

	//ワイド文字文字列を1文字ずつ16進数4文字に変換（終端文字は変換しない）
	for (index = 0; index < length; index ++) {
		_stprintf_s(hexChar, 5, _T("%04X"), pStr[index]);
		_tcscat_s(pHexString, bufSize, hexChar);
	}

	//文字列登録
	result = SetStr(pKey, pHexString);
	if (result != 0) goto EXIT;

EXIT:;
	delete [] pHexString;
	return result;
}

} // end of namespace


