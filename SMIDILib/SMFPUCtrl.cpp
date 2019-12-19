//******************************************************************************
//
// Simple MIDI Library / SMFPUCtrl
//
// �����_�����v���Z�b�T����N���X
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
// �R���X�g���N�^
//******************************************************************************
SMFPUCtrl::SMFPUCtrl(void)
{
	m_ThreadID = 0;
	m_FPUCtrl = 0;
	m_isLock = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMFPUCtrl::~SMFPUCtrl(void)
{
	//�ݒ�J�n�����܂܂ł���Ή�������
	if ((m_isLock) && (m_ThreadID == GetCurrentThreadId())) {
		unsigned int curCtrl = 0;
#ifndef _WIN64
		_controlfp_s(&curCtrl, m_FPUCtrl, _MCW_PC);
#endif
		m_isLock = false;
	}
}

//******************************************************************************
// ���x�ݒ�J�n
//******************************************************************************
int SMFPUCtrl::Start(FPUPrecision precision)
{
	int result = 0;
	errno_t eresult = 0;
	unsigned int curCtrl = 0;
	unsigned int flag = 0;

	//���łɐݒ�J�n�ς݂Ȃ�G���[
	if (m_isLock) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�ύX�O�̐���t���O���擾
	eresult = _controlfp_s(
					&m_FPUCtrl,	//���݂̐��䃏�[�h
					0,			//���䃏�[�h�F�Ȃ�
					0			//�}�X�N�F�Ȃ�
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Windows API error.", eresult, GetLastError());
		goto EXIT;
	}

	_DisplayCurCtrl(_T("Start before"));

	//���������_���x��ݒ�
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
	//���x����̕K�v�Ȃ�
#else
	//x86(32bit)
	eresult = _controlfp_s(
					&curCtrl,	//���݂̐��䃏�[�h
					flag,		//���䃏�[�h�F������
					_MCW_PC		//�}�X�N�F���x����
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
// ���x�ݒ�I��
//******************************************************************************
int SMFPUCtrl::End()
{
	int result = 0;
	errno_t eresult = 0;
	unsigned int curCtrl = 0;

	//�ݒ�J�n���Ă��Ȃ��ꍇ�̓G���[
	if (!m_isLock) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�ݒ�J�n���ƈقȂ�X���b�h�Őݒ�I�����邱�Ƃ͂ł��Ȃ�
	if (m_ThreadID != GetCurrentThreadId()) {
		result = YN_SET_ERR("Program error.", m_ThreadID, GetCurrentThreadId());
		goto EXIT;
	}

	//���������_���x�𕜌�����
#ifdef _WIN64
	//x64(64bit)
	//���x����̕K�v�Ȃ�
#else
	//x86(32bit)
	eresult = _controlfp_s(
					&curCtrl,	//���݂̐��䃏�[�h
					m_FPUCtrl,	//���䃏�[�h�F�ݒ�J�n���_
					_MCW_PC		//�}�X�N�F���x����
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
// ���x�ݒ��Ԋm�F
//******************************************************************************
bool SMFPUCtrl::IsLocked()
{
	return m_isLock;
}

//******************************************************************************
// ���������_���䃏�[�h�\��
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
//			(fpuctrl & _MCW_DN), //DENORMAL����
//			(fpuctrl & _MCW_EM), //���荞�ݗ�O�}�X�N
//			(fpuctrl & _MCW_IC), //��������
//			(fpuctrl & _MCW_RC), //�ۂߐ���
//			(fpuctrl & _MCW_PC)  //���x����
//		);
//
//	MessageBox(NULL, msg, pTitle, MB_OK);
//
//EXIT:;
//	return;
}

} // end of namespace


