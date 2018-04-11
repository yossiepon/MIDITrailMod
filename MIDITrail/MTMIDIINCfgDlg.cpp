//******************************************************************************
//
// MIDITrail / MTMIDIINCfgDlg
//
// MIDI IN �ݒ�_�C�A���O
//
// Copyright (C) 2012-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "MTParam.h"
#include "MTMIDIINCfgDlg.h"
#include <string>


//******************************************************************************
// �E�B���h�E�v���V�[�W������p�p�����[�^�ݒ�
//******************************************************************************
MTMIDIINCfgDlg* MTMIDIINCfgDlg::m_pThis = NULL;

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTMIDIINCfgDlg::MTMIDIINCfgDlg(void)
{
	m_pThis = this;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTMIDIINCfgDlg::~MTMIDIINCfgDlg(void)
{
	m_hComboDevA = NULL;
	m_hMIDITHRU = NULL;
}

//******************************************************************************
// �E�B���h�E�v���V�[�W��
//******************************************************************************
INT_PTR CALLBACK MTMIDIINCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// �E�B���h�E�v���V�[�W���F����
//******************************************************************************
INT_PTR MTMIDIINCfgDlg::_WndProcImpl(
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
				result = _Save();
				if (result != 0) goto EXIT;
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
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
int MTMIDIINCfgDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	//�A�v���P�[�V�����C���X�^���X�n���h�����擾
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//�_�C�A���O�\��
	dresult = DialogBox(
					hInstance,							//�C���X�^���X�n���h��
					MAKEINTRESOURCE(IDD_MIDIIN_CFG),	//�_�C�A���O�{�b�N�X�e���v���[�g
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
int MTMIDIINCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	//�ݒ�t�@�C��������
	result = _InitConfFile();

	//MIDI���̓f�o�C�X���䏉����
	result = m_MIDIInDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	//MIDI�o�̓f�o�C�X�I���R���{�{�b�N�X������
	m_hComboDevA = GetDlgItem(hDlg, IDC_COMBO_PORT_A);
	result = _InitComboDev(m_hComboDevA, _T("PortA"));
	if (result != 0) goto EXIT;

	//MIDITHRU�ݒ�`�F�b�N�{�b�N�X������
	m_hMIDITHRU = GetDlgItem(hDlg, IDC_CHECK_MIDITHRU);
	result = _InitCheckBtnMIDITHRU();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�t�@�C��������
//******************************************************************************
int MTMIDIINCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_MIDI);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

	result = m_ConfFile.SetCurSection(_T("MIDIIN"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X�I���R���{�{�b�N�X������
//******************************************************************************
int MTMIDIINCfgDlg::_InitComboDev(
		HWND hComboDev,
		TCHAR* pPortName
	)
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long index = 0;
	unsigned long devNum = 0;
	int comboIndex = 0;
	int selectedIndex = -1;
	TCHAR devName[MAXPNAMELEN];
	std::string selectedProductName;
	std::string productName;

	//���[�U�I���f�o�C�X���擾
	result = m_ConfFile.GetStr(pPortName, devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;
	selectedProductName = devName;

	//���[�U�I���f�o�C�X���Ȃ��ꍇ�́u�I���Ȃ��v��I����Ԃɂ���
	if (selectedProductName == _T("")) {
		selectedIndex = 0;
	}

	//�u�I���Ȃ��v���R���{�{�b�N�X�ɒǉ�
	lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)_T("(none)"));
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
		goto EXIT;
	}
	comboIndex++;

	//MIDI�f�o�C�X�̐�
	devNum = m_MIDIInDevCtrl.GetDevNum();

	for (index = 0; index < devNum; index++) {
		//MIDI IN�f�o�C�X���擾
		result = m_MIDIInDevCtrl.GetDevProductName(index, productName);
		if (result != 0) goto EXIT;

		//�f�o�C�X�����R���{�{�b�N�X�ɒǉ�
		lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)productName.c_str());
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
			goto EXIT;
		}
		if (selectedProductName == productName) {
			selectedIndex = comboIndex;
		}

		comboIndex++;
	}

	//USB�f�o�C�X���l������
	//���[�U�I���f�o�C�X�����ݐڑ�����Ă��Ȃ��ꍇ�̓R���{�{�b�N�X�̖����ɒǉ�����
	if (selectedIndex < 0) {
		//�f�o�C�X�����R���{�{�b�N�X�ɒǉ�
		lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)selectedProductName.c_str());
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
			goto EXIT;
		}
		selectedIndex = comboIndex;
		comboIndex++;
	}

	//�I����Ԑݒ�
	lresult = SendMessage(hComboDev, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDITHRU�`�F�b�N�{�^��������
//******************************************************************************
int MTMIDIINCfgDlg::_InitCheckBtnMIDITHRU()
{
	int result = 0;
	LRESULT lresult = 0;
	int checkMIDITHRU = 0;

	//MIDITHRU�ݒ�擾
	result = m_ConfFile.GetInt(_T("MIDITHRU"), &checkMIDITHRU, 1);
	if (result != 0) goto EXIT;

	//�`�F�b�N�{�b�N�X�ɔ��f
	if (checkMIDITHRU == 0) {
		SendMessage(m_hMIDITHRU, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	else {
		SendMessage(m_hMIDITHRU, BM_SETCHECK, BST_CHECKED, 0);
	}

EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X�I�����ۑ�
//******************************************************************************
int MTMIDIINCfgDlg::_Save()
{
	int result = 0;

	result = _SavePortCfg(m_hComboDevA, _T("PortA"));
	if (result != 0) goto EXIT;

	result = _SaveMIDITHRU();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g�ݒ�ۑ�
//******************************************************************************
int MTMIDIINCfgDlg::_SavePortCfg(
		HWND hComboDev,
		TCHAR* pPortName
	)
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long selectedIndex = 0;
	std::string selectedProductName;
	std::string productName;
	unsigned long devNum = 0;
	bool isUpdate = true;

	//MIDI�f�o�C�X�̐�
	devNum = m_MIDIInDevCtrl.GetDevNum();

	//�I�����ڂ̃C���f�b�N�X���擾
	lresult = SendMessage(hComboDev, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
		goto EXIT;
	}
	selectedIndex = (unsigned long)lresult;

	//�I�����ڂ̃f�o�C�X�����擾
	if (selectedIndex == 0) {
		selectedProductName = _T("");
	}
	else if (selectedIndex <= devNum) {
		result = m_MIDIInDevCtrl.GetDevProductName((selectedIndex-1), selectedProductName);
		if (result != 0) goto EXIT;
	}
	else {
		//�����ɒǉ��������ݐڑ�����Ă��Ȃ����[�U�I���f�o�C�X���I�����ꂽ�܂�
		isUpdate = false;
	}

	//�ݒ�ۑ�
	if (isUpdate) {
		result = m_ConfFile.SetStr(pPortName, selectedProductName.c_str());
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDITHRU �ݒ�ۑ�
//******************************************************************************
int MTMIDIINCfgDlg::_SaveMIDITHRU()
{
	int result = 0;
	LRESULT lresult = 0;
	int checkMIDITHRU = 0;

	lresult = SendMessage(m_hMIDITHRU, BM_GETCHECK, 0, 0);
	if (lresult == BST_CHECKED) {
		checkMIDITHRU = 1;
	}

	result = m_ConfFile.SetInt(_T("MIDITHRU"), checkMIDITHRU);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


