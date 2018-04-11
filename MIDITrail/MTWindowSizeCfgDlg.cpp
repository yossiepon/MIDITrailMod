//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// ウィンドウサイズ設定ダイアログクラス
//
// Copyright (C) 2010-2016 WADA Masashi. All Rights Reserved.
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
	m_hSizeList = NULL;
	m_hEditWidth = NULL;
	m_hEditHeight = NULL;
	m_hCheckApplyToView = NULL;
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
			//リストボックス
			else if (LOWORD(wParam) == IDC_WINDOWSIZE_LIST) {
				//選択状態変化
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
bool MTWindowSizeCfgDlg::IsChanged()
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

	//ウィンドウハンドル取得
	m_hSizeList = GetDlgItem(hDlg, IDC_WINDOWSIZE_LIST);
	m_hEditWidth = GetDlgItem(hDlg, IDC_EDIT_WIDTH);
	m_hEditHeight = GetDlgItem(hDlg, IDC_EDIT_HEIGHT);
	m_hCheckApplyToView = GetDlgItem(hDlg, IDC_CHECK_APPLY_TO_VIEW);

	//ウィンドウサイズ選択コンボボックス初期化
	result = _InitSizeList();
	if (result != 0) goto EXIT;

	//ウィンドウサイズエディットボックス初期化
	result = _InitSizeEditbox();
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

	//ウィンドウサイズ取得
	result = _GetConfWindowSize(&curWidth, &curHeight);
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

	//リスト一致するサイズが見つかった場合は選択状態にする
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
// ウィンドウサイズエディットボックス初期化
//******************************************************************************
int MTWindowSizeCfgDlg::_InitSizeEditbox()
{
	int result = 0;
	LRESULT lresult = 0;
	int width = 0;
	int height = 0;
	int maxsize = 0;
	int applyToViewArea = 0;

	//ウィンドウサイズ取得
	result = _GetConfWindowSize(&width, &height);
	if (result != 0) goto EXIT;

	//入力可能最大文字数を設定：5桁 99,999まで
	maxsize = sizeof(TCHAR) * MT_WINDOW_SIZE_CHAR_MAX;
	SendMessage(m_hEditWidth, EM_SETLIMITTEXT, (WPARAM)maxsize, 0);
	SendMessage(m_hEditHeight, EM_SETLIMITTEXT, (WPARAM)maxsize, 0);

	//エディットボックスにウィンドウサイズの数値文字列を設定
	result = _UpdateSizeEditBox(width, height);
	if (result != 0) goto EXIT;

	//ビュー領域適用フラグ取得
	result = m_ConfFile.GetInt(_T("ApplyToViewArea"), &applyToViewArea, 0);
	if (result != 0) goto EXIT;

	//ビュー領域適用チェックボックス初期化
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
// ウィンドウサイズ設定値取得
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

	//ウィンドウサイズ設定値取得
	result = m_ConfFile.GetInt(_T("Width"), &width, 0);
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetInt(_T("Height"), &height, 0);
	if (result != 0) goto EXIT;

	//サイズが異常な場合は初回起動時のウィンドウサイズに更新
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
// ウィンドウサイズ情報保存
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

	//幅
	apiresult = GetWindowText(m_hEditWidth, str, 32);
	if (apiresult == 0) {
		//テキスト無しまたはウィンドウハンドル無効の場合
		width = 0;
	}
	else {
		width = _tstoi(str);
	}

	//高さ
	apiresult = GetWindowText(m_hEditHeight, str, 32);
	if (apiresult == 0) {
		//テキスト無しまたはウィンドウハンドル無効の場合
		height = 0;
	}
	else {
		height = _tstoi(str);
	}

	//クリッピング
	if (width < MT_WINDOW_SIZE_MIN) {
		width = MT_WINDOW_SIZE_MIN;
	}
	if (height < MT_WINDOW_SIZE_MIN) {
		height = MT_WINDOW_SIZE_MIN;
	}

	//設定保存
	result = m_ConfFile.SetInt(_T("Width"), width);
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("Height"), height);
	if (result != 0) goto EXIT;

	//ビュー領域適用チェックボックス設定保存
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
// ウィンドウサイズリストボックス選択状態変化
//******************************************************************************
int MTWindowSizeCfgDlg::_OnSizeListChanged()
{
	int result = 0;
	LRESULT lresult = 0;
	int selectedIndex = 0;
	MTWindowSizeItem item;
	MTWindowSizeList::iterator itr;

	//選択項目のインデックスを取得
	lresult = SendMessage(m_hSizeList, LB_GETCURSEL, 0, 0);
	if (lresult == LB_ERR) {
		//未選択の場合は何もしない
		goto EXIT;
	}
	else if (lresult < 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hSizeList);
		goto EXIT;
	}

	//選択サイズ取得
	selectedIndex = (unsigned long)lresult;
	itr = m_SizeList.begin();
	advance(itr, selectedIndex);
	item = *itr;

	//エディットボックスにウィンドウサイズの数値文字列を設定
	result = _UpdateSizeEditBox(item.width, item.height);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ウィンドウサイズエディットボックス更新
//******************************************************************************
int MTWindowSizeCfgDlg::_UpdateSizeEditBox(
		int width,
		int height
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR str[32] = {_T('\0')};

	//幅
	_stprintf_s(str, 32, _T("%d"), width);
	bresult = SetWindowText(m_hEditWidth, str);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), width);
		goto EXIT;
	}

	//高さ
	_stprintf_s(str, 32, _T("%d"), height);
	bresult = SetWindowText(m_hEditHeight, str);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), height);
		goto EXIT;
	}

EXIT:;
	return result;
}

