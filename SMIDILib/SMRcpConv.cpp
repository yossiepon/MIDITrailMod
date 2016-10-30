//******************************************************************************
//
// Simple MIDI Library / SMRcpConv
//
// RCPファイル変換クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "shlwapi.h"
#include "YNBaseLib.h"
#include "SMRcpConv.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMRcpConv::SMRcpConv(void)
{
	m_hModule = NULL;
	m_pFuncConvertFile = NULL;
	m_pFuncSaveSMF = NULL;
	m_pFuncDeleteObject = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMRcpConv::~SMRcpConv(void)
{
	_Release();
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMRcpConv::Initialize()
{
	int result = 0;
	TCHAR dllFilePath[_MAX_PATH] = {_T('\0')};

	_Release();

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPath(dllFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//DLLファイルパス
	_tcscat_s(dllFilePath, _MAX_PATH, _T("RCPCV.DLL"));

	//DLL存在確認
	if (!PathFileExists(dllFilePath)) {
		//DLLが存在しないので何もせず正常終了
		goto EXIT;
	}

	//DLL読み込み
	//  LoadLibrary / FreeLibrary はAPI側で参照カウントを管理するため
	//  本クラスのインスタンスが複数存在しても問題ない
	m_hModule = LoadLibrary(dllFilePath);
	if (m_hModule == NULL) {
		result = YN_SET_ERR("LoadLibrary Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//関数ポインタ取得：rcpcvConvertFile
	m_pFuncConvertFile = (RCPCV_ConvertFile)GetProcAddress(m_hModule, "rcpcvConvertFile");
	if (m_pFuncConvertFile == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//関数ポインタ取得：rcpcvSaveSMF
	m_pFuncSaveSMF = (RCPCV_SaveSMF)GetProcAddress(m_hModule, "rcpcvSaveSMF");
	if (m_pFuncSaveSMF == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//関数ポインタ取得：rcpcvDeleteObject
	m_pFuncDeleteObject = (RCPCV_DeleteObject)GetProcAddress(m_hModule, "rcpcvDeleteObject");
	if (m_pFuncDeleteObject == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	if (result != 0) {
		_Release();
	}
	return result;
}

//******************************************************************************
// 使用可否判定
//******************************************************************************
bool SMRcpConv::IsAvailable()
{
	bool isAvailable = false;

	if (m_hModule != NULL) {
		isAvailable = true;
	}

	return isAvailable;
}

//******************************************************************************
// ファイル変換
//******************************************************************************
int SMRcpConv::Convert(
		const TCHAR* pRCPPath,
		const TCHAR* pSMFPath
	)
{
	int result = 0;
	int apiresult = 0;
	DWORD hRCPCV = 0;

	if (!IsAvailable()) {
		result = YN_SET_ERR("Program Error.", 0, 0);
		goto EXIT;
	}

	try {

		//RCPファイル読み込み
		hRCPCV = (*m_pFuncConvertFile)(
						pRCPPath,	//ファイルパス
						0,			//コールバック種別：なし
						NULL,		//コールバック関数／ウィンドウハンドル：なし
						0,			//ウィンドウメッセージ：なし
						0			//インスタンス判別用ID：なし
					);
		if (hRCPCV == 0) {
			result = YN_SET_ERR("File read error.", 0, 0);
			goto EXIT;
		}

		//SMF出力
		apiresult = (*m_pFuncSaveSMF)(hRCPCV, pSMFPath);
		if (apiresult != 1) {
			result = YN_SET_ERR("File save error.", apiresult, 0);
			goto EXIT;
		}

	}
	catch (...) {
		result = YN_SET_ERR("Exception occurred. (rcpcv.dll)", 0, 0);
		goto EXIT;
	}

EXIT:;
	if (hRCPCV != 0) {
		(*m_pFuncDeleteObject)(hRCPCV);
	}
	return result;
}

//******************************************************************************
// リリース
//******************************************************************************
void SMRcpConv::_Release()
{
	if (m_hModule != NULL) {
		FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

//******************************************************************************
// 拡張子によるサポート対象ファイル判定
//******************************************************************************
bool SMRcpConv::IsSupportFileExt(
		const TCHAR* pFilePath
	)
{
	bool isSupport = false;

	if (YNPathUtil::IsFileExtMatch(pFilePath, ".rcp")
	 || YNPathUtil::IsFileExtMatch(pFilePath, ".r36")
	 || YNPathUtil::IsFileExtMatch(pFilePath, ".g36")) {
		isSupport = true;
	}

	return isSupport;
}

//******************************************************************************
// GetOpenFileName用ファイルフィルタ取得
//******************************************************************************
const TCHAR* SMRcpConv::GetOpenFileNameFilter()
{
	TCHAR* pFilter =
		_T("MIDI File (*.mid *.rcp *.r36 *.g36)\0*.mid;*.rcp;*.r36;*.g36\0")
		_T("Standard MIDI File (*.mid)\0*.mid\0")
		_T("Recomposer Data File (*.rcp *.r36 *.g36)\0*.rcp;*.r36;*.g36\0")
		_T("\0");
	return pFilter;
}

} // end of namespace

