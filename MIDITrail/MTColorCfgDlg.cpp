//******************************************************************************
//
// MIDITrail / MTColorCfgDlg
//
// カラー設定ダイアログ
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "MTColorCfgDlg.h"


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTColorCfgDlg* MTColorCfgDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTColorCfgDlg::MTColorCfgDlg(void)
{
	m_pThis = this;
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_SelectedColorPaletteNo = 0;
	m_isChanged = false;

	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTColorCfgDlg::~MTColorCfgDlg(void)
{
	return;
}

//******************************************************************************
// 表示
//******************************************************************************
int MTColorCfgDlg::Show(
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
					MAKEINTRESOURCE(IDD_COLOR_CFG),		//ダイアログボックステンプレート
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
bool MTColorCfgDlg::IsChanged()
{
	return m_isChanged;
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTColorCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTColorCfgDlg::_WndProcImpl(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	LRESULT lResult = 0;

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
			else if (LOWORD(wParam) == IDC_RADIO_DEFAULT) {
				result = _OnBtnRadio(0);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_1) {
				result = _OnBtnRadio(1);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_2) {
				result = _OnBtnRadio(2);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_3) {
				result = _OnBtnRadio(3);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_4) {
				result = _OnBtnRadio(4);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_5) {
				result = _OnBtnRadio(5);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_RADIO_PALETTE_6) {
				result = _OnBtnRadio(6);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_1) {
				result = _OnBtnEdit(1);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_2) {
				result = _OnBtnEdit(2);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_3) {
				result = _OnBtnEdit(3);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_4) {
				result = _OnBtnEdit(4);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_5) {
				result = _OnBtnEdit(5);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EDIT_6) {
				result = _OnBtnEdit(6);
				if (result != 0) goto EXIT;
			}
			break;
		case WM_DRAWITEM:
			//カラーボタン描画
			result = _DrawColorButton((DRAWITEMSTRUCT*)lParam);
			if (result != 0) goto EXIT;
			break;
		default:
			//処理しないメッセージ
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(hDlg);
	}
	return lResult;
}

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTColorCfgDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isChanged = false;

	//ラジオボタンリスト初期化
	_InitRadioButtonList();

	//カラーボタンリスト初期化
	_InitColorButtonList();

	//色設定初期化：PianoRoll3D.ini の色設定をデフォルト設定として表示
	result = m_ColorConf.Initialize(_T("PianoRoll3D"));
	if (result != 0) goto EXIT;

	//ラジオボタン初期化
	result = _InitRadioButtons();
	if (result != 0) goto EXIT;

	//カラーボタン初期化
	result = _InitColorButtons();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ラジオボタンリスト初期化
//******************************************************************************
void MTColorCfgDlg::_InitRadioButtonList()
{
	m_hBtnRadioPaletteList[0] = GetDlgItem(m_hWnd, IDC_RADIO_DEFAULT);
	m_hBtnRadioPaletteList[1] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_1);
	m_hBtnRadioPaletteList[2] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_2);
	m_hBtnRadioPaletteList[3] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_3);
	m_hBtnRadioPaletteList[4] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_4);
	m_hBtnRadioPaletteList[5] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_5);
	m_hBtnRadioPaletteList[6] = GetDlgItem(m_hWnd, IDC_RADIO_PALETTE_6);
	
	return;
}

//******************************************************************************
// カラーボタンリスト初期化
//******************************************************************************
void MTColorCfgDlg::_InitColorButtonList()
{
	//カラーパレット デフォルト
	m_hBtnColorList[0][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_1);
	m_hBtnColorList[0][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_2);
	m_hBtnColorList[0][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_3);
	m_hBtnColorList[0][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_4);
	m_hBtnColorList[0][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_5);
	m_hBtnColorList[0][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_6);
	m_hBtnColorList[0][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_7);
	m_hBtnColorList[0][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_8);
	m_hBtnColorList[0][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_9);
	m_hBtnColorList[0][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_10);
	m_hBtnColorList[0][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_11);
	m_hBtnColorList[0][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_12);
	m_hBtnColorList[0][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_13);
	m_hBtnColorList[0][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_14);
	m_hBtnColorList[0][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_15);
	m_hBtnColorList[0][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_16);
	m_hBtnColorList[0][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_BG);
	m_hBtnColorList[0][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_GL);
	m_hBtnColorList[0][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_0_CT);
	//カラーパレット 1
	m_hBtnColorList[1][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_1);
	m_hBtnColorList[1][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_2);
	m_hBtnColorList[1][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_3);
	m_hBtnColorList[1][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_4);
	m_hBtnColorList[1][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_5);
	m_hBtnColorList[1][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_6);
	m_hBtnColorList[1][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_7);
	m_hBtnColorList[1][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_8);
	m_hBtnColorList[1][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_9);
	m_hBtnColorList[1][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_10);
	m_hBtnColorList[1][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_11);
	m_hBtnColorList[1][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_12);
	m_hBtnColorList[1][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_13);
	m_hBtnColorList[1][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_14);
	m_hBtnColorList[1][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_15);
	m_hBtnColorList[1][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_16);
	m_hBtnColorList[1][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_BG);
	m_hBtnColorList[1][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_GL);
	m_hBtnColorList[1][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1_CT);
	//カラーパレット 2
	m_hBtnColorList[2][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_1);
	m_hBtnColorList[2][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_2);
	m_hBtnColorList[2][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_3);
	m_hBtnColorList[2][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_4);
	m_hBtnColorList[2][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_5);
	m_hBtnColorList[2][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_6);
	m_hBtnColorList[2][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_7);
	m_hBtnColorList[2][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_8);
	m_hBtnColorList[2][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_9);
	m_hBtnColorList[2][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_10);
	m_hBtnColorList[2][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_11);
	m_hBtnColorList[2][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_12);
	m_hBtnColorList[2][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_13);
	m_hBtnColorList[2][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_14);
	m_hBtnColorList[2][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_15);
	m_hBtnColorList[2][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_16);
	m_hBtnColorList[2][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_BG);
	m_hBtnColorList[2][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_GL);
	m_hBtnColorList[2][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2_CT);
	//カラーパレット 3
	m_hBtnColorList[3][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_1);
	m_hBtnColorList[3][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_2);
	m_hBtnColorList[3][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_3);
	m_hBtnColorList[3][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_4);
	m_hBtnColorList[3][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_5);
	m_hBtnColorList[3][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_6);
	m_hBtnColorList[3][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_7);
	m_hBtnColorList[3][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_8);
	m_hBtnColorList[3][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_9);
	m_hBtnColorList[3][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_10);
	m_hBtnColorList[3][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_11);
	m_hBtnColorList[3][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_12);
	m_hBtnColorList[3][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_13);
	m_hBtnColorList[3][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_14);
	m_hBtnColorList[3][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_15);
	m_hBtnColorList[3][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_16);
	m_hBtnColorList[3][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_BG);
	m_hBtnColorList[3][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_GL);
	m_hBtnColorList[3][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3_CT);
	//カラーパレット 4
	m_hBtnColorList[4][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_1);
	m_hBtnColorList[4][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_2);
	m_hBtnColorList[4][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_3);
	m_hBtnColorList[4][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_4);
	m_hBtnColorList[4][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_5);
	m_hBtnColorList[4][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_6);
	m_hBtnColorList[4][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_7);
	m_hBtnColorList[4][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_8);
	m_hBtnColorList[4][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_9);
	m_hBtnColorList[4][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_10);
	m_hBtnColorList[4][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_11);
	m_hBtnColorList[4][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_12);
	m_hBtnColorList[4][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_13);
	m_hBtnColorList[4][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_14);
	m_hBtnColorList[4][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_15);
	m_hBtnColorList[4][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_16);
	m_hBtnColorList[4][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_BG);
	m_hBtnColorList[4][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_GL);
	m_hBtnColorList[4][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4_CT);
	//カラーパレット 5
	m_hBtnColorList[5][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_1);
	m_hBtnColorList[5][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_2);
	m_hBtnColorList[5][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_3);
	m_hBtnColorList[5][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_4);
	m_hBtnColorList[5][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_5);
	m_hBtnColorList[5][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_6);
	m_hBtnColorList[5][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_7);
	m_hBtnColorList[5][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_8);
	m_hBtnColorList[5][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_9);
	m_hBtnColorList[5][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_10);
	m_hBtnColorList[5][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_11);
	m_hBtnColorList[5][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_12);
	m_hBtnColorList[5][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_13);
	m_hBtnColorList[5][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_14);
	m_hBtnColorList[5][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_15);
	m_hBtnColorList[5][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_16);
	m_hBtnColorList[5][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_BG);
	m_hBtnColorList[5][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_GL);
	m_hBtnColorList[5][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5_CT);
	//カラーパレット 6
	m_hBtnColorList[6][0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_1);
	m_hBtnColorList[6][1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_2);
	m_hBtnColorList[6][2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_3);
	m_hBtnColorList[6][3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_4);
	m_hBtnColorList[6][4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_5);
	m_hBtnColorList[6][5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_6);
	m_hBtnColorList[6][6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_7);
	m_hBtnColorList[6][7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_8);
	m_hBtnColorList[6][8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_9);
	m_hBtnColorList[6][9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_10);
	m_hBtnColorList[6][10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_11);
	m_hBtnColorList[6][11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_12);
	m_hBtnColorList[6][12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_13);
	m_hBtnColorList[6][13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_14);
	m_hBtnColorList[6][14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_15);
	m_hBtnColorList[6][15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_16);
	m_hBtnColorList[6][16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_BG);
	m_hBtnColorList[6][17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_GL);
	m_hBtnColorList[6][18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6_CT);
	
	return;
}

//******************************************************************************
// ラジオボタン初期化
//******************************************************************************
int MTColorCfgDlg::_InitRadioButtons()
{
	int result = 0;
	unsigned long colorPaletteNo = 0;

	//選択カラーパレット番号取得
	colorPaletteNo = m_ColorConf.GetSelectedColorPaletteNo();
	if (colorPaletteNo >= MT_COLOR_PALETTE_NUM_MAX) {
		result = YN_SET_ERR("Program error.", colorPaletteNo, 0);
		goto EXIT;
	}

	//ラジオボタン選択状態設定
	SendMessage(m_hBtnRadioPaletteList[colorPaletteNo], BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	m_SelectedColorPaletteNo = colorPaletteNo;

EXIT:;
	return result;
}

//******************************************************************************
// カラーボタン初期化
//******************************************************************************
int MTColorCfgDlg::_InitColorButtons()
{
	int result = 0;
	
	//WM_DRAWITEMメッセージ受信時にボタンを描画するためここでは何もしない
	
	return result;
}

//******************************************************************************
// ラジオボタン押下
//******************************************************************************
int MTColorCfgDlg::_OnBtnRadio(unsigned long buttonNo)
{
	int result = 0;

	m_SelectedColorPaletteNo = buttonNo;

	//カラー設定に登録
	result = m_ColorConf.SetSelectedColorPaletteNo(m_SelectedColorPaletteNo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 編集ボタン押下
//******************************************************************************
int MTColorCfgDlg::_OnBtnEdit(unsigned long paletteNo)
{
	int result = 0;
	
	//カラーパレット設定ダイアログ表示
	result = _ShowColorPaletteCfgDlg(paletteNo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// カラーパレット設定ダイアログ表示
//******************************************************************************
int MTColorCfgDlg::_ShowColorPaletteCfgDlg(unsigned long colorPaletteNo)
{
	int result = 0;
	MTColorPalette colorPalette;
	MTColorPalette defaultColorPalette;

	//カラーパレット取得
	result = m_ColorConf.GetColorPalette(colorPaletteNo, &colorPalette);
	if (result != 0) goto EXIT;
	result = m_ColorConf.GetColorPalette(0, &defaultColorPalette);
	if (result != 0) goto EXIT;

	//カラーパレット設定
	m_ColorPaletteCfgDlg.SetColorPalette(&colorPalette, &defaultColorPalette, colorPaletteNo);

	//ダイアログ表示
	result = m_ColorPaletteCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;	

	//変更された場合はレンダラとシーンオブジェクトを再生成
	if (m_ColorPaletteCfgDlg.IsChanged()) {
		//更新されたカラーパレットを取得
		m_ColorPaletteCfgDlg.GetColorPalette(&colorPalette);
		//カラーパレット登録
		result = m_ColorConf.SetColorPalette(colorPaletteNo, &colorPalette);
		if (result != 0) goto EXIT;
		//カラーボタン更新
		result = _UpdateColorButtons(colorPaletteNo);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// カラーボタン更新
//******************************************************************************
int MTColorCfgDlg::_UpdateColorButtons(unsigned long colorPaletteNo)
{
	int result = 0;
	BOOL bResult = FALSE;
	unsigned long i = 0;

	//ボタン再描画を指示：WM_DRAWITEMメッセージで更新
	for (i = 0; i < (SM_MAX_CH_NUM + 3); i++) {
		bResult = InvalidateRect(m_hBtnColorList[colorPaletteNo][i], NULL, FALSE);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), i);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// カラーボタン描画
//******************************************************************************
int MTColorCfgDlg::_DrawColorButton(DRAWITEMSTRUCT* pDrawItem)
{
	int result = 0;
	BOOL bResult = FALSE;
	bool isFound = false;
	unsigned long i = 0;
	unsigned long j = 0;
	unsigned long paletteNo = 0;
	unsigned long chNo = 0;
	MTColorPalette colorPalette;
	D3DXCOLOR color;
	COLORREF bkColor1;
	COLORREF bkColor2;
	
	//---------------------------------
	// ボタン特定
	//---------------------------------
	//コントロールタイプがボタンでなければ何もしない
	if (pDrawItem->CtlType != ODT_BUTTON) goto EXIT;
	
	//コントロールID
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		for (j = 0; j < (SM_MAX_CH_NUM + 3); j++) {
			if (pDrawItem->CtlID == GetDlgCtrlID(m_hBtnColorList[i][j])) {
				//対象のボタンを識別
				isFound = true;
				paletteNo = i;
				chNo = j;
			}
		}
	}
	//対象のボタンが見つからなかったら何もしない
	if (!isFound) goto EXIT;
	
	//---------------------------------
	// 色特定
	//---------------------------------
	//カラーパレット取得
	result = m_ColorConf.GetColorPalette(paletteNo, &colorPalette);
	if (result != 0) goto EXIT;
	
	//色取得
	if (chNo < SM_MAX_CH_NUM) {
		//Ch.1 - 16
		colorPalette.GetChColor(chNo, &color);
	}
	else if (chNo == SM_MAX_CH_NUM + 0) {
		//背景
		colorPalette.GetBackgroundColor(&color);
	}
	else if (chNo == SM_MAX_CH_NUM + 1) {
		//グリッドライン
		colorPalette.GetGridLineColor(&color);
	}
	else if (chNo == SM_MAX_CH_NUM + 2) {
		//カウンター
		colorPalette.GetCounterColor(&color);
	}
	
	//---------------------------------
	// ボタン描画
	//---------------------------------
	//デバイスコンテキストの背景色を設定
	bkColor1 = SetBkColor(pDrawItem->hDC, RGB(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f));
	if (bkColor1 == CLR_INVALID) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//ボタンの四角形を描画
	bResult = ExtTextOut(
					pDrawItem->hDC,	//デバイスコンテキスト
					0,				//文字列配置参照ポイントX座標（論理座標）
					0,				//文字列配置参照ポイントY座標（論理座標）
					ETO_OPAQUE,		//四角形使用方法：現在の背景色を使用して四角形を塗りつぶす
					&(pDrawItem->rcItem),	//四角形の論理座標
					NULL,			//描画するテキスト
					0,				//文字列の長さ
					NULL			//隣接する文字セルの原点間の距離
				);
	if (!bResult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//ボタン押下状態を反映した境界を描画
	if (pDrawItem->itemState & ODS_SELECTED) {
		//沈んだ状態を描画
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_SUNKEN, BF_TOPLEFT | BF_BOTTOMRIGHT | BF_FLAT);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	else {
		//浮いた状態を描画
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_RAISED, BF_TOPLEFT | BF_BOTTOMRIGHT | BF_FLAT);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	
	//デバイスコンテキストの背景色を戻す
	bkColor2 = SetBkColor(pDrawItem->hDC, bkColor1);
	if (bkColor2 == CLR_INVALID) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// カラー情報保存
//******************************************************************************
int MTColorCfgDlg::_Save()
{
	int result = 0;
	
	//カラー設定を保存
	result = m_ColorConf.Save();
	if (result != 0) goto EXIT;
	
	m_isChanged = true;
	
EXIT:;
	return result;
}


