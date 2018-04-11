//******************************************************************************
//
// MIDITrail / MTMIDIOUTCfgDlg
//
// MIDI OUT �ݒ�_�C�A���O�N���X
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "MTParam.h"
#include "MTMIDIOUTCfgDlg.h"
#include <string>


//******************************************************************************
// �E�B���h�E�v���V�[�W������p�p�����[�^�ݒ�
//******************************************************************************
MTMIDIOUTCfgDlg* MTMIDIOUTCfgDlg::m_pThis = NULL;

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTMIDIOUTCfgDlg::MTMIDIOUTCfgDlg(void)
{
	m_pThis = this;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTMIDIOUTCfgDlg::~MTMIDIOUTCfgDlg(void)
{
	m_hComboDevA = NULL;
	m_hComboDevB = NULL;
	m_hComboDevC = NULL;
	m_hComboDevD = NULL;
	m_hComboDevE = NULL;
	m_hComboDevF = NULL;
}

//******************************************************************************
// �E�B���h�E�v���V�[�W��
//******************************************************************************
INT_PTR CALLBACK MTMIDIOUTCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// �E�B���h�E�v���V�[�W���F����
//******************************************************************************
INT_PTR MTMIDIOUTCfgDlg::_WndProcImpl(
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
int MTMIDIOUTCfgDlg::Show(
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
					MAKEINTRESOURCE(IDD_MIDIOUT_CFG),	//�_�C�A���O�{�b�N�X�e���v���[�g
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
int MTMIDIOUTCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	//�ݒ�t�@�C��������
	result = _InitConfFile();

	//MIDI�o�̓f�o�C�X���䏉����
	result = m_MIDIOutDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	//MIDI�o�̓f�o�C�X�I���R���{�{�b�N�X������
	m_hComboDevA = GetDlgItem(hDlg, IDC_COMBO_PORT_A);
	m_hComboDevB = GetDlgItem(hDlg, IDC_COMBO_PORT_B);
	m_hComboDevC = GetDlgItem(hDlg, IDC_COMBO_PORT_C);
	m_hComboDevD = GetDlgItem(hDlg, IDC_COMBO_PORT_D);
	m_hComboDevE = GetDlgItem(hDlg, IDC_COMBO_PORT_E);
	m_hComboDevF = GetDlgItem(hDlg, IDC_COMBO_PORT_F);

	result = _InitComboDev(m_hComboDevA, _T("PortA"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevB, _T("PortB"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevC, _T("PortC"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevD, _T("PortD"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevE, _T("PortE"));
	if (result != 0) goto EXIT;
	result = _InitComboDev(m_hComboDevF, _T("PortF"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�t�@�C��������
//******************************************************************************
int MTMIDIOUTCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_MIDI);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

	result = m_ConfFile.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X�I���R���{�{�b�N�X������
//******************************************************************************
int MTMIDIOUTCfgDlg::_InitComboDev(
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
	devNum = m_MIDIOutDevCtrl.GetDevNum();

	for (index = 0; index < devNum; index++) {
		//MIDI OUT�f�o�C�X���擾
		result = m_MIDIOutDevCtrl.GetDevProductName(index, productName);
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
// �f�o�C�X�I�����ۑ�
//******************************************************************************
int MTMIDIOUTCfgDlg::_Save()
{
	int result = 0;

	result = _SavePortCfg(m_hComboDevA, _T("PortA"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevB, _T("PortB"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevC, _T("PortC"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevD, _T("PortD"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevE, _T("PortE"));
	if (result != 0) goto EXIT;
	result = _SavePortCfg(m_hComboDevF, _T("PortF"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g�ݒ�ۑ�
//******************************************************************************
int MTMIDIOUTCfgDlg::_SavePortCfg(
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
	devNum = m_MIDIOutDevCtrl.GetDevNum();

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
		result = m_MIDIOutDevCtrl.GetDevProductName((selectedIndex-1), selectedProductName);
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

