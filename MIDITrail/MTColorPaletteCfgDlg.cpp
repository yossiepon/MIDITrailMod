//******************************************************************************
//
// MIDITrail / MTColorPaletteCfgDlg
//
// カラーパレット設定ダイアログ
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTColorPaletteCfgDlg.h"
#include "MTColorParamExportDlg.h"
#include "MTColorParamImportDlg.h"
#include <Commdlg.h>
#include <sstream> //for std::stringstream
#include <istream> //for std::getline

using namespace YNBaseLib;


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTColorPaletteCfgDlg* MTColorPaletteCfgDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTColorPaletteCfgDlg::MTColorPaletteCfgDlg(void)
{
	unsigned long i = 0;
	
	m_pThis = this;
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_ColorPaletteNo = 0;
	m_isChanged = false;

	//色設定 Start/End
	m_ColorStart = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
	m_ColorEnd = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA

	//色選択ダイアログ用パラメータ
	for (i = 0; i < 16; i++) {
		m_CustColors[i] = RGB(255, 255, 255);
	}

	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTColorPaletteCfgDlg::~MTColorPaletteCfgDlg(void)
{
	return;
}

//******************************************************************************
// カラーパレット設定
//******************************************************************************
void MTColorPaletteCfgDlg::SetColorPalette(
			MTColorPalette* pColorPalette, 
			MTColorPalette* pDefaultColorPalette,
		 	unsigned long colorPaletteNo
		 )
{
	m_ColorPalette.CopyFrom(pColorPalette);
	m_DefaultColorPalette.CopyFrom(pDefaultColorPalette);
	m_ColorPaletteNo = colorPaletteNo;
}

//******************************************************************************
// カラーパレット取得
//******************************************************************************
void MTColorPaletteCfgDlg::GetColorPalette(MTColorPalette* pColorPalette)
{
	pColorPalette->CopyFrom(&m_ColorPalette);
}

//******************************************************************************
// 表示
//******************************************************************************
int MTColorPaletteCfgDlg::Show(
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
					MAKEINTRESOURCE(IDD_COLOR_PALETTE_CFG),	//ダイアログボックステンプレート
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
bool MTColorPaletteCfgDlg::IsChanged()
{
	return m_isChanged;
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTColorPaletteCfgDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTColorPaletteCfgDlg::_WndProcImpl(
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
				m_isChanged = true;
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_1) {
				result = _OnBtnColor(0);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_2) {
				result = _OnBtnColor(1);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_3) {
				result = _OnBtnColor(2);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_4) {
				result = _OnBtnColor(3);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_5) {
				result = _OnBtnColor(4);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_6) {
				result = _OnBtnColor(5);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_7) {
				result = _OnBtnColor(6);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_8) {
				result = _OnBtnColor(7);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_9) {
				result = _OnBtnColor(8);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_10) {
				result = _OnBtnColor(9);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_11) {
				result = _OnBtnColor(10);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_12) {
				result = _OnBtnColor(11);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_13) {
				result = _OnBtnColor(12);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_14) {
				result = _OnBtnColor(13);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_15) {
				result = _OnBtnColor(14);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_16) {
				result = _OnBtnColor(15);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_BG) {
				result = _OnBtnColor(16);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_GL) {
				result = _OnBtnColor(17);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_CT) {
				result = _OnBtnColor(18);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_START) {
				result = _OnBtnColor(19);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_COLOR_END) {
				result = _OnBtnColor(20);
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_SET_GRADATION_COLORS) {
				result = _OnBtnSetGradationColors();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_SET_DEFAULT_COLORS) {
				result = _OnBtnSetDefaultColors();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_EXPORT_COLOR_PARAMETERS) {
				result = _OonBtnExportColorParameters();
				if (result != 0) goto EXIT;
			}
			else if (LOWORD(wParam) == IDC_BTN_IMPORT_COLOR_PARAMETERS) {
				result = _OnBtnImportColorParameters();
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
	return lresult;
}

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTColorPaletteCfgDlg::_OnInitDlg(HWND hDlg)
{
	int result = 0;

	m_hWnd = hDlg;
	m_isChanged = false;
	TCHAR title[256] = {_T('\0')};

	//カラーボタンリスト初期化
	_InitColorButtonList();

	//カラーボタン初期化
	result = _InitColorButtons();
	if (result != 0) goto EXIT;

	//カラーテキスト初期化
	result = _InitColorText();
	if (result != 0) goto EXIT;

	//ウィンドウタイトル
	_stprintf_s(title, 256, _T("Color Palette %u"), m_ColorPaletteNo);
	SetWindowText(m_hWnd, title);

	//コンボボックス Start/End 初期化
	result = _InitCombobox(GetDlgItem(m_hWnd, IDC_COMBO_START), 0);
	if (result != 0) goto EXIT;
	result = _InitCombobox(GetDlgItem(m_hWnd, IDC_COMBO_END), SM_MAX_CH_NUM - 1);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// カラーボタンリスト初期化
//******************************************************************************
void MTColorPaletteCfgDlg::_InitColorButtonList()
{
	//カラーボタンリスト
	m_hColorButtonList[0]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_1);
	m_hColorButtonList[1]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_2);
	m_hColorButtonList[2]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_3);
	m_hColorButtonList[3]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_4);
	m_hColorButtonList[4]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_5);
	m_hColorButtonList[5]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_6);
	m_hColorButtonList[6]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_7);
	m_hColorButtonList[7]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_8);
	m_hColorButtonList[8]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_9);
	m_hColorButtonList[9]  = GetDlgItem(m_hWnd, IDC_BTN_COLOR_10);
	m_hColorButtonList[10] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_11);
	m_hColorButtonList[11] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_12);
	m_hColorButtonList[12] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_13);
	m_hColorButtonList[13] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_14);
	m_hColorButtonList[14] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_15);
	m_hColorButtonList[15] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_16);
	m_hColorButtonList[16] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_BG);
	m_hColorButtonList[17] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_GL);
	m_hColorButtonList[18] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_CT);
	m_hColorButtonList[19] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_START);
	m_hColorButtonList[20] = GetDlgItem(m_hWnd, IDC_BTN_COLOR_END);
	//カラーテキストリスト
	m_hColorTextList[0]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_1);
	m_hColorTextList[1]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_2);
	m_hColorTextList[2]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_3);
	m_hColorTextList[3]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_4);
	m_hColorTextList[4]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_5);
	m_hColorTextList[5]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_6);
	m_hColorTextList[6]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_7);
	m_hColorTextList[7]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_8);
	m_hColorTextList[8]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_9);
	m_hColorTextList[9]  = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_10);
	m_hColorTextList[10] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_11);
	m_hColorTextList[11] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_12);
	m_hColorTextList[12] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_13);
	m_hColorTextList[13] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_14);
	m_hColorTextList[14] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_15);
	m_hColorTextList[15] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_16);
	m_hColorTextList[16] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_BG);
	m_hColorTextList[17] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_GL);
	m_hColorTextList[18] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_CT);
	m_hColorTextList[19] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_START);
	m_hColorTextList[20] = GetDlgItem(m_hWnd, IDC_COLOR_TEXT_END);

	return;
}

//******************************************************************************
// カラーボタン初期化
//******************************************************************************
int MTColorPaletteCfgDlg::_InitColorButtons()
{
	int result = 0;
	
	//WM_DRAWITEMメッセージ受信時にボタンを描画するためここでは何もしない
	
	return result;
}

//******************************************************************************
// カラーテキスト初期化
//******************************************************************************
int MTColorPaletteCfgDlg::_InitColorText()
{
	int result = 0;
	unsigned long targetNo = 0;
	D3DXCOLOR color;
	TCHAR hexColor[16] = {_T('\0')};
	BOOL bResult = FALSE;

	for (targetNo = 0; targetNo < SM_MAX_CH_NUM + 3 + 2; targetNo++) {
		//現在の色を取得
		result = _GetCurColor(targetNo, &color);
		if (result != 0) goto EXIT;

		//カラーテキスト設定
		DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
		bResult = SetWindowText(m_hColorTextList[targetNo], hexColor);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// コンボボックス初期化
//******************************************************************************
int MTColorPaletteCfgDlg::_InitCombobox(HWND hCombobox, int selectedIndex)
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long chNo = 0;
	TCHAR itemStr[16] = {_T('\0')};
	
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(itemStr, 16, _T("Ch.%u"), chNo + 1);
		lresult = SendMessage(hCombobox, CB_ADDSTRING, 0, (LPARAM)itemStr);
		if ((lresult == CB_ERR) || (lresult == CB_ERRSPACE)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		lresult = SendMessage(hCombobox, CB_SETITEMDATA, chNo, 0);
		if (lresult == CB_ERR) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), chNo);
			goto EXIT;
		}
	}
	
	//選択状態設定
	lresult = SendMessage(hCombobox, CB_SETCURSEL, selectedIndex, 0);
	if (lresult == CB_ERR) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), selectedIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// カラーボタン押下
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnColor(unsigned long targetNo)
{
	int result = 0;
	D3DXCOLOR color;
	D3DXCOLOR newColor;
	bool isChoosed = false;
	TCHAR hexColor[16] = {_T('\0')};
	BOOL bResult = FALSE;

	//現在の色を取得
	result = _GetCurColor(targetNo, &color);
	if (result != 0) goto EXIT;

	//カラー選択ダイアログ表示
	newColor = color;
	result = _ShowChooseColorDlg(color, &newColor, &isChoosed);
	if (result != 0) goto EXIT;

	//新しい色が選択された場合
	if (isChoosed) {
		//現在の色を設定
		result = _SetCurColor(targetNo, newColor);
		if (result != 0) goto EXIT;

		//カラーボタン再描画を指示：WM_DRAWITEMメッセージで更新
		InvalidateRect(m_hColorButtonList[targetNo], NULL, FALSE);

		//カラーテキスト更新
		DXColorUtil::MakeHexRGBAFromColor(newColor, hexColor, 16);
		bResult = SetWindowText(m_hColorTextList[targetNo], hexColor);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// グラデーションツール：Set Gradation Colors ボタン押下
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnSetGradationColors()
{
	int result = 0;
	LRESULT lresult = 0;
	unsigned long chNoStart = 0;
	unsigned long chNoEnd = 0;

	//選択されたチャンネル番号 Start
	lresult = SendMessage(GetDlgItem(m_hWnd, IDC_COMBO_START), CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	chNoStart = (unsigned long)lresult;

	//選択されたチャンネル番号 End
	lresult = SendMessage(GetDlgItem(m_hWnd, IDC_COMBO_END), CB_GETCURSEL, 0, 0);
	if ((lresult == CB_ERR) || (lresult < 0)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	chNoEnd = (unsigned long)lresult;

	//グラデーションカラー設定
	result = _SetGradationColor(chNoStart, chNoEnd, m_ColorStart, m_ColorEnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// パラメータセットアップツール：Set Default Colors ボタン押下
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnSetDefaultColors()
{
	int result = 0;

	//デフォルトカラーパレットを反映
	m_ColorPalette.CopyFrom(&m_DefaultColorPalette);

	//カラーボタン更新
	result = _UpdateColorButtons();
	if (result != 0) goto EXIT;
	
	//カラーテキスト更新
	result = _UpdateColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// パラメータセットアップツール：Export Color Parameters ボタン押下
//******************************************************************************
int MTColorPaletteCfgDlg::_OonBtnExportColorParameters()
{
	int result = 0;
	TCHAR paramString[2048] = {_T('\0')};
	MTColorParamExportDlg colorParamExportDlg;
	
	//出力用パラメータ文字列生成
	result = _MakeColorParamForExport(paramString, 2048);
	if (result != 0) goto EXIT;

	//出力用パラメータ文字列設定
	colorParamExportDlg.SetParamString(paramString);

	//カラーパラメータ出力ダイアログ表示
	result = colorParamExportDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// パラメータセットアップツール：Import Color Parameters ボタン押下
//******************************************************************************
int MTColorPaletteCfgDlg::_OnBtnImportColorParameters()
{
	int result = 0;
	MTColorParamImportDlg colorParamImportDlg;

	//カラーパラメータ入力ダイアログ表示
	result = colorParamImportDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//カラーパラメータ入力処理
	if (colorParamImportDlg.IsExecImport()) {
		result = _ImportColorParam(colorParamImportDlg.GetParamString());
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// カラーボタン更新
//******************************************************************************
int MTColorPaletteCfgDlg::_UpdateColorButtons()
{
	int result = 0;
	BOOL bResult = FALSE;
	unsigned long i = 0;

	//ボタン再描画を指示：WM_DRAWITEMメッセージで更新
	for (i = 0; i < (SM_MAX_CH_NUM + 3 + 2); i++) {
		bResult = InvalidateRect(m_hColorButtonList[i], NULL, FALSE);
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
int MTColorPaletteCfgDlg::_DrawColorButton(DRAWITEMSTRUCT* pDrawItem)
{
	int result = 0;
	BOOL bResult = FALSE;
	bool isFound = false;
	unsigned long i = 0;
	unsigned long targetNo = 0;
	D3DXCOLOR color;
	COLORREF bkColor1;
	COLORREF bkColor2;
	
	//コントロールタイプがボタンでなければ何もしない
	if (pDrawItem->CtlType != ODT_BUTTON) goto EXIT;

	//---------------------------------
	// ボタンとカラーを特定
	//---------------------------------	
	//コントロールID
	for (i = 0; i < (SM_MAX_CH_NUM + 3 + 2); i++) {
		if (pDrawItem->CtlID == GetDlgCtrlID(m_hColorButtonList[i])) {
			//対象のボタンを識別
			isFound = true;
			targetNo = i;
		}
	}
	//対象のボタンが見つからなかったら何もしない
	if (!isFound) goto EXIT;

	//カラー取得
	result = _GetCurColor(targetNo, &color);
	if (result != 0) goto EXIT;
	
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
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_SUNKEN, BF_TOPLEFT | BF_BOTTOMRIGHT);
		if (!bResult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	else {
		//浮いた状態を描画
		bResult = DrawEdge(pDrawItem->hDC, &(pDrawItem->rcItem), EDGE_RAISED, BF_TOPLEFT | BF_BOTTOMRIGHT);
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
// カラーテキスト更新
//******************************************************************************
int MTColorPaletteCfgDlg::_UpdateColorText()
{
	int result = 0;

	result =_InitColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// カラー取得
//******************************************************************************
int MTColorPaletteCfgDlg::_GetCurColor(
		unsigned long targetNo,
		D3DXCOLOR* pColor
	)
{
	int result = 0;

	if (targetNo < SM_MAX_CH_NUM) {
		//Ch.1 - 16
		m_ColorPalette.GetChColor(targetNo, pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 0) {
		//背景
		m_ColorPalette.GetBackgroundColor(pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 1) {
		//グリッドライン
		m_ColorPalette.GetGridLineColor(pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 2) {
		//カウンター
		m_ColorPalette.GetCounterColor(pColor);
	}
	else if (targetNo == SM_MAX_CH_NUM + 3) {
		//Start
		*pColor = m_ColorStart;
	}
	else if (targetNo == SM_MAX_CH_NUM + 4) {
		//End
		*pColor = m_ColorEnd;
	}
	else {
		result = YN_SET_ERR("Program error.", targetNo, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// カラー設定
//******************************************************************************
int MTColorPaletteCfgDlg::_SetCurColor(
	unsigned long targetNo,
	D3DXCOLOR color
)
{
	int result = 0;

	if (targetNo < SM_MAX_CH_NUM) {
		//Ch.1 - 16
		m_ColorPalette.SetChColor(targetNo, color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 0) {
		//背景
		m_ColorPalette.SetBackgroundColor(color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 1) {
		//グリッドライン
		m_ColorPalette.SetGridLineColor(color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 2) {
		//カウンター
		m_ColorPalette.SetCounterColor(color);
	}
	else if (targetNo == SM_MAX_CH_NUM + 3) {
		//Start
		m_ColorStart = color;
	}
	else if (targetNo == SM_MAX_CH_NUM + 4) {
		//End
		m_ColorEnd = color;
	}
	else {
		result = YN_SET_ERR("Program error.", targetNo, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// カラー選択ダイアログ表示
//******************************************************************************
int MTColorPaletteCfgDlg::_ShowChooseColorDlg(
		D3DXCOLOR color,
		D3DXCOLOR* pNewColor,
		bool* pIsChoosed
	)
{
	int result = 0;
	CHOOSECOLOR cc;
	BOOL bResult = FALSE;
	DWORD dlgErrorCode;

	//カラー選択ダイアログの設定
	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = m_hWnd;
	cc.hInstance = NULL;
	cc.rgbResult = RGB(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f);
	cc.lpCustColors = m_CustColors;
	cc.Flags = CC_FULLOPEN		//ダイアログボックス全体を表示 
				| CC_RGBINIT;	//rgbResultメンバで指定した色を初期カラーとして使用
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;

	//カラー選択ダイアログ表示
	bResult = ChooseColor(&cc);
	if (!bResult) {
		dlgErrorCode = CommDlgExtendedError();
		if (dlgErrorCode == 0) {
			//キャンセル
			*pIsChoosed = false;
		}
		else {
			//エラー発生
			result = YN_SET_ERR("Windows API error.", dlgErrorCode, 0);
			goto EXIT;
		}
	}
	else {
		//新しい色が選択された場合
		*pNewColor = D3DXCOLOR(
							GetRValue(cc.rgbResult) / 255.0f,
							GetGValue(cc.rgbResult) / 255.0f,
							GetBValue(cc.rgbResult) / 255.0f,
							1.0f
						);
		*pIsChoosed = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// グラデーションカラー設定
//******************************************************************************
int MTColorPaletteCfgDlg::_SetGradationColor(
		unsigned long chNoStart,
		unsigned long chNoEnd,
		D3DXCOLOR colorStart,
		D3DXCOLOR colorEnd
	)
{
	int result = 0;
	unsigned int chNo = 0;
	D3DXCOLOR color;
	float ratio = 0.0f;

	if ((chNoStart >= SM_MAX_CH_NUM) || (chNoEnd >= SM_MAX_CH_NUM)) {
		//何もしない
	}

	if (chNoStart == chNoEnd) {
		//何もしない
	}
	else if (chNoStart < chNoEnd) {
		for (chNo = chNoStart; chNo <= chNoEnd; chNo++) {
			ratio = (float)(chNo - chNoStart) / (float)(chNoEnd - chNoStart);
			color = D3DXCOLOR((colorEnd.r - colorStart.r) * ratio + colorStart.r,
				(colorEnd.g - colorStart.g) * ratio + colorStart.g,
				(colorEnd.b - colorStart.b) * ratio + colorStart.b,
				1.0f);
			//カラー設定
			m_ColorPalette.SetChColor(chNo, color);
		}
	}
	else {
		for (chNo = chNoEnd; chNo <= chNoStart; chNo++) {
			ratio = (float)(chNo - chNoEnd) / (float)(chNoStart - chNoEnd);
			color = D3DXCOLOR((colorStart.r - colorEnd.r) * ratio + colorEnd.r,
				(colorStart.g - colorEnd.g) * ratio + colorEnd.g,
				(colorStart.b - colorEnd.b) * ratio + colorEnd.b,
				1.0f);
			//カラー設定
			m_ColorPalette.SetChColor(chNo, color);
		}
	}

	//カラーボタン更新
	result = _UpdateColorButtons();
	if (result != 0) goto EXIT;

	//カラーテキスト更新
	result = _UpdateColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 出力用パラメータ文字列生成
//******************************************************************************
int MTColorPaletteCfgDlg::_MakeColorParamForExport(TCHAR* pTextBuf, unsigned long bufSize)
{
	int result = 0;
	unsigned long chNo = 0;
	D3DXCOLOR color;
	TCHAR hexColor[16] = { _T('\0') };
	TCHAR line[64] = {_T('\0')};

	if ((pTextBuf == NULL) || (bufSize < 2048)) {
		result = YN_SET_ERR("Program error.", bufSize, 0);
		goto EXIT;
	}

	pTextBuf[0] = _T('\0');

	//パラメータ文字列生成：Ch.1-16
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		m_ColorPalette.GetChColor(chNo, &color);
		DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
		_stprintf_s(line, 64, _T("Ch-%02u-NoteRGBA=%s\r\n"), chNo + 1, hexColor);
		_tcscat_s(pTextBuf, bufSize, line);
	}

	//パラメータ文字列生成：背景
	m_ColorPalette.GetBackgroundColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	_stprintf_s(line, 64, _T("BackGroundRGBA=%s\r\n"), hexColor);
	_tcscat_s(pTextBuf, bufSize, line);

	//パラメータ文字列生成：グリッドライン
	m_ColorPalette.GetGridLineColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	_stprintf_s(line, 64, _T("GridLineRGBA=%s\r\n"), hexColor);
	_tcscat_s(pTextBuf, bufSize, line);

	//パラメータ文字列生成：カウンター
	m_ColorPalette.GetCounterColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	_stprintf_s(line, 64, _T("CounterRGBA=%s\r\n"), hexColor);
	_tcscat_s(pTextBuf, bufSize, line);

EXIT:;
	return result;
}

//******************************************************************************
// パラメータ入力処理
//******************************************************************************
int MTColorPaletteCfgDlg::_ImportColorParam(TCHAR* pParamString)
{
	int result = 0;
	MTColorParamDictionary paramDictionary;

	//パラメータマップ作成
	result = _MakeImportKeyValueMap(pParamString, &paramDictionary);
	if (result != 0) goto EXIT;

	//パラメータ読み込み
	result = _LoadParam(&paramDictionary);
	if (result != 0) goto EXIT;

	//カラーボタン更新
	result = _UpdateColorButtons();
	if (result != 0) goto EXIT;

	//カラーテキスト更新
	result = _UpdateColorText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// パラメータマップ作成
//******************************************************************************
int MTColorPaletteCfgDlg::_MakeImportKeyValueMap(
		TCHAR* pParamString,
		MTColorParamDictionary* pParamDictionary
	)
{
	int result = 0;
	std::string paramString = pParamString;
	std::stringstream stream{ paramString };
	std::string line;
	std::string trimLine;
	std::string key;
	std::string value;
	std::string::size_type trimLeft;
	std::string::size_type trimRight;
	TCHAR* trimCharList = _T(" \t\r\n");
	MTColorParamDictionary::iterator itr;

	//各行を解析
	while (std::getline(stream, line, '\n')) {
		//前後の空白と改行をトリミングする
		trimLeft = line.find_first_not_of(trimCharList);
		trimRight = line.find_last_not_of(trimCharList);

		//トリミングすると何も残らない場合は次行へ進む
		if ((trimLeft == std::string::npos) || (trimRight == std::string::npos)) {
			continue;
		}

		//論理エラーを検出
		if (trimLeft > trimRight) {
			result = YN_SET_ERR("Program error.", trimLeft, trimRight);
			goto EXIT;
		}

		//コメント行を無視する
		trimLine = line.substr(trimLeft, trimRight - trimLeft + 1);
		if ((trimLine.front() == _T('#')) || (trimLine.front() == _T(';'))) {
			//コメント行として無視して次行へ進む
			continue;
		}

		//キーを識別
		trimLeft = trimLine.find_first_of(_T("="));
		if ((trimLeft == std::string::npos) || (trimLeft == 0)) {
			//デリミタが見つからない、またはデリミタが先頭のため、無視して次行へ進む
			continue;
		}
		key = trimLine.substr(0, trimLeft);

		//値を識別
		if ((trimLeft + 1) == trimLine.length()) {
			//デリミタが末尾にあるなら値は空文字
			value = _T("");
		}
		else {
			//デリミタより後ろを値とする
			value = trimLine.substr(trimLeft + 1, trimLine.length() - trimLeft + 1);
			//値の先頭末尾がクオーテーションで囲まれている場合はクオーテーションを取り除く
			//デリミタとクオーテーションの間に空白がある場合は空白を値とみなす
			if (value.length() >= 2) {
				if ((value.front() == _T('\'')) && (value.back() == _T('\''))) {
					value = value.substr(1, value.length() - 2);
				}
				else if ((value.front() == _T('\"')) && (value.back() == _T('\"'))) {
					value = value.substr(1, value.length() - 2);
				}
			}
		}

		//マップ登録
		//すでにキーが存在する場合は既存データを削除
		itr = pParamDictionary->find(key);
		if (itr != pParamDictionary->end()) {
			pParamDictionary->erase(itr);
		}
		//データ登録
		pParamDictionary->insert(MTColorParamDictionaryPair(key, value));

		//OutputDebugString(_T("key:"));
		//OutputDebugString(key.c_str());
		//OutputDebugString(_T("\n"));
		//OutputDebugString(_T("value:"));
		//OutputDebugString(value.c_str());
		//OutputDebugString(_T("\n"));
	}

EXIT:;
	return result;
}

//******************************************************************************
// パラメータ読み込み
//******************************************************************************
int MTColorPaletteCfgDlg::_LoadParam(MTColorParamDictionary* pParamDictionary)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR hexColor[16] = { _T('\0') };
	TCHAR key[32] = { _T('\0') };
	const TCHAR* pValue = NULL;
	MTColorParamDictionary::iterator itr;
	D3DXCOLOR color;
	D3DCOLOR colorRGB;

	//Ch.1-16
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo + 1);
		itr = pParamDictionary->find(key);
		if (itr != pParamDictionary->end()) {
			pValue = (itr->second).c_str();
			if (_tcslen(pValue) == 8) {
				color = DXColorUtil::MakeColorFromHexRGBA(pValue);
				m_ColorPalette.SetChColor(chNo, color);
			}
		}
	}

	//背景
	itr = pParamDictionary->find(_T("BackGroundRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetBackgroundColor(color);
		}
	}
	//互換性を保つため ini ファイルに定義されている"BackGroundRGB"（Aなし）をインポート可能とする
	itr = pParamDictionary->find(_T("BackGroundRGB"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 6) {
			colorRGB = DXColorUtil::MakeColorFromHexRGB(pValue);
			color = D3DXCOLOR(colorRGB);
			m_ColorPalette.SetBackgroundColor(color);
		}
	}

	//グリッドライン
	itr = pParamDictionary->find(_T("GridLineRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetGridLineColor(color);
		}
	}

	//カウンタ
	itr = pParamDictionary->find(_T("CounterRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetCounterColor(color);
		}
	}
	//互換性を保つため ini ファイルに定義されている"CaptionRGBA"をインポート可能とする
	itr = pParamDictionary->find(_T("CaptionRGBA"));
	if (itr != pParamDictionary->end()) {
		pValue = (itr->second).c_str();
		if (_tcslen(pValue) == 8) {
			color = DXColorUtil::MakeColorFromHexRGBA(pValue);
			m_ColorPalette.SetCounterColor(color);
		}
	}

//EXIT:;
	return result;
}


