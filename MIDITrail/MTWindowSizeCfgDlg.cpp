//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// ウィンドウサイズ設定ダイアログクラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTParam.h"
#include "MTWindowSizeCfgDlg.h"


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTWindowSizeCfgDlg* MTWindowSizeCfgDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTWindowSizeCfgDlg::MTWindowSizeCfgDlg(void)
{
	m_pThis = this;
	m_isSaved = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTWindowSizeCfgDlg::~MTWindowSizeCfgDlg(void)
{
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTWindowSizeCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
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
int MTWindowSizeCfgDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	m_isSaved = false;

	//アプリケーションインスタンスハンドルを取得
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//ダイアログ表示
	dresult = DialogBox(
					hInstance,							//インスタンスハンドル
					MAKEINTRESOURCE(IDD_WINDOWSIZE_CFG),//ダイアログボックステンプレート
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
// 変更確認
//******************************************************************************
bool MTWindowSizeCfgDlg::IsCahnged()
{
	//本当は値の変化を確認すべき
	return m_isSaved;
}

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTWindowSizeCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	//設定ファイル初期化
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//ウィンドウサイズ選択コンボボックス初期化
	m_hSizeList = GetDlgItem(hDlg, IDC_WINDOWSIZE_LIST);
	result = _InitSizeList();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル初期化
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
// ウィンドウサイズ選択リストボックス初期化
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

	//ユーザ選択ウィンドウサイズ取得
	result = m_ConfFile.GetInt(_T("Width"), &curWidth, 0);
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetInt(_T("Height"), &curHeight, 0);
	if (result != 0) goto EXIT;

	m_SizeList.clear();

	for (index = 0; ; index++) {

		//グラフィックスモード情報取得
		bresult = EnumDisplaySettings(
						NULL,		//取得対象ディスプレイデバイス
						index,		//グラフィックモードインデックス
						&devMode	//取得したグラフィックスモード
					);
		if (!bresult) {
			//モード一覧取得完了
			break;
		}

		//ビット深度32bit以外は無視
		if (devMode.dmBitsPerPel != 32) {
			continue;
		}

		//すでにリスト登録済みか確認する
		isExist = false;
		for (itr = m_SizeList.begin(); itr != m_SizeList.end(); itr++) {
			if ((itr->width == devMode.dmPelsWidth)
			 && (itr->height == devMode.dmPelsHeight)) {
				isExist = true;
				break;
			}
		}

		//リスト登録
		if (!isExist) {
			item.width = devMode.dmPelsWidth;
			item.height = devMode.dmPelsHeight;
			m_SizeList.push_back(item);
			
			//キャプション作成
			_stprintf_s(caption, 64, _T("%d x %d  32bit"), item.width, item.height);

			//ウィンドウサイズをリストボックスに追加
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

	//一覧に前回設定値が見つからなかった場合はリストの先頭を選択状態とする
	if (selectedIndex < 0) {
		selectedIndex = 0;
	}

	//選択状態設定
	lresult = SendMessage(m_hSizeList, LB_SETCURSEL, selectedIndex, 0);
	if (lresult == LB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// デバイス選択情報保存
//******************************************************************************
int MTWindowSizeCfgDlg::_Save()
{
	int result = 0;
	LRESULT lresult = 0;
	int selectedIndex = 0;
	MTWindowSizeItem item;
	MTWindowSizeList::iterator itr;

	//選択項目のインデックスを取得
	lresult = SendMessage(m_hSizeList, LB_GETCURSEL, 0, 0);
	if ((lresult == LB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hSizeList);
		goto EXIT;
	}
	selectedIndex = (unsigned long)lresult;

	itr = m_SizeList.begin();
	advance(itr, selectedIndex);
	item = *itr;

	//設定保存
	result = m_ConfFile.SetInt(_T("Width"), item.width);
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("Height"), item.height);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

