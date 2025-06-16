//******************************************************************************
//
// Simple MIDI Library / SMRcpConv
//
// RCPファイル変換クラス
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "shlwapi.h"
#include "YNBaseLib.h"
#include "SMRcpConv.h"
#include <io.h>

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

	//関数ポインタ取得：rcpcvConvertFileFromBuffer
	m_pFuncConvertFileFromBuffer = (RCPCV_ConvertFileFromBuffer)GetProcAddress(m_hModule, "rcpcvConvertFileFromBuffer");
	if (m_pFuncConvertFileFromBuffer == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//関数ポインタ取得：rcpcvGetSMF
	m_pFuncGetSMF = (RCPCV_GetSMF)GetProcAddress(m_hModule, "rcpcvGetSMF");
	if (m_pFuncGetSMF == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//関数ポインタ取得：rcpcvGetSMFLength
	m_pFuncGetSMFLength = (RCPCV_GetSMFLength)GetProcAddress(m_hModule, "rcpcvGetSMFLength");
	if (m_pFuncGetSMFLength == NULL) {
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
		const WCHAR* pRCPPath,
		const WCHAR* pSMFPath
	)
{
	int result = 0;
	DWORD hRCPCV = 0;
	FILE* pRCPFile = NULL;
	FILE* pSMFFile = NULL;
	unsigned char* pBuffer = NULL;
	errno_t eresult = 0;
	__int64 fileSize = 0;
	size_t buffSize = 0;
	size_t readSize = 0;
	size_t writeSize = 0;
	LPCSTR pSMFData = NULL;
	int SMFDataSize = 0;

	if (!IsAvailable()) {
		result = YN_SET_ERR("Program Error.", 0, 0);
		goto EXIT;
	}

	//ファイルを開く
	eresult = _wfopen_s(&pRCPFile, pRCPPath, L"rb");
	if (eresult != 0) {
		result = YN_SET_ERR("File open error.", 0, 0);
		goto EXIT;
	}

	//ファイルサイズを確認
	fileSize = _filelengthi64(_fileno(pRCPFile));
	if (fileSize == -1L) {
		result = YN_SET_ERR("File open error.", 0, 0);
		goto EXIT;
	}
	//ファイルサイズが100MBを超える場合はサポートしない
	if (fileSize > (1024 * 1024 * 100)) {
		result = YN_SET_ERR("File size is too long.", fileSize, 0);
		goto EXIT;
	}
	buffSize = (size_t)fileSize;

	//メモリ確保
	try {
		pBuffer = new unsigned char[buffSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", buffSize, 0);
		goto EXIT;
	}

	//RCPファイルの内容をメモリに展開
	readSize = fread_s(pBuffer, buffSize, 1, buffSize, pRCPFile);
	if (readSize != buffSize) {
		result = YN_SET_ERR("File read error.", readSize, buffSize);
		goto EXIT;
	}

	try {
		//RCP->SMFコンバート実行
		//  フォーマット形式を渡す必要があるが自動判別とする
		//  メモリ上での変換はファイル拡張子の情報がないためと推定
		hRCPCV = (*m_pFuncConvertFileFromBuffer)(
						(LPCSTR)pBuffer,//入力バッファ
						(UINT)buffSize,		//入力バッファ長さ
						0,				//入力データのフォーマット形式：自動判別
						0,				//コールバック種別：なし
						NULL,			//コールバック関数／ウィンドウハンドル：なし
						0,				//ウィンドウメッセージ：なし
						0				//インスタンス判別用ID：なし
					);
		if (hRCPCV == 0) {
			result = YN_SET_ERR("File convert error. (rcpcv.dll)", 0, 0);
			goto EXIT;
		}

		//SMFデータ位置
		pSMFData = (*m_pFuncGetSMF)(hRCPCV);

		//SMFデータサイズ
		SMFDataSize = (*m_pFuncGetSMFLength)(hRCPCV);
	}
	catch (...) {
		result = YN_SET_ERR("File convert error. (rcpcv.dll)", 0, 0);
		goto EXIT;
	}

	//出力先ファイルを開く
	eresult = _wfopen_s(&pSMFFile, pSMFPath, L"wb");
	if (eresult != 0) {
		result = YN_SET_ERR("File open error.", 0, 0);
		goto EXIT;
	}

	//出力先ファイルにSMFデータを書き込む
	writeSize = fwrite(pSMFData, 1, SMFDataSize, pSMFFile);
	if (writeSize != SMFDataSize) {
		result = YN_SET_ERR("File write error.", writeSize, SMFDataSize);
		goto EXIT;
	}

EXIT:;
	delete [] pBuffer;
	if (hRCPCV != 0) {
		(*m_pFuncDeleteObject)(hRCPCV);
	}
	if (pRCPFile != NULL) {
		fclose(pRCPFile);
	}
	if (pSMFFile != NULL) {
		fclose(pSMFFile);
	}
	return result;
}

// メモ
// RCPCV はワイド文字列によるファイルパスの指定に対応していないため
// RCPCV でのRCPファイル読み込みとSMFファイル出力を取りやめ、
// メモリ上でのコンバート処理に変更する。
//******************************************************************************
// ファイル変換
//******************************************************************************
//int SMRcpConv::Convert(
//		const TCHAR* pRCPPath,
//		const TCHAR* pSMFPath
//	)
//{
//	int result = 0;
//	int apiresult = 0;
//	DWORD hRCPCV = 0;
//
//	if (!IsAvailable()) {
//		result = YN_SET_ERR("Program Error.", 0, 0);
//		goto EXIT;
//	}
//
//	try {
//
//		//RCPファイル読み込み
//		hRCPCV = (*m_pFuncConvertFile)(
//						pRCPPath,	//ファイルパス
//						0,			//コールバック種別：なし
//						NULL,		//コールバック関数／ウィンドウハンドル：なし
//						0,			//ウィンドウメッセージ：なし
//						0			//インスタンス判別用ID：なし
//					);
//		if (hRCPCV == 0) {
//			result = YN_SET_ERR("File read error.", 0, 0);
//			goto EXIT;
//		}
//
//		//SMF出力
//		apiresult = (*m_pFuncSaveSMF)(hRCPCV, pSMFPath);
//		if (apiresult != 1) {
//			result = YN_SET_ERR("File save error.", apiresult, 0);
//			goto EXIT;
//		}
//
//	}
//	catch (...) {
//		result = YN_SET_ERR("Exception occurred. (rcpcv.dll)", 0, 0);
//		goto EXIT;
//	}
//
//EXIT:;
//	if (hRCPCV != 0) {
//		(*m_pFuncDeleteObject)(hRCPCV);
//	}
//	return result;
//}

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
		const WCHAR* pFilePath
	)
{
	bool isSupport = false;

	if (YNPathUtil::IsFileExtMatch(pFilePath, L".rcp")
	 || YNPathUtil::IsFileExtMatch(pFilePath, L".r36")
	 || YNPathUtil::IsFileExtMatch(pFilePath, L".g36")) {
		isSupport = true;
	}

	return isSupport;
}

//******************************************************************************
// GetOpenFileName用ファイルフィルタ取得
//******************************************************************************
const WCHAR* SMRcpConv::GetOpenFileNameFilter()
{
	WCHAR* pFilter =
		L"MIDI File (*.mid *.rcp *.r36 *.g36)\0*.mid;*.rcp;*.r36;*.g36\0"
		L"Standard MIDI File (*.mid)\0*.mid\0"
		L"Recomposer Data File (*.rcp *.r36 *.g36)\0*.rcp;*.r36;*.g36\0"
		L"\0";
	return pFilter;
}

} // end of namespace

