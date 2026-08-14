//******************************************************************************
//
// YN Base Library / YNErrCtrl
//
// Error control class.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNErrCtrl.h"

namespace YNBaseLib {

DWORD g_TlsIndex = 0xFFFFFFFF;

//******************************************************************************
// Constructor
//******************************************************************************
YNErrCtrl::YNErrCtrl()
{
	return;
}

//******************************************************************************
// Destructor
//******************************************************************************
YNErrCtrl::~YNErrCtrl()
{
	return;
}

//******************************************************************************
// Initialize
//******************************************************************************
int YNErrCtrl::Initialize()
{
	int result = 0;

	g_TlsIndex = TlsAlloc();
	if (g_TlsIndex == 0xFFFFFFFF) {
		result = -1;
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Terminate
//******************************************************************************
int YNErrCtrl::Terminate()
{
	int result = 0;
	BOOL apiresult = false;

	if (g_TlsIndex != 0xFFFFFFFF) {
		apiresult = TlsFree(g_TlsIndex);
		if (!apiresult) {
			result = -1;
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Register error information
//******************************************************************************
int YNErrCtrl::SetErr(
		YNErrInfo::ErrLevel errLevel,
		unsigned long lineNo,
		const TCHAR* pFuncName,
		const TCHAR* pMessage,
		unsigned long long errInfo1,
		unsigned long long errInfo2
	)
{
	int result = 0;
	BOOL apiresult = false;
	YNErrInfo* pErrInfo = NULL;

	//Discard any error information that's still registered
	pErrInfo = GetErr();
	if (pErrInfo != NULL) {
		delete pErrInfo;
		pErrInfo = NULL;
	}

	//Create the error information object
	pErrInfo = new YNErrInfo(errLevel, lineNo, pFuncName, pMessage, errInfo1, errInfo2);
	if (pErrInfo == NULL) {
		result = -2;
		goto EXIT;
	}

	//Store it in thread-local storage
	apiresult = TlsSetValue(g_TlsIndex, (void*)pErrInfo);
	if (!apiresult) {
		result = -2;
		goto EXIT;
	}
	pErrInfo = NULL;

	//TODO: generate an error code
	result = -1;

EXIT:;
	delete pErrInfo;
	return result;
}

//******************************************************************************
// Get error information
//******************************************************************************
YNErrInfo* YNErrCtrl::GetErr()
{
	int result = 0;
	BOOL apiresult = false;
	YNErrInfo* pErrInfo = NULL;

	//Get the error information object from thread-local storage
	pErrInfo = (YNErrInfo*)TlsGetValue(g_TlsIndex);
	if (pErrInfo == NULL) {
		result = -1;
		goto EXIT;
	}

	//Clear thread-local storage
	apiresult = TlsSetValue(g_TlsIndex, NULL);
	if (!apiresult) {
		result = -1;
		goto EXIT;
	}

EXIT:;
	return pErrInfo;
}

//******************************************************************************
// Show error
//******************************************************************************
int YNErrCtrl::ShowErr(
		HWND hOwner
	)
{
	int result = 0;
	int apiresult = 0;
	UINT type = 0;
	YNErrInfo* pErrInfo = NULL;
	TCHAR msgex[512];

#ifdef _UNICODE
	wstring msg;
	wstring title;
#else
	string msg;
	string title;
#endif

	//Do nothing if there's no error information
	pErrInfo = GetErr();
	if (pErrInfo == NULL) goto EXIT;

	switch (pErrInfo->GetErrLevel()) {
		case (YNErrInfo::LVL_ERR):
			type |= MB_ICONERROR;
			title = _T("ERROR");
			break;
		case (YNErrInfo::LVL_WARN):
			type |= MB_ICONWARNING;
			title = _T("WARNING");
			break;
		case (YNErrInfo::LVL_INFO):
			type |= MB_ICONINFORMATION;
			title = _T("INFORMATION");
			break;
	}

	if (hOwner == NULL) {
		type |= MB_SYSTEMMODAL;
	}

	msg = pErrInfo->GetMessage();
	_stprintf_s(
		msgex,
		512,
		_T("\n\nFUNC: %s\nLINE: %d\nINFO: %08llX %08llX"),
		pErrInfo->GetFuncName(),
		pErrInfo->GetLineNo(),
		pErrInfo->GetErrInfo1(),
		pErrInfo->GetErrInfo2()
	);
	msg += msgex;

	apiresult = MessageBox(
					hOwner,			//Owner window
					msg.c_str(),	//Message
					title.c_str(),	//Title
					MB_OK | type	//Flags
				);
	if (apiresult == 0) {
		result = -1;
		goto EXIT;
	}

EXIT:;
	delete pErrInfo;
	return result;
}

} // end of namespace

