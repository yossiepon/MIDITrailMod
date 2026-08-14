//******************************************************************************
//
// Simple MIDI Library / SMRcpConv
//
// RCP-format file conversion class.
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
// Constructor
//******************************************************************************
SMRcpConv::SMRcpConv(void)
{
	m_hModule = NULL;
	m_pFuncConvertFile = NULL;
	m_pFuncSaveSMF = NULL;
	m_pFuncDeleteObject = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMRcpConv::~SMRcpConv(void)
{
	_Release();
}

//******************************************************************************
// Initialize
//******************************************************************************
int SMRcpConv::Initialize()
{
	int result = 0;
	TCHAR dllFilePath[_MAX_PATH] = {_T('\0')};

	_Release();

	// Get process executable directory path
	result = YNPathUtil::GetModuleDirPath(dllFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	// DLL file path
	_tcscat_s(dllFilePath, _MAX_PATH, _T("RCPCV.DLL"));

	// Check DLL existence
	if (!PathFileExists(dllFilePath)) {
		// DLL does not exist, so finish normally without doing anything
		goto EXIT;
	}

	// Load DLL
	//  LoadLibrary / FreeLibrary manage the reference count on the API side,
	//  so it is not a problem even if multiple instances of this class exist
	m_hModule = LoadLibrary(dllFilePath);
	if (m_hModule == NULL) {
		result = YN_SET_ERR("LoadLibrary Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	// Get function pointer: rcpcvConvertFile
	m_pFuncConvertFile = (RCPCV_ConvertFile)GetProcAddress(m_hModule, "rcpcvConvertFile");
	if (m_pFuncConvertFile == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	// Get function pointer: rcpcvSaveSMF
	m_pFuncSaveSMF = (RCPCV_SaveSMF)GetProcAddress(m_hModule, "rcpcvSaveSMF");
	if (m_pFuncSaveSMF == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	// Get function pointer: rcpcvDeleteObject
	m_pFuncDeleteObject = (RCPCV_DeleteObject)GetProcAddress(m_hModule, "rcpcvDeleteObject");
	if (m_pFuncDeleteObject == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	// Get function pointer: rcpcvConvertFileFromBuffer
	m_pFuncConvertFileFromBuffer = (RCPCV_ConvertFileFromBuffer)GetProcAddress(m_hModule, "rcpcvConvertFileFromBuffer");
	if (m_pFuncConvertFileFromBuffer == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	// Get function pointer: rcpcvGetSMF
	m_pFuncGetSMF = (RCPCV_GetSMF)GetProcAddress(m_hModule, "rcpcvGetSMF");
	if (m_pFuncGetSMF == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	// Get function pointer: rcpcvGetSMFLength
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
// Determine availability
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
// File conversion
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

	// Open the file
	eresult = _wfopen_s(&pRCPFile, pRCPPath, L"rb");
	if (eresult != 0) {
		result = YN_SET_ERR("File open error.", 0, 0);
		goto EXIT;
	}

	// Check the file size
	fileSize = _filelengthi64(_fileno(pRCPFile));
	if (fileSize == -1L) {
		result = YN_SET_ERR("File open error.", 0, 0);
		goto EXIT;
	}
	// Not supported if the file size exceeds 100MB
	if (fileSize > (1024 * 1024 * 100)) {
		result = YN_SET_ERR("File size is too long.", fileSize, 0);
		goto EXIT;
	}
	buffSize = (size_t)fileSize;

	// Allocate memory
	try {
		pBuffer = new unsigned char[buffSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", buffSize, 0);
		goto EXIT;
	}

	// Load the RCP file content into memory
	readSize = fread_s(pBuffer, buffSize, 1, buffSize, pRCPFile);
	if (readSize != buffSize) {
		result = YN_SET_ERR("File read error.", readSize, buffSize);
		goto EXIT;
	}

	try {
		// Perform RCP->SMF conversion
		//  the format type must be passed, but auto-detection is used here
		//  presumably because a conversion in memory has no file extension info available
		hRCPCV = (*m_pFuncConvertFileFromBuffer)(
						(LPCSTR)pBuffer,//input buffer
						(UINT)buffSize,		//input buffer length
						0,				//input data format type: auto-detect
						0,				//callback type: none
						NULL,			//callback function / window handle: none
						0,				//window message: none
						0				//instance identification ID: none
					);
		if (hRCPCV == 0) {
			result = YN_SET_ERR("File convert error. (rcpcv.dll)", 0, 0);
			goto EXIT;
		}

		// SMF data position
		pSMFData = (*m_pFuncGetSMF)(hRCPCV);

		// SMF data size
		SMFDataSize = (*m_pFuncGetSMFLength)(hRCPCV);
	}
	catch (...) {
		result = YN_SET_ERR("File convert error. (rcpcv.dll)", 0, 0);
		goto EXIT;
	}

	// Open the output file
	eresult = _wfopen_s(&pSMFFile, pSMFPath, L"wb");
	if (eresult != 0) {
		result = YN_SET_ERR("File open error.", 0, 0);
		goto EXIT;
	}

	// Write the SMF data to the output file
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

// Note
// Since RCPCV does not support specifying file paths with wide-character strings,
// RCPRead file and SMF file output via RCPCV are dropped,
// and the conversion is changed to be done in memory instead.
//******************************************************************************
// File conversion
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
//		//RCPRead file
//		hRCPCV = (*m_pFuncConvertFile)(
//						pRCPPath,	//file path
//						0,			//callback type: none
//						NULL,		//callback function / window handle: none
//						0,			//window message: none
//						0			//instance identification ID: none
//					);
//		if (hRCPCV == 0) {
//			result = YN_SET_ERR("File read error.", 0, 0);
//			goto EXIT;
//		}
//
//		//SMF output
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
// Release
//******************************************************************************
void SMRcpConv::_Release()
{
	if (m_hModule != NULL) {
		FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

//******************************************************************************
// Determine supported file by extension
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
// Get file filter for GetOpenFileName
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

