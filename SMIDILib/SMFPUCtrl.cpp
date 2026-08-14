//******************************************************************************
//
// Simple MIDI Library / SMFPUCtrl
//
// Floating-point unit precision control class.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMFPUCtrl.h"
#include <float.h>

#pragma fenv_access (on)

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMFPUCtrl::SMFPUCtrl(void)
{
	m_ThreadID = 0;
	m_FPUCtrl = 0;
	m_isLock = false;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMFPUCtrl::~SMFPUCtrl(void)
{
	//Release if still active
	if ((m_isLock) && (m_ThreadID == GetCurrentThreadId())) {
		unsigned int curCtrl = 0;
#ifndef _WIN64
		_controlfp_s(&curCtrl, m_FPUCtrl, _MCW_PC);
#endif
		m_isLock = false;
	}
}

//******************************************************************************
// Start precision setting
//******************************************************************************
int SMFPUCtrl::Start(FPUPrecision precision)
{
	int result = 0;
	errno_t eresult = 0;
	unsigned int curCtrl = 0;
	unsigned int flag = 0;

	//Error if already started
	if (m_isLock) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Get control flags before change
	eresult = _controlfp_s(
					&m_FPUCtrl,	//Current control word
					0,			//Control word: none
					0			//Mask: none
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Windows API error.", eresult, GetLastError());
		goto EXIT;
	}

	_DisplayCurCtrl(_T("Start before"));

	//Set floating-point precision
	switch (precision) {
		case FPUSingle:
			flag = _PC_24;
			break;
		case FPUDouble:
			flag = _PC_53;
			break;
		case FPUExtended:
			flag = _PC_64;
			break;
		default:
			result = YN_SET_ERR("Program error.", 0, 0);
			goto EXIT;
	}

#ifdef _WIN64
	//x64(64bit)
	//No precision control needed
#else
	//x86(32bit)
	eresult = _controlfp_s(
					&curCtrl,	//Current control word
					flag,		//Control word: control type
					_MCW_PC		//Mask: precision control
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Windows API error.", eresult, GetLastError());
		goto EXIT;
	}
#endif

	_DisplayCurCtrl(_T("Start after"));

	m_ThreadID = GetCurrentThreadId();
	m_isLock = true;

EXIT:;
	return result;
}

//******************************************************************************
// End precision setting
//******************************************************************************
int SMFPUCtrl::End()
{
	int result = 0;
	errno_t eresult = 0;
	unsigned int curCtrl = 0;

	//Error if not started
	if (!m_isLock) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Cannot end on a thread different from the one that started it
	if (m_ThreadID != GetCurrentThreadId()) {
		result = YN_SET_ERR("Program error.", m_ThreadID, GetCurrentThreadId());
		goto EXIT;
	}

	//Restore floating-point precision
#ifdef _WIN64
	//x64(64bit)
	//No precision control needed
#else
	//x86(32bit)
	eresult = _controlfp_s(
					&curCtrl,	//Current control word
					m_FPUCtrl,	//Control word: value at start
					_MCW_PC		//Mask: precision control
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Windows API error.", eresult, GetLastError());
		goto EXIT;
	}
#endif

	_DisplayCurCtrl(_T("End after"));

	m_ThreadID = 0;
	m_FPUCtrl = 0;
	m_isLock = false;

EXIT:;
	return result;
}

//******************************************************************************
// Check precision setting state
//******************************************************************************
bool SMFPUCtrl::IsLocked()
{
	return m_isLock;
}

//******************************************************************************
// Display floating-point control word
//******************************************************************************
void SMFPUCtrl::_DisplayCurCtrl(
		TCHAR* pTitle
	)
{
//	errno_t eresult = 0;
//	unsigned int fpuctrl = 0;
//	TCHAR msg[256];
//
//	eresult = _controlfp_s(&fpuctrl, 0, 0);
//	if (eresult != 0) goto EXIT;
//
//	_stprintf_s(
//			msg,
//			256,
//			_T("Thread ID %08X\n")
//			_T("FPUCTRL %08X\n")
//			_T("_MCW_DN %08X\n")
//			_T("_MCW_EM %08X\n")
//			_T("_MCW_IC %08X\n")
//			_T("_MCW_RC %08X\n")
//			_T("_MCW_PC %08X"),
//			GetCurrentThreadId(),
//			fpuctrl,
//			(fpuctrl & _MCW_DN), //DENORMAL control
//			(fpuctrl & _MCW_EM), //Interrupt exception mask
//			(fpuctrl & _MCW_IC), //Infinity control
//			(fpuctrl & _MCW_RC), //Rounding control
//			(fpuctrl & _MCW_PC)  //Precision control
//		);
//
//	MessageBox(NULL, msg, pTitle, MB_OK);
//
//EXIT:;
//	return;
}

} // end of namespace


