//******************************************************************************
//
// MIDITrail / MTAboutDlg
//
// �o�[�W�������_�C�A���O�N���X
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MIDITrailVersion.h"
#include "MTAboutDlg.h"

using namespace YNBaseLib;


//******************************************************************************
// �E�B���h�E�v���V�[�W������p�p�����[�^�ݒ�
//******************************************************************************
MTAboutDlg* MTAboutDlg::m_pThis = NULL;

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTAboutDlg::MTAboutDlg(void)
{
	m_pThis = this;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTAboutDlg::~MTAboutDlg(void)
{
}

//******************************************************************************
// �E�B���h�E�v���V�[�W��
//******************************************************************************
INT_PTR CALLBACK MTAboutDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// �E�B���h�E�v���V�[�W���F����
//******************************************************************************
INT_PTR MTAboutDlg::_WndProcImpl(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	BOOL bresult = FALSE;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG:
			result = _OnInitDlg(hDlg);
			if (result != 0) goto EXIT;
			bresult = TRUE;
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			break;
	}

EXIT:;
	return (INT_PTR)bresult;
}

//******************************************************************************
// �\��
//******************************************************************************
int MTAboutDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	//�A�v���P�[�V�����C���X�^���X�n���h�����擾
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�_�C�A���O�\��
	dresult = DialogBox(
					hInstance,						//�C���X�^���X�n���h��
					MAKEINTRESOURCE(IDD_ABOUTBOX),	//�_�C�A���O�{�b�N�X�e���v���[�g
					hParentWnd,						//�e�E�B���h�E�n���h��
					_WndProc						//�_�C�A���O�{�b�N�X�v���V�[�W��
				);
	if ((dresult == 0) || (dresult == -1)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �_�C�A���O�\�����O������
//******************************************************************************
int MTAboutDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR* pVersion = NULL;
	TCHAR* pCopyright = NULL;

	//�o�[�W����������
#ifdef _WIN64
	//64bit
	pVersion = MIDITRAIL_VERSION_STRING_X64;
#else
	//32bit
	pVersion = MIDITRAIL_VERSION_STRING_X86;
#endif

	//�R�s�[���C�g������
	pCopyright = MIDITRAIL_COPYRIGHT;

	//�o�[�W����������ݒ�
	bresult = SetWindowText(GetDlgItem(hDlg, IDC_TEXT_VERSION), pVersion);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�R�s�[���C�g������ݒ�
	bresult = SetWindowText(GetDlgItem(hDlg, IDC_TEXT_COPYRIGHT), pCopyright);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


