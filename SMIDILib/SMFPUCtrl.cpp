//******************************************************************************
//
// Simple MIDI Library / SMFPUCtrl
//
// 浮動点小数プロセッサ制御クラス
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
// コンストラクタ
//******************************************************************************
SMFPUCtrl::SMFPUCtrl(void)
{
	m_ThreadID = 0;
	m_FPUCtrl = 0;
	m_isLock = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMFPUCtrl::~SMFPUCtrl(void)
{
	//設定開始したままであれば解除する
	if ((m_isLock) && (m_ThreadID == GetCurrentThreadId())) {
		unsigned int curCtrl = 0;
#ifndef _WIN64
		_controlfp_s(&curCtrl, m_FPUCtrl, _MCW_PC);
#endif
		m_isLock = false;
	}
}

//******************************************************************************
// 精度設定開始
//******************************************************************************
int SMFPUCtrl::Start(FPUPrecision precision)
{
	int result = 0;
	errno_t eresult = 0;
	unsigned int curCtrl = 0;
	unsigned int flag = 0;

	//すでに設定開始済みならエラー
	if (m_isLock) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//変更前の制御フラグを取得
	eresult = _controlfp_s(
					&m_FPUCtrl,	//現在の制御ワード
					0,			//制御ワード：なし
					0			//マスク：なし
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Windows API error.", eresult, GetLastError());
		goto EXIT;
	}

	_DisplayCurCtrl(_T("Start before"));

	//浮動小数点精度を設定
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
	//精度制御の必要なし
#else
	//x86(32bit)
	eresult = _controlfp_s(
					&curCtrl,	//現在の制御ワード
					flag,		//制御ワード：制御種別
					_MCW_PC		//マスク：精度制御
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
// 精度設定終了
//******************************************************************************
int SMFPUCtrl::End()
{
	int result = 0;
	errno_t eresult = 0;
	unsigned int curCtrl = 0;

	//設定開始していない場合はエラー
	if (!m_isLock) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//設定開始時と異なるスレッドで設定終了することはできない
	if (m_ThreadID != GetCurrentThreadId()) {
		result = YN_SET_ERR("Program error.", m_ThreadID, GetCurrentThreadId());
		goto EXIT;
	}

	//浮動小数点精度を復元する
#ifdef _WIN64
	//x64(64bit)
	//精度制御の必要なし
#else
	//x86(32bit)
	eresult = _controlfp_s(
					&curCtrl,	//現在の制御ワード
					m_FPUCtrl,	//制御ワード：設定開始時点
					_MCW_PC		//マスク：精度制御
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
// 精度設定状態確認
//******************************************************************************
bool SMFPUCtrl::IsLocked()
{
	return m_isLock;
}

//******************************************************************************
// 浮動小数点制御ワード表示
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
//			(fpuctrl & _MCW_DN), //DENORMAL制御
//			(fpuctrl & _MCW_EM), //割り込み例外マスク
//			(fpuctrl & _MCW_IC), //無限制御
//			(fpuctrl & _MCW_RC), //丸め制御
//			(fpuctrl & _MCW_PC)  //精度制御
//		);
//
//	MessageBox(NULL, msg, pTitle, MB_OK);
//
//EXIT:;
//	return;
}

} // end of namespace


