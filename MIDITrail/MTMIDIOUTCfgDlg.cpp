//******************************************************************************
//
// MIDITrail / MTMIDIOUTCfgDlg
//
// MIDI OUT 設定ダイアログクラス
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
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTMIDIOUTCfgDlg* MTMIDIOUTCfgDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTMIDIOUTCfgDlg::MTMIDIOUTCfgDlg(void)
{
	m_pThis = this;
}

//******************************************************************************
// デストラクタ
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
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTMIDIOUTCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
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
// 表示
//******************************************************************************
int MTMIDIOUTCfgDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	//アプリケーションインスタンスハンドルを取得
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//ダイアログ表示
	dresult = DialogBox(
					hInstance,							//インスタンスハンドル
					MAKEINTRESOURCE(IDD_MIDIOUT_CFG),	//ダイアログボックステンプレート
					hParentWnd,							//親ウィンドウハンドル
					_WndProc							//ダイアログボックスプロシージャ
				);
	if ((dresult == 0) || (dresult == -1)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hInstance);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTMIDIOUTCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	//設定ファイル初期化
	result = _InitConfFile();

	//MIDI出力デバイス制御初期化
	result = m_MIDIOutDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	//MIDI出力デバイス選択コンボボックス初期化
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
// 設定ファイル初期化
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
// デバイス選択コンボボックス初期化
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

	//ユーザ選択デバイス名取得
	result = m_ConfFile.GetStr(pPortName, devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;
	selectedProductName = devName;

	//ユーザ選択デバイスがない場合は「選択なし」を選択状態にする
	if (selectedProductName == _T("")) {
		selectedIndex = 0;
	}

	//「選択なし」をコンボボックスに追加
	lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)_T("(none)"));
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
		goto EXIT;
	}
	comboIndex++;

	//MIDIデバイスの数
	devNum = m_MIDIOutDevCtrl.GetDevNum();

	for (index = 0; index < devNum; index++) {
		//MIDI OUTデバイス名取得
		result = m_MIDIOutDevCtrl.GetDevProductName(index, productName);
		if (result != 0) goto EXIT;

		//デバイス名をコンボボックスに追加
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

	//USBデバイスを考慮する
	//ユーザ選択デバイスが現在接続されていない場合はコンボボックスの末尾に追加する
	if (selectedIndex < 0) {
		//デバイス名をコンボボックスに追加
		lresult = SendMessage(hComboDev, CB_ADDSTRING, 0, (LPARAM)selectedProductName.c_str());
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
			goto EXIT;
		}
		selectedIndex = comboIndex;
		comboIndex++;
	}

	//選択状態設定
	lresult = SendMessage(hComboDev, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// デバイス選択情報保存
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
// ポート設定保存
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

	//MIDIデバイスの数
	devNum = m_MIDIOutDevCtrl.GetDevNum();

	//選択項目のインデックスを取得
	lresult = SendMessage(hComboDev, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hComboDev);
		goto EXIT;
	}
	selectedIndex = (unsigned long)lresult;

	//選択項目のデバイス名を取得
	if (selectedIndex == 0) {
		selectedProductName = _T("");
	}
	else if (selectedIndex <= devNum) {
		result = m_MIDIOutDevCtrl.GetDevProductName((selectedIndex-1), selectedProductName);
		if (result != 0) goto EXIT;
	}
	else {
		//末尾に追加した現在接続されていないユーザ選択デバイスが選択されたまま
		isUpdate = false;
	}

	//設定保存
	if (isUpdate) {
		result = m_ConfFile.SetStr(pPortName, selectedProductName.c_str());
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

