//******************************************************************************
//
// MIDITrail / MTGraphicCfgDlg
//
// グラフィック設定ダイアログクラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "MTParam.h"
#include "MTGraphicCfgDlg.h"


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTGraphicCfgDlg* MTGraphicCfgDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTGraphicCfgDlg::MTGraphicCfgDlg(void)
{
	unsigned long type = 0;

	m_pThis = this;
	m_MultiSampleType = 0;
	m_isCahnged = false;

	for (type = 0; type < DX_MULTI_SAMPLE_TYPE_MAX; type++) {
		m_MultSampleTypeSupport[type] = false;
	}

	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTGraphicCfgDlg::~MTGraphicCfgDlg(void)
{
	m_hComboMultiSampleType = NULL;
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTGraphicCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTGraphicCfgDlg::_WndProcImpl(
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
// アンチエイリアシングサポート情報設定
//******************************************************************************
void MTGraphicCfgDlg::SetAntialiasSupport(
		unsigned long multiSampleType,	//2-16
		bool isSupport
	)
{
	if ((DX_MULTI_SAMPLE_TYPE_MIN <= multiSampleType)
	 && (multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX)) {
		m_MultSampleTypeSupport[multiSampleType] = isSupport;
	}
	return;
}

//******************************************************************************
// 表示
//******************************************************************************
int MTGraphicCfgDlg::Show(
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
					MAKEINTRESOURCE(IDD_GRAPHIC_CFG),	//ダイアログボックステンプレート
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
int MTGraphicCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_isCahnged = false;

	//設定ファイル初期化
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//設定ファイル読み込み
	result = _LoadConf();
	if (result != 0) goto EXIT;

	//マルチサンプル種別選択コンボボックス初期化
	m_hComboMultiSampleType = GetDlgItem(hDlg, IDC_COMBO_MULTISAMPLETYPE);
	result = _InitComboMultiSampleType(m_hComboMultiSampleType, m_MultiSampleType);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル初期化
//******************************************************************************
int MTGraphicCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTGraphicCfgDlg::_LoadConf()
{
	int result = 0;
	int multiSampleType = 0;

	result = m_ConfFile.SetCurSection(_T("Anti-aliasing"));
	if (result != 0) goto EXIT;

	result = m_ConfFile.GetInt(
					_T("MultiSampleType"),
					&multiSampleType,
					MT_GRAPHIC_MULTI_SAMPLE_TYPE_DEF
				);
	if (result != 0) goto EXIT;

	//無効値はアンチエイリアスOFFにする
	if ((DX_MULTI_SAMPLE_TYPE_MIN <= multiSampleType)
	 && (multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX)) {
		m_MultiSampleType = multiSampleType;
	}
	else {
		m_MultiSampleType = 0;
	}

EXIT:;
	return result;
}

//******************************************************************************
// デバイス選択コンボボックス初期化
//******************************************************************************
int MTGraphicCfgDlg::_InitComboMultiSampleType(
		HWND hCombo,
		unsigned long selMultiSampleType
	)
{
	int result = 0;
	LRESULT lresult = 0;
	int comboIndex = 0;
	int selectedIndex = -1;
	unsigned long type = 0;
	bool isSupportAA = false;
	TCHAR itemStr[256];

	//アンチエイリアシングサポート確認
	for (type = DX_MULTI_SAMPLE_TYPE_MIN; type <= DX_MULTI_SAMPLE_TYPE_MAX; type++) {
		if (m_MultSampleTypeSupport[type]) {
			isSupportAA = true;
		}
	}

	//先頭項目を登録
	if (isSupportAA) {
		_stprintf_s(itemStr, 256, _T("OFF"));
	}
	else {
		_stprintf_s(itemStr, 256, _T("Not supported"));
	}
	lresult = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)itemStr);
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(hCombo, CB_SETITEMDATA, comboIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}
	selectedIndex = comboIndex;
	comboIndex++;

	//マルチサンプル種別を追加登録
	for (type = DX_MULTI_SAMPLE_TYPE_MIN; type <= DX_MULTI_SAMPLE_TYPE_MAX; type++) {
		if (m_MultSampleTypeSupport[type]) {
			//マルチサンプリング種別をコンボボックスに追加
			_stprintf_s(itemStr, 256, _T("%dx"), type);
			lresult = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)itemStr);
			if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
				goto EXIT;
			}
			lresult = SendMessage(hCombo, CB_SETITEMDATA, comboIndex, type);
			if (lresult == CB_ERR) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
				goto EXIT;
			}
			if (type == selMultiSampleType) {
				selectedIndex = comboIndex;
			}
			comboIndex++;
		}
	}

	//選択状態設定
	lresult = SendMessage(hCombo, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

	//アンチエイリアシングをサポートしていなければ不活性にする
	if (!isSupportAA) {
		EnableWindow(hCombo, FALSE);
	}

EXIT:;
	return result;
}

//******************************************************************************
// デバイス選択情報保存
//******************************************************************************
int MTGraphicCfgDlg::_Save()
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long selectedIndex = 0;
	unsigned long selectedMultiSampleType = 0;

	//選択項目のインデックスを取得
	lresult = SendMessage(m_hComboMultiSampleType, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	selectedIndex = (unsigned long)lresult;

	//選択項目のユーザデータを取得：マルチサンプル種別
	lresult = SendMessage(m_hComboMultiSampleType, CB_GETITEMDATA, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}
	selectedMultiSampleType = (unsigned long)lresult;

	//設定保存
	result = m_ConfFile.SetCurSection(_T("Anti-aliasing"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("MultiSampleType"), selectedMultiSampleType);
	if (result != 0) goto EXIT;

	if (m_MultiSampleType != selectedMultiSampleType) {
		m_isCahnged = true;
	}
	m_MultiSampleType = selectedMultiSampleType;

EXIT:;
	return result;
}

//******************************************************************************
// パラメータ変更確認
//******************************************************************************
bool MTGraphicCfgDlg::IsCahnged()
{
	return m_isCahnged;
}


