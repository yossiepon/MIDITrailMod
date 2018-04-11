//******************************************************************************
//
// MIDITrail / MTHowToViewDlg
//
// ������@�_�C�A���O
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTHowToViewDlg.h"

using namespace YNBaseLib;

//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�\���摜��
#define MT_HOWTOVIEW_IMAGE_NUM  (3)

//******************************************************************************
// �E�B���h�E�v���V�[�W������p�p�����[�^�ݒ�
//******************************************************************************
MTHowToViewDlg* MTHowToViewDlg::m_pThis = NULL;

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTHowToViewDlg::MTHowToViewDlg(void)
{
	m_pThis = this;
	m_hMemBmpPixel = NULL;
	m_pBmpPixcel = NULL;
	m_PageNo = 0;
	m_hWnd = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTHowToViewDlg::~MTHowToViewDlg(void)
{
	_Clear();
}

//******************************************************************************
// �E�B���h�E�v���V�[�W��
//******************************************************************************
INT_PTR CALLBACK MTHowToViewDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// �E�B���h�E�v���V�[�W���F����
//******************************************************************************
INT_PTR MTHowToViewDlg::_WndProcImpl(
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
			if (LOWORD(wParam) == IDOK) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_PREVIOUS) {
				result = _OnPreviousButton();
				if (result != 0) goto EXIT;
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_NEXT) {
				result = _OnNextButton();
				if (result != 0) goto EXIT;
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_CLOSE) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			break;
		case WM_PAINT:
			result = _DrawHowToBmp();
			if (result != 0) goto EXIT;
			bresult = TRUE;
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(hDlg);
	}
	return (INT_PTR)bresult;
}

//******************************************************************************
// �\��
//******************************************************************************
int MTHowToViewDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	m_PageNo = 0;

	//�A�v���P�[�V�����C���X�^���X�n���h�����擾
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//�_�C�A���O�\��
	dresult = DialogBox(
					hInstance,							//�C���X�^���X�n���h��
					MAKEINTRESOURCE(IDD_HOWTOVIEW),		//�_�C�A���O�{�b�N�X�e���v���[�g
					hParentWnd,							//�e�E�B���h�E�n���h��
					_WndProc							//�_�C�A���O�{�b�N�X�v���V�[�W��
				);
	if ((dresult == 0) || (dresult == -1)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hInstance);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �_�C�A���O�\�����O������
//******************************************************************************
int MTHowToViewDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_hWnd = hDlg;

	//HowTo�r�b�g�}�b�v�ǂݍ���
	result = _LoadHowToBmp();
	if (result != 0) goto EXIT;

	//�{�^����ԍX�V
	_UpdateButtonStatus();

EXIT:;
	return result;
}

//******************************************************************************
// HowTo�r�b�g�}�b�v�ǂݍ���
//******************************************************************************
int MTHowToViewDlg::_LoadHowToBmp()
{
	int result = 0;
	HANDLE hFile = NULL;
	BOOL bresult = FALSE;
	DWORD numOfBytesRead = 0;
	DWORD fp = 0;
	HANDLE hMemBmpPixel = NULL;
	BYTE* pBmpPixcel = NULL;
	TCHAR bmpFilePath[_MAX_PATH] = {_T('\0')};
	TCHAR* pBmpFileName[3] = { MT_IMGFILE_HOWTOVIEW1, MT_IMGFILE_HOWTOVIEW2, MT_IMGFILE_HOWTOVIEW3 };

	_Clear();

	//----------------------------------------------------------------
	//�t�@�C���I�[�v��
	//----------------------------------------------------------------
	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	result = YNPathUtil::GetModuleDirPath(bmpFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	if (m_PageNo >= MT_HOWTOVIEW_IMAGE_NUM) {
		result = YN_SET_ERR("Program error.", m_PageNo, 0);
		goto EXIT;
	}

	//BMP�t�@�C���p�X�쐬
	_tcscat_s(bmpFilePath, _MAX_PATH, pBmpFileName[m_PageNo]);

	//BMP�t�@�C�����J��
	hFile = CreateFile(
				bmpFilePath,			//�t�@�C���p�X
				GENERIC_READ,			//�A�N�Z�X�^�C�v
				0,						//���L���@
				NULL,					//�Z�L�����e�B����
				OPEN_EXISTING,			//�����w��
				FILE_ATTRIBUTE_NORMAL,	//�t�@�C�������ƃt���O
				NULL					//�e���v���[�g�t�@�C���n���h��
			);
	if (hFile == INVALID_HANDLE_VALUE) {
		result = YN_SET_ERR("File open error.", GetLastError(), 0);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMP�t�@�C���w�b�_
	//----------------------------------------------------------------
	//BMP�t�@�C���w�b�_�ǂݍ���
	bresult = ReadFile(
					hFile,							//�t�@�C���n���h��
					(LPBITMAPFILEHEADER)&m_BmpHead,	//�o�b�t�@�ʒu
					sizeof(BITMAPFILEHEADER),		//�o�b�t�@�T�C�Y
					&numOfBytesRead,				//�ǂݎ��T�C�Y
					NULL							//�I�[�o�[���b�v�\���̃o�b�t�@
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	//�t�@�C���^�C�v�̊m�F "BM"
	if (m_BmpHead.bfType != 0x4D42) {
		result = YN_SET_ERR("Invalid data found.", m_BmpHead.bfType, 0);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMP���w�b�_
	//----------------------------------------------------------------
	//BMP���w�b�_�ǂݍ���
	bresult = ReadFile(
					hFile,							//�t�@�C���n���h��
					(LPBITMAPINFOHEADER)&m_BmpInfo,	//�o�b�t�@�ʒu
					sizeof(BITMAPINFOHEADER),		//�o�b�t�@�T�C�Y
					&numOfBytesRead,				//�ǂݎ��T�C�Y
					NULL							//�I�[�o�[���b�v�\���̃o�b�t�@
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	//24bit�摜�ȊO�͓ǂ݂܂���
	if ((m_BmpInfo.biBitCount != 24) || (m_BmpInfo.biClrUsed != 0)) {
		result = YN_SET_ERR("Invalid BMP file.", m_BmpInfo.biBitCount, m_BmpInfo.biClrUsed);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMP�s�N�Z���f�[�^
	//----------------------------------------------------------------
	//�s�N�Z���f�[�^�J�n�ʒu�Ƀt�@�C���|�C���^��ݒ�
	fp = SetFilePointer(
				hFile,					//�t�@�C���n���h��
				m_BmpHead.bfOffBits,	//�t�@�C���|�C���^�ړ��o�C�g���F����32bit
				0,						//�t�@�C���|�C���^�ړ��o�C�g���F���32bit
				FILE_BEGIN				//�J�n�_
			);
	if (fp == INVALID_SET_FILE_POINTER) {
		result = YN_SET_ERR("File access error.", GetLastError(), m_BmpHead.bfOffBits);
		goto EXIT;
	}

	//�s�N�Z���f�[�^�ǂݍ��ݗp�������m��
	hMemBmpPixel = GlobalAlloc(GHND, m_BmpHead.bfSize);
	if (hMemBmpPixel == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	pBmpPixcel = (BYTE*)GlobalLock(hMemBmpPixel);
	if (pBmpPixcel == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//BMP�s�N�Z���f�[�^�ǂݍ���
	bresult = ReadFile(
					hFile,				//�t�@�C���n���h��
					pBmpPixcel,			//�o�b�t�@�ʒu
					m_BmpHead.bfSize,	//�o�b�t�@�T�C�Y
					&numOfBytesRead,	//�ǂݎ��T�C�Y
					NULL				//�I�[�o�[���b�v�\���̃o�b�t�@
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	m_hMemBmpPixel = hMemBmpPixel;
	m_pBmpPixcel = pBmpPixcel;

EXIT:;
	if (hFile != NULL) {
		CloseHandle(hFile);
	}
	if (result != 0) {
		if (hMemBmpPixel != NULL) {
			GlobalUnlock(hMemBmpPixel);
			GlobalFree(hMemBmpPixel);
		}
	}
	return result;
}

//******************************************************************************
// HowTo�r�b�g�}�b�v�`��
//******************************************************************************
int MTHowToViewDlg::_DrawHowToBmp()
{
	int result = 0;
	int apiresult = 0;
	HDC hdc = NULL;
	HWND hWndPicture = NULL;
	PAINTSTRUCT ps;

	if (m_pBmpPixcel == NULL) goto EXIT;

	//�`��ΏۃE�B���h�E�n���h���擾
	hWndPicture = GetDlgItem(m_hWnd, IDC_HOWTO_PICTURE);

	//�`�揀��
	hdc = BeginPaint(hWndPicture, &ps);

	apiresult = SetDIBitsToDevice(
					hdc,					//�f�o�C�X�R���e�L�X�g�n���h��
					0,						//�]���捶����̍��W�Fx
					0,						//�]���捶����̍��W�Fy
					m_BmpInfo.biWidth,		//�]�����T�C�Y�F��
					m_BmpInfo.biHeight,		//�]�����T�C�Y�F����
					0,						//�]�������W�������̍��W�Fx
					0,						//�]�������W�������̍��W�Fy
					0,						//�����J�n�s
					m_BmpInfo.biHeight,		//�����s��
					m_pBmpPixcel,			//�r�b�g�}�b�v�f�[�^�J�n�̃A�h���X
					(BITMAPINFO*)&m_BmpInfo,//BMP���w�b�_
					DIB_RGB_COLORS			//�F�w��
				);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�`��I��
	EndPaint(hWndPicture, &ps);

EXIT:;
	return result;
}

//******************************************************************************
// �N���A
//******************************************************************************
void MTHowToViewDlg::_Clear()
{
	ZeroMemory(&m_BmpHead, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&m_BmpInfo, sizeof(BITMAPINFO));

	if (m_hMemBmpPixel != NULL) {
		GlobalUnlock(m_hMemBmpPixel);
		GlobalFree(m_hMemBmpPixel);
		m_hMemBmpPixel = NULL;
	}
	m_pBmpPixcel = NULL;
}

//******************************************************************************
// �O�{�^������
//******************************************************************************
int MTHowToViewDlg::_OnPreviousButton()
{
	int result = 0;

	//�O�̉摜�ֈړ�
	m_PageNo--;

	//�O�̂��߃K�[�h����
	if (m_PageNo < 0) {
		m_PageNo = 0;
	}

	//�{�^����ԍX�V
	_UpdateButtonStatus();

	//�摜�\��
	result = _DrawHowToImage();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���{�^������
//******************************************************************************
int MTHowToViewDlg::_OnNextButton()
{
	int result = 0;

	//���̉摜�ֈړ�
	m_PageNo++;

	//�O�̂��߃K�[�h����
	if (m_PageNo >= MT_HOWTOVIEW_IMAGE_NUM) {
		m_PageNo = MT_HOWTOVIEW_IMAGE_NUM - 1;
	}

	//�{�^����ԍX�V
	_UpdateButtonStatus();

	//�摜�\��
	result = _DrawHowToImage();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �摜�\��
//******************************************************************************
int MTHowToViewDlg::_DrawHowToImage()
{
	int result = 0;

	//HowTo�r�b�g�}�b�v�ǂݍ���
	result = _LoadHowToBmp();
	if (result != 0) goto EXIT;

	//�ĕ`��
	InvalidateRect(
			m_hWnd,	//�E�B���h�E�n���h��
			NULL,	//�X�V���[�W�����w��F�S��
			FALSE	//�w�i�����F�Ȃ�
		);
	UpdateWindow(m_hWnd);

EXIT:;
	return result;
}

//******************************************************************************
// �{�^����ԍX�V
//******************************************************************************
void MTHowToViewDlg::_UpdateButtonStatus()
{
	HWND hPreviousButton = NULL;
	HWND hNextButton = NULL;

	hPreviousButton = GetDlgItem(m_hWnd, IDC_BTN_PREVIOUS);
	hNextButton = GetDlgItem(m_hWnd, IDC_BTN_NEXT);

	EnableWindow(hPreviousButton, TRUE);
	EnableWindow(hNextButton, TRUE);

	//�擪�摜�\���F�O�{�^���s����
	if (m_PageNo == 0) {
		EnableWindow(hPreviousButton, FALSE);
	}
	//�ŏI�摜�\���F���{�^���s����
	if (m_PageNo == (MT_HOWTOVIEW_IMAGE_NUM - 1)) {
		EnableWindow(hNextButton, FALSE);
	}
}


