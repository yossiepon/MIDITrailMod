//******************************************************************************
//
// MIDITrail / MTWavetableSynthCfgDlg
//
// ウェーブテーブルシンセサイザ設定ダイアログクラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "Commdlg.h"
#include "MTParam.h"
#include "MTDlgLib.h"
#include "MTWavetableSynthCfgDlg.h"
#include "SMIDILib.h"
#include <mbctype.h>
#include <shlwapi.h>
#include <shellapi.h>


using namespace SMIDILib;

//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTWavetableSynthCfgDlg* MTWavetableSynthCfgDlg::m_pThis = NULL;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTWavetableSynthCfgDlg::MTWavetableSynthCfgDlg(void)
{
	m_pThis = this;
	m_hWnd = NULL;
	m_SelectedWavetableFileIndex = 0;
	m_UserWavetableFilePath[0] = L'\0';
	m_hComboWavetableFile = NULL;
	m_hComboMaxVoices = NULL;
	m_hComboSustain = NULL;
	m_isChanged = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTWavetableSynthCfgDlg::~MTWavetableSynthCfgDlg(void)
{
}

//******************************************************************************
// 表示：ダイアログが閉じられるまで制御を返さない
//******************************************************************************
int MTWavetableSynthCfgDlg::Show(HWND hParentWnd)
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
	//  ファイル名でUNICODE文字を表示可能とするため
	//  ワイド文字列版のAPIを用いて表示する
	dresult = DialogBoxW(
					hInstance,							//インスタンスハンドル
					MAKEINTRESOURCEW(IDD_WAVETABLE_SYNTH_CFG),	//ダイアログボックステンプレート
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
// パラメータ変更確認
//******************************************************************************
bool MTWavetableSynthCfgDlg::IsChanged()
{
	return m_isChanged;
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTWavetableSynthCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTWavetableSynthCfgDlg::_WndProcImpl(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	LRESULT lresult = 0;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG:
			result = _OnInitDlg(hDlg);
			if (result != 0) goto EXIT;
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				result = _Save();
				if (result != 0) goto EXIT;
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDC_BTN_SELECT_FILE) {
				result = _OnBtnSelectFile();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_CLEAR_FILE) {
				result = _OnBtnClearFile();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_ACKNOWLEDGEMENTS) {
				result = _OnBtnAcknowledgements();
				if (result != 0) goto EXIT;
			}
			break;
		default:
			//処理しないメッセージ
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(hDlg);
	}
	return lresult;
}

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTWavetableSynthCfgDlg::_OnInitDlg(HWND hDlg)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isChanged = false;

	//設定ファイル初期化
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//コンボボックス初期化：ウェーブテーブルファイル
	m_hComboWavetableFile = GetDlgItem(hDlg, IDC_COMBO_WAVETABLE_FILE);
	result = _InitComboWavetableFile();
	if (result != 0) goto EXIT;

	//コンボボックス初期化：Max Voices
	m_hComboMaxVoices = GetDlgItem(hDlg, IDC_COMBO_MAX_VOICES);
	result = _InitComboMaxVoices();
	if (result != 0) goto EXIT;

	//コンボボックス初期化：Sustan
	m_hComboSustain = GetDlgItem(hDlg, IDC_COMBO_SUSTAIN);
	result = _InitComboSustain();
	if (result != 0) goto EXIT;

	//親ウィンドウの中央に表示（スクリーン内側）
	MTDlgLib::SetWindowPositionToCenter(hDlg, true);

EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル初期化
//******************************************************************************
int MTWavetableSynthCfgDlg::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_SYNTHESIZER);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

	result = m_ConfFile.SetCurSection(_T("Wavetable"));
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// コンボボックス初期化：ウェーブテーブルファイル
//******************************************************************************
int MTWavetableSynthCfgDlg::_InitComboWavetableFile()
{
	int result = 0;
	int wavetableIndex = 0;

	//ウェーブテーブルファイルインデックス
	result = m_ConfFile.GetInt(_T("WavetableIndex"), &wavetableIndex, 0);
	if (result != 0) goto EXIT;

	if (wavetableIndex == 0) {
		m_SelectedWavetableFileIndex = 0;
	}
	else {
		m_SelectedWavetableFileIndex = 1;
	}

	//ユーザ選択ウェーブテーブルファイルパス
	result = m_ConfFile.GetWStr(_T("WavetableFilePath"), m_UserWavetableFilePath, _MAX_PATH, L"");
	if (result != 0) goto EXIT;

	if (wcslen (m_UserWavetableFilePath) == 0) {
		m_SelectedWavetableFileIndex = 0;
	}

	//ウェーブテーブルファイルリスト更新
	result = _UpdateWavetableFileList();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// コンボボックス初期化：Max Voices
//******************************************************************************
int MTWavetableSynthCfgDlg::_InitComboMaxVoices()
{
	int result = 0;
	LRESULT lresult = 0;
	int comboIndex = 0;
	int selectedIndex = 0;
	int maxVoices = 0;
	SM_WAVETABLE_SYNTH_PARAM defaultParam;

	//メニュー追加：256
	comboIndex = 0;
	lresult = SendMessage(m_hComboMaxVoices, CB_ADDSTRING, 0, (LPARAM)"256");
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(m_hComboMaxVoices, CB_SETITEMDATA, comboIndex, 256);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}
	//メニュー追加：512
	comboIndex = 1;
	lresult = SendMessage(m_hComboMaxVoices, CB_ADDSTRING, 0, (LPARAM)"512");
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(m_hComboMaxVoices, CB_SETITEMDATA, comboIndex, 512);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}
	//メニュー追加：1024
	comboIndex = 2;
	lresult = SendMessage(m_hComboMaxVoices, CB_ADDSTRING, 0, (LPARAM)"1024");
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(m_hComboMaxVoices, CB_SETITEMDATA, comboIndex, 1024);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}

	//既存の設定値を取得して選択状態に反映
	defaultParam = SMWavetableSynthCtrl::GetDefaultParam();
	result = m_ConfFile.GetInt(
					_T("MaxVoices"),
					&maxVoices,
					defaultParam.maxVoices
				);
	if (result != 0) goto EXIT;

	if (maxVoices == 256) {
		selectedIndex = 0;
	}
	else if (maxVoices == 512) {
		selectedIndex = 1;
	}
	else if (maxVoices == 1024) {
		selectedIndex = 2;
	}
	else {
		selectedIndex = 1;
	}

	lresult = SendMessage(m_hComboMaxVoices, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// コンボボックス初期化：Sustain
//******************************************************************************
int MTWavetableSynthCfgDlg::_InitComboSustain()
{
	int result = 0;
	LRESULT lresult = 0;
	int comboIndex = 0;
	int selectedIndex = 0;
	int sustain = 0;
	SM_WAVETABLE_SYNTH_PARAM defaultParam;

	//メニュー追加：Enable
	comboIndex = 0;
	lresult = SendMessage(m_hComboSustain, CB_ADDSTRING, 0, (LPARAM)"Enable");
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(m_hComboSustain, CB_SETITEMDATA, comboIndex, 1);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}
	//メニュー追加：Disable
	comboIndex = 1;
	lresult = SendMessage(m_hComboSustain, CB_ADDSTRING, 0, (LPARAM)"Disable");
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(m_hComboSustain, CB_SETITEMDATA, comboIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}

	//既存の設定値を取得して選択状態に反映
	defaultParam = SMWavetableSynthCtrl::GetDefaultParam();
	result = m_ConfFile.GetInt(
					_T("Sustain"),
					&sustain,
					defaultParam.sustain
				);
	if (result != 0) goto EXIT;

	if (sustain == 1) {
		selectedIndex = 0;
	}
	else {
		selectedIndex = 1;
	}

	lresult = SendMessage(m_hComboSustain, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ウェーブテーブルファイルリスト更新
//******************************************************************************
int MTWavetableSynthCfgDlg::_UpdateWavetableFileList()
{
	int result = 0;
	LRESULT lresult = 0;
	int comboIndex = 0;
	WCHAR itemStr[256];
	const WCHAR* pFileName = NULL;

	//リストをクリア
	lresult = SendMessage(m_hComboWavetableFile, CB_RESETCONTENT, 0, 0);
	if (lresult != CB_OKAY) {
		//ドキュメントには常にCB_OKAY(0)を返すと記載あるが、実際はTRUE(1)を返している
		//戻り値はチェックしないことにする
		//result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		//goto EXIT;
	}

	//デフォルトファイルをリストに追加
	comboIndex = 0;
	swprintf_s(itemStr, 256, L"Default (%lS %lS)", MT_WAVETABLE_DEFAULT_FILE_NAME, MT_WAVETABLE_DEFAULT_FILE_VER);
	lresult = SendMessageW(m_hComboWavetableFile, CB_ADDSTRING, 0, (LPARAM)itemStr);
	if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	lresult = SendMessage(m_hComboWavetableFile, CB_SETITEMDATA, comboIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
		goto EXIT;
	}

	//ユーザ選択ウェーブテーブルファイルをリストに追加
	if (wcslen (m_UserWavetableFilePath) != 0) {
		comboIndex = 1;
		pFileName = PathFindFileNameW(m_UserWavetableFilePath);
		lresult = SendMessageW(m_hComboWavetableFile, CB_ADDSTRING, 0, (LPARAM)pFileName);
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		lresult = SendMessage(m_hComboWavetableFile, CB_SETITEMDATA, comboIndex, 1);
		if (lresult == CB_ERR) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)comboIndex);
			goto EXIT;
		}
	}

	//選択状態設定
	lresult = SendMessage(m_hComboWavetableFile, CB_SETCURSEL, m_SelectedWavetableFileIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), m_SelectedWavetableFileIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Selectボタン押下
//******************************************************************************
int MTWavetableSynthCfgDlg::_OnBtnSelectFile()
{
	int result = 0;
	BOOL apiresult = FALSE;
	errno_t eresult = 0;
	WCHAR filePath[_MAX_PATH] = { L'\0' };
	OPENFILENAMEW ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner   = m_hWnd;
	ofn.lpstrFilter = L"SF2 file (*.sf2)\0*.sf2\0";
	ofn.lpstrFile   = filePath;
	ofn.nMaxFile    = _MAX_PATH;
	ofn.lpstrTitle  = L"Select SF2 file.";
	ofn.Flags       = OFN_FILEMUSTEXIST;  //OFN_HIDEREADONLY

	//ファイル選択ダイアログ表示
	apiresult = GetOpenFileNameW(&ofn);
	if (!apiresult) {
		//キャンセルまたはエラー発生：エラーはチェックしない
		goto EXIT;
	}

	//新たに選択されたファイルのパス
	eresult = wcscpy_s(m_UserWavetableFilePath, _MAX_PATH, filePath);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", eresult, 0);
		goto EXIT;
	}

	//ユーザ選択ファイルを選択状態にする
	m_SelectedWavetableFileIndex = 1;

	//ウェーブテーブルファイルメニュー更新
	result = _UpdateWavetableFileList();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Clearボタン押下
//******************************************************************************
int MTWavetableSynthCfgDlg::_OnBtnClearFile()
{
	int result = 0;

	//ユーザ選択ファイルをクリア
	m_UserWavetableFilePath[0] = L'\0';

	//デフォルトファイルを選択状態にする
	m_SelectedWavetableFileIndex = 0;

	//ウェーブテーブルファイルメニュー更新
	result = _UpdateWavetableFileList();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Acknowledgementsボタン押下
//******************************************************************************
int MTWavetableSynthCfgDlg::_OnBtnAcknowledgements()
{
	int result = 0;
	HINSTANCE hresult = 0;
	TCHAR manualPath[_MAX_PATH] = { _T('\0') };
	LANGID langId = 0;

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPath(manualPath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//マニュアルファイルパス作成
	langId = GetUserDefaultUILanguage();
	if (langId == 0x0411) {
		//日本語(ja-JP)の場合
		_tcscat_s(manualPath, _MAX_PATH, MT_ACKNOWLEDGEMENTS_JA);
	}
	else {
		//それ以外
		_tcscat_s(manualPath, _MAX_PATH, MT_ACKNOWLEDGEMENTS_EN);
	}

	//マニュアルファイルを開く
	hresult = ShellExecute(
					NULL,			//親ウィンドウハンドル
					_T("open"),		//操作
					manualPath,		//操作対象のファイル
					NULL,			//操作パラメータ
					NULL,			//既定ディレクトリ
					SW_SHOWNORMAL	//表示状態
				);
	if (hresult <= (HINSTANCE)32) {
		result = YN_SET_ERR("File open error.", (DWORD64)hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 設定保存
//******************************************************************************
int MTWavetableSynthCfgDlg::_Save()
{
	int result = 0;

	//設定保存：ウェーブテーブルファイル
	result = _SaveWavetableFile();
	if (result != 0) goto EXIT;

	//設定保存：最大同時発音数
	result = _SaveMaxVoices();
	if (result != 0) goto EXIT;

	//設定保存：サスティン
	result = _SaveSustain();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 設定保存：ウェーブテーブルファイル
//******************************************************************************
int MTWavetableSynthCfgDlg::_SaveWavetableFile()
{
	int result = 0;
	LRESULT lresult = 0;
	int selectedIndex = 0;

	//選択項目のインデックスを取得
	lresult = SendMessage(m_hComboWavetableFile, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	selectedIndex = (int)lresult;

	//ウェーブテーブルファイルインデックス
	result = m_ConfFile.SetInt(_T("WavetableIndex"), selectedIndex);
	if (result != 0) goto EXIT;

	//ユーザ選択ウェーブテーブルファイルパス
	result = m_ConfFile.SetWStr(_T("WavetableFilePath"), m_UserWavetableFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 設定保存：最大同時発音数
//******************************************************************************
int MTWavetableSynthCfgDlg::_SaveMaxVoices()
{
	int result = 0;
	LRESULT lresult = 0;
	int selectedIndex = 0;
	int maxVoices = 0;

	//選択項目のインデックスを取得
	lresult = SendMessage(m_hComboMaxVoices, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	selectedIndex = (int)lresult;

	//選択項目のユーザデータを取得
	lresult = SendMessage(m_hComboMaxVoices, CB_GETITEMDATA, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}
	maxVoices = (int)lresult;

	//ウェーブテーブルファイルインデックス
	result = m_ConfFile.SetInt(_T("MaxVoices"), maxVoices);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 設定保存：サスティン
//******************************************************************************
int MTWavetableSynthCfgDlg::_SaveSustain()
{
	int result = 0;
	LRESULT lresult = 0;
	int selectedIndex = 0;
	int sustain = 0;

	//選択項目のインデックスを取得
	lresult = SendMessage(m_hComboSustain, CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	selectedIndex = (int)lresult;

	//選択項目のユーザデータを取得
	lresult = SendMessage(m_hComboSustain, CB_GETITEMDATA, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}
	sustain = (int)lresult;

	//ウェーブテーブルファイルインデックス
	result = m_ConfFile.SetInt(_T("Sustain"), sustain);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


