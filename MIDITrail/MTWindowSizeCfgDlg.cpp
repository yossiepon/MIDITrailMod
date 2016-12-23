//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// �E�B���h�E�T�C�Y�ݒ�_�C�A���O�N���X
//
// Copyright (C) 2010-2016 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTParam.h"
#include "MTWindowSizeCfgDlg.h"


//******************************************************************************
// �E�B���h�E�v���V�[�W������p�p�����[�^�ݒ�
//******************************************************************************
MTWindowSizeCfgDlg* MTWindowSizeCfgDlg::m_pThis = NULL;

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTWindowSizeCfgDlg::MTWindowSizeCfgDlg(void)
{
	m_pThis = this;
	m_hSizeList = NULL;
	m_hEditWidth = NULL;
	m_hEditHeight = NULL;
	m_hCheckApplyToView = NULL;
	m_isSaved = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTWindowSizeCfgDlg::~MTWindowSizeCfgDlg(void)
{
}

//******************************************************************************
// �E�B���h�E�v���V�[�W��
//******************************************************************************
INT_PTR CALLBACK MTWindowSizeCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// �E�B���h�E�v���V�[�W���F����
//******************************************************************************
INT_PTR MTWindowSizeCfgDlg::_WndProcImpl(
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
				m_isSaved = true;
				result = _Save();
				if (result != 0) goto EXIT;
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			//���X�g�{�b�N�X
			else if (LOWORD(wParam) == IDC_WINDOWSIZE_LIST) {
				//�I����ԕω�
				if  (HIWORD(wParam) == LBN_SELCHANGE){
					result = _OnSizeListChanged();
					if (result != 0) goto EXIT;
				}
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
int MTWindowSizeCfgDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	m_isSaved = false;

	//�A�v���P�[�V�����C���X�^���X�n���h�����擾
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//�_�C�A���O�\��
	dresult = DialogBox(
					hInstance,							//�C���X�^���X�n���h��
					MAKEINTRESOURCE(IDD_WINDOWSIZE_CFG),//�_�C�A���O�{�b�N�X�e���v���[�g
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
// �ύX�m�F
//******************************************************************************
bool MTWindowSizeCfgDlg::IsChanged()
{
	//�{���͒l�̕ω����m�F���ׂ�
	return m_isSaved;
}

//******************************************************************************
// �_�C�A���O�\�����O������
//******************************************************************************
int MTWindowSizeCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	//�ݒ�t�@�C��������
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//�E�B���h�E�n���h���擾
	m_hSizeList = GetDlgItem(hDlg, IDC_WINDOWSIZE_LIST);
	m_hEditWidth = GetDlgItem(hDlg, IDC_EDIT_WIDTH);
	m_hEditHeight = GetDlgItem(hDlg, IDC_EDIT_HEIGHT);
	m_hCheckApplyToView = GetDlgItem(hDlg, IDC_CHECK_APPLY_TO_VIEW);

	//�E�B���h�E�T�C�Y�I���R���{�{�b�N�X������
	result = _InitSizeList();
	if (result != 0) goto EXIT;

	//�E�B���h�E�T�C�Y�G�f�B�b�g�{�b�N�X������
	result = _InitSizeEditbox();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�t�@�C��������
//******************************************************************************
int MTWindowSizeCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_VIEW);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

	result = m_ConfFile.SetCurSection(_T("WindowSize"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y�I�����X�g�{�b�N�X������
//******************************************************************************
int MTWindowSizeCfgDlg::_InitSizeList()
{
	int result = 0;
	LRESULT lresult = 0;
	BOOL bresult = FALSE;
	unsigned long index = 0;
	int selectedIndex = -1;
	int curWidth = 0;
	int curHeight = 0;
	DEVMODE devMode;
	TCHAR caption[64];
	MTWindowSizeItem item;
	MTWindowSizeList::iterator itr;
	bool isExist = false;

	//�E�B���h�E�T�C�Y�擾
	result = _GetConfWindowSize(&curWidth, &curHeight);
	if (result != 0) goto EXIT;

	m_SizeList.clear();

	for (index = 0; ; index++) {

		//�O���t�B�b�N�X���[�h���擾
		bresult = EnumDisplaySettings(
						NULL,		//�擾�Ώۃf�B�X�v���C�f�o�C�X
						index,		//�O���t�B�b�N���[�h�C���f�b�N�X
						&devMode	//�擾�����O���t�B�b�N�X���[�h
					);
		if (!bresult) {
			//���[�h�ꗗ�擾����
			break;
		}

		//�r�b�g�[�x32bit�ȊO�͖���
		if (devMode.dmBitsPerPel != 32) {
			continue;
		}

		//���łɃ��X�g�o�^�ς݂��m�F����
		isExist = false;
		for (itr = m_SizeList.begin(); itr != m_SizeList.end(); itr++) {
			if ((itr->width == devMode.dmPelsWidth)
			 && (itr->height == devMode.dmPelsHeight)) {
				isExist = true;
				break;
			}
		}

		//���X�g�o�^
		if (!isExist) {
			item.width = devMode.dmPelsWidth;
			item.height = devMode.dmPelsHeight;
			m_SizeList.push_back(item);
			
			//�L���v�V�����쐬
			_stprintf_s(caption, 64, _T("%d x %d  32bit"), item.width, item.height);

			//�E�B���h�E�T�C�Y�����X�g�{�b�N�X�ɒǉ�
			lresult = SendMessage(m_hSizeList, LB_ADDSTRING, 0, (LPARAM)caption);
			if ((lresult == LB_ERR) || (lresult == LB_ERRSPACE)) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hSizeList);
				goto EXIT;
			}

			if ((item.width == curWidth) && (item.height == curHeight)) {
				selectedIndex = (int)(m_SizeList.size() - 1);
			}
		}
	}

	//���X�g��v����T�C�Y�����������ꍇ�͑I����Ԃɂ���
	if (selectedIndex >= 0) {
		lresult = SendMessage(m_hSizeList, LB_SETCURSEL, selectedIndex, 0);
		if (lresult == LB_ERR) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y�G�f�B�b�g�{�b�N�X������
//******************************************************************************
int MTWindowSizeCfgDlg::_InitSizeEditbox()
{
	int result = 0;
	LRESULT lresult = 0;
	int width = 0;
	int height = 0;
	int maxsize = 0;
	int applyToViewArea = 0;

	//�E�B���h�E�T�C�Y�擾
	result = _GetConfWindowSize(&width, &height);
	if (result != 0) goto EXIT;

	//���͉\�ő啶������ݒ�F5�� 99,999�܂�
	maxsize = sizeof(TCHAR) * MT_WINDOW_SIZE_CHAR_MAX;
	SendMessage(m_hEditWidth, EM_SETLIMITTEXT, (WPARAM)maxsize, 0);
	SendMessage(m_hEditHeight, EM_SETLIMITTEXT, (WPARAM)maxsize, 0);

	//�G�f�B�b�g�{�b�N�X�ɃE�B���h�E�T�C�Y�̐��l�������ݒ�
	result = _UpdateSizeEditBox(width, height);
	if (result != 0) goto EXIT;

	//�r���[�̈�K�p�t���O�擾
	result = m_ConfFile.GetInt(_T("ApplyToViewArea"), &applyToViewArea, 0);
	if (result != 0) goto EXIT;

	//�r���[�̈�K�p�`�F�b�N�{�b�N�X������
	if (applyToViewArea == 0) {
		SendMessage(m_hCheckApplyToView, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	else {
		SendMessage(m_hCheckApplyToView, BM_SETCHECK, BST_CHECKED, 0);
	}

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y�ݒ�l�擾
//******************************************************************************
int MTWindowSizeCfgDlg::_GetConfWindowSize(
		int* pWidth,
		int* pHeight
	)
{
	int result = 0;
	int width = 0;
	int height = 0;

	if ((pHeight == NULL) || (pHeight == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�E�B���h�E�T�C�Y�ݒ�l�擾
	result = m_ConfFile.GetInt(_T("Width"), &width, 0);
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetInt(_T("Height"), &height, 0);
	if (result != 0) goto EXIT;

	//�T�C�Y���ُ�ȏꍇ�͏���N�����̃E�B���h�E�T�C�Y�ɍX�V
	if ((width <= 0)
	  || (height <= 0)
	  || (width > MT_WINDOW_SIZE_MAX)
	  || (height > MT_WINDOW_SIZE_MAX)) {
		width = 800;
		height = 600;
	}

	*pWidth = width;
	*pHeight = height;

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y���ۑ�
//******************************************************************************
int MTWindowSizeCfgDlg::_Save()
{
	int result = 0;
	int apiresult = 0;
	LRESULT lresult = 0;
	int width = 0;
	int height = 0;
	int applyToViewArea = 0;
	TCHAR str[32] = {_T('\0')};

	//��
	apiresult = GetWindowText(m_hEditWidth, str, 32);
	if (apiresult == 0) {
		//�e�L�X�g�����܂��̓E�B���h�E�n���h�������̏ꍇ
		width = 0;
	}
	else {
		width = _tstoi(str);
	}

	//����
	apiresult = GetWindowText(m_hEditHeight, str, 32);
	if (apiresult == 0) {
		//�e�L�X�g�����܂��̓E�B���h�E�n���h�������̏ꍇ
		height = 0;
	}
	else {
		height = _tstoi(str);
	}

	//�N���b�s���O
	if (width < MT_WINDOW_SIZE_MIN) {
		width = MT_WINDOW_SIZE_MIN;
	}
	if (height < MT_WINDOW_SIZE_MIN) {
		height = MT_WINDOW_SIZE_MIN;
	}

	//�ݒ�ۑ�
	result = m_ConfFile.SetInt(_T("Width"), width);
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("Height"), height);
	if (result != 0) goto EXIT;

	//�r���[�̈�K�p�`�F�b�N�{�b�N�X�ݒ�ۑ�
	lresult = SendMessage(m_hCheckApplyToView, BM_GETCHECK, 0, 0);
	if (lresult == BST_CHECKED) {
		applyToViewArea = 1;
	}
	result = m_ConfFile.SetInt(_T("ApplyToViewArea"), applyToViewArea);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y���X�g�{�b�N�X�I����ԕω�
//******************************************************************************
int MTWindowSizeCfgDlg::_OnSizeListChanged()
{
	int result = 0;
	LRESULT lresult = 0;
	int selectedIndex = 0;
	MTWindowSizeItem item;
	MTWindowSizeList::iterator itr;

	//�I�����ڂ̃C���f�b�N�X���擾
	lresult = SendMessage(m_hSizeList, LB_GETCURSEL, 0, 0);
	if (lresult == LB_ERR) {
		//���I���̏ꍇ�͉������Ȃ�
		goto EXIT;
	}
	else if (lresult < 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hSizeList);
		goto EXIT;
	}

	//�I���T�C�Y�擾
	selectedIndex = (unsigned long)lresult;
	itr = m_SizeList.begin();
	advance(itr, selectedIndex);
	item = *itr;

	//�G�f�B�b�g�{�b�N�X�ɃE�B���h�E�T�C�Y�̐��l�������ݒ�
	result = _UpdateSizeEditBox(item.width, item.height);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y�G�f�B�b�g�{�b�N�X�X�V
//******************************************************************************
int MTWindowSizeCfgDlg::_UpdateSizeEditBox(
		int width,
		int height
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR str[32] = {_T('\0')};

	//��
	_stprintf_s(str, 32, _T("%d"), width);
	bresult = SetWindowText(m_hEditWidth, str);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), width);
		goto EXIT;
	}

	//����
	_stprintf_s(str, 32, _T("%d"), height);
	bresult = SetWindowText(m_hEditHeight, str);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), height);
		goto EXIT;
	}

EXIT:;
	return result;
}

