//******************************************************************************
//
// Simple MIDI Library / SMRcpConv
//
// RCP-format file conversion class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Uses RCPCV.DLL(*1), published by Fumii, to convert Recomposer
// data files (*.rcp,*.r36,*.g36) into Standard MIDI Files.
// Available only when RCPCV.DLL exists in the same folder as the application.
// If RCPCV.DLL does not exist, this class's functionality is unavailable.
// Check availability beforehand using the IsAvailable() method.
//
// (*1) RCPCV.DLL
// http://www.vector.co.jp/soft/win95/art/se114143.html

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <windows.h>

namespace SMIDILib {

//******************************************************************************
// SysEx event class
//******************************************************************************
class SMIDILIB_API SMRcpConv
{
public:

	//Constructor / Destructor
	SMRcpConv();
	virtual ~SMRcpConv(void);

	//Initialize
	int Initialize();

	//Determine availability
	bool IsAvailable();

	//Convert to Standard MIDI File
	int Convert(const WCHAR* pRCPPath, const WCHAR* pSMFPath);

	//Determine supported file by extension
	bool IsSupportFileExt(const WCHAR* pFilePath);

	//Get file filter for GetOpenFileName
	const WCHAR* GetOpenFileNameFilter();

private:

	HMODULE m_hModule;

	//RCPCV.DLL API definitions
	typedef DWORD  (WINAPI *RCPCV_ConvertFile)(LPCSTR, UINT, DWORD, UINT, DWORD);
	typedef int    (WINAPI *RCPCV_SaveSMF)(DWORD, LPCSTR);
	typedef void   (WINAPI *RCPCV_DeleteObject)(DWORD);
	typedef DWORD  (WINAPI *RCPCV_ConvertFileFromBuffer)(LPCSTR, UINT, UINT, UINT, DWORD, UINT, DWORD);
	typedef LPCSTR (WINAPI *RCPCV_GetSMF)(DWORD);
	typedef int    (WINAPI *RCPCV_GetSMFLength)(DWORD);

	//Function pointers
	RCPCV_ConvertFile           m_pFuncConvertFile;
	RCPCV_SaveSMF               m_pFuncSaveSMF;
	RCPCV_DeleteObject          m_pFuncDeleteObject;
	RCPCV_ConvertFileFromBuffer m_pFuncConvertFileFromBuffer;
	RCPCV_GetSMF                m_pFuncGetSMF;
	RCPCV_GetSMFLength          m_pFuncGetSMFLength;

	void _Release();

};

} // end of namespace

