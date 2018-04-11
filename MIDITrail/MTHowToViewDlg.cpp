//******************************************************************************
//
// MIDITrail / MTHowToViewDlg
//
// 操作方法ダイアログ
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTHowToViewDlg.h"

using namespace YNBaseLib;

//******************************************************************************
// パラメータ定義
//******************************************************************************
//表示画像数
#define MT_HOWTOVIEW_IMAGE_NUM  (3)

//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTHowToViewDlg* MTHowToViewDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTHowToViewDlg::MTHowToViewDlg(void)
{
	m_pThis = this;
	m_hMemBmpPixel = NULL;
	m_pBmpPixcel = NULL;
	m_PageNo = 0;
	m_hWnd = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTHowToViewDlg::~MTHowToViewDlg(void)
{
	_Clear();
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTHowToViewDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTHowToViewDlg::_WndProcImpl(
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
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_PREVIOUS) {
				result = _OnPreviousButton();
				if (result != 0) goto EXIT;
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_NEXT) {
				result = _OnNextButton();
				if (result != 0) goto EXIT;
				bresult = TRUE;
			}
			else if (LOWORD(wParam) == IDC_BTN_CLOSE) {
				EndDialog(hDlg, LOWORD(wParam));
				bresult = TRUE;
			}
			break;
		case WM_PAINT:
			result = _DrawHowToBmp();
			if (result != 0) goto EXIT;
			bresult = TRUE;
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
int MTHowToViewDlg::Show(
		HWND hParentWnd
	)
{
	int result = 0;
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;

	m_PageNo = 0;

	//アプリケーションインスタンスハンドルを取得
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hParentWnd);
		goto EXIT;
	}

	//ダイアログ表示
	dresult = DialogBox(
					hInstance,							//インスタンスハンドル
					MAKEINTRESOURCE(IDD_HOWTOVIEW),		//ダイアログボックステンプレート
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
int MTHowToViewDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_hWnd = hDlg;

	//HowToビットマップ読み込み
	result = _LoadHowToBmp();
	if (result != 0) goto EXIT;

	//ボタン状態更新
	_UpdateButtonStatus();

EXIT:;
	return result;
}

//******************************************************************************
// HowToビットマップ読み込み
//******************************************************************************
int MTHowToViewDlg::_LoadHowToBmp()
{
	int result = 0;
	HANDLE hFile = NULL;
	BOOL bresult = FALSE;
	DWORD numOfBytesRead = 0;
	DWORD fp = 0;
	HANDLE hMemBmpPixel = NULL;
	BYTE* pBmpPixcel = NULL;
	TCHAR bmpFilePath[_MAX_PATH] = {_T('\0')};
	TCHAR* pBmpFileName[3] = { MT_IMGFILE_HOWTOVIEW1, MT_IMGFILE_HOWTOVIEW2, MT_IMGFILE_HOWTOVIEW3 };

	_Clear();

	//----------------------------------------------------------------
	//ファイルオープン
	//----------------------------------------------------------------
	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPath(bmpFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	if (m_PageNo >= MT_HOWTOVIEW_IMAGE_NUM) {
		result = YN_SET_ERR("Program error.", m_PageNo, 0);
		goto EXIT;
	}

	//BMPファイルパス作成
	_tcscat_s(bmpFilePath, _MAX_PATH, pBmpFileName[m_PageNo]);

	//BMPファイルを開く
	hFile = CreateFile(
				bmpFilePath,			//ファイルパス
				GENERIC_READ,			//アクセスタイプ
				0,						//共有方法
				NULL,					//セキュリティ属性
				OPEN_EXISTING,			//生成指定
				FILE_ATTRIBUTE_NORMAL,	//ファイル属性とフラグ
				NULL					//テンプレートファイルハンドル
			);
	if (hFile == INVALID_HANDLE_VALUE) {
		result = YN_SET_ERR("File open error.", GetLastError(), 0);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMPファイルヘッダ
	//----------------------------------------------------------------
	//BMPファイルヘッダ読み込み
	bresult = ReadFile(
					hFile,							//ファイルハンドル
					(LPBITMAPFILEHEADER)&m_BmpHead,	//バッファ位置
					sizeof(BITMAPFILEHEADER),		//バッファサイズ
					&numOfBytesRead,				//読み取りサイズ
					NULL							//オーバーラップ構造体バッファ
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	//ファイルタイプの確認 "BM"
	if (m_BmpHead.bfType != 0x4D42) {
		result = YN_SET_ERR("Invalid data found.", m_BmpHead.bfType, 0);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMP情報ヘッダ
	//----------------------------------------------------------------
	//BMP情報ヘッダ読み込み
	bresult = ReadFile(
					hFile,							//ファイルハンドル
					(LPBITMAPINFOHEADER)&m_BmpInfo,	//バッファ位置
					sizeof(BITMAPINFOHEADER),		//バッファサイズ
					&numOfBytesRead,				//読み取りサイズ
					NULL							//オーバーラップ構造体バッファ
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	//24bit画像以外は読みません
	if ((m_BmpInfo.biBitCount != 24) || (m_BmpInfo.biClrUsed != 0)) {
		result = YN_SET_ERR("Invalid BMP file.", m_BmpInfo.biBitCount, m_BmpInfo.biClrUsed);
		goto EXIT;
	}

	//----------------------------------------------------------------
	//BMPピクセルデータ
	//----------------------------------------------------------------
	//ピクセルデータ開始位置にファイルポインタを設定
	fp = SetFilePointer(
				hFile,					//ファイルハンドル
				m_BmpHead.bfOffBits,	//ファイルポインタ移動バイト数：下位32bit
				0,						//ファイルポインタ移動バイト数：上位32bit
				FILE_BEGIN				//開始点
			);
	if (fp == INVALID_SET_FILE_POINTER) {
		result = YN_SET_ERR("File access error.", GetLastError(), m_BmpHead.bfOffBits);
		goto EXIT;
	}

	//ピクセルデータ読み込み用メモリ確保
	hMemBmpPixel = GlobalAlloc(GHND, m_BmpHead.bfSize);
	if (hMemBmpPixel == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	pBmpPixcel = (BYTE*)GlobalLock(hMemBmpPixel);
	if (pBmpPixcel == NULL) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//BMPピクセルデータ読み込み
	bresult = ReadFile(
					hFile,				//ファイルハンドル
					pBmpPixcel,			//バッファ位置
					m_BmpHead.bfSize,	//バッファサイズ
					&numOfBytesRead,	//読み取りサイズ
					NULL				//オーバーラップ構造体バッファ
				);
	if (!bresult) {
		result = YN_SET_ERR("File read error.", GetLastError(), 0);
		goto EXIT;
	}

	m_hMemBmpPixel = hMemBmpPixel;
	m_pBmpPixcel = pBmpPixcel;

EXIT:;
	if (hFile != NULL) {
		CloseHandle(hFile);
	}
	if (result != 0) {
		if (hMemBmpPixel != NULL) {
			GlobalUnlock(hMemBmpPixel);
			GlobalFree(hMemBmpPixel);
		}
	}
	return result;
}

//******************************************************************************
// HowToビットマップ描画
//******************************************************************************
int MTHowToViewDlg::_DrawHowToBmp()
{
	int result = 0;
	int apiresult = 0;
	HDC hdc = NULL;
	HWND hWndPicture = NULL;
	PAINTSTRUCT ps;

	if (m_pBmpPixcel == NULL) goto EXIT;

	//描画対象ウィンドウハンドル取得
	hWndPicture = GetDlgItem(m_hWnd, IDC_HOWTO_PICTURE);

	//描画準備
	hdc = BeginPaint(hWndPicture, &ps);

	apiresult = SetDIBitsToDevice(
					hdc,					//デバイスコンテキストハンドル
					0,						//転送先左上隅の座標：x
					0,						//転送先左上隅の座標：y
					m_BmpInfo.biWidth,		//転送元サイズ：幅
					m_BmpInfo.biHeight,		//転送元サイズ：高さ
					0,						//転送元座標左下隅の座標：x
					0,						//転送元座標左下隅の座標：y
					0,						//走査開始行
					m_BmpInfo.biHeight,		//走査行数
					m_pBmpPixcel,			//ビットマップデータ開始のアドレス
					(BITMAPINFO*)&m_BmpInfo,//BMP情報ヘッダ
					DIB_RGB_COLORS			//色指定
				);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//描画終了
	EndPaint(hWndPicture, &ps);

EXIT:;
	return result;
}

//******************************************************************************
// クリア
//******************************************************************************
void MTHowToViewDlg::_Clear()
{
	ZeroMemory(&m_BmpHead, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&m_BmpInfo, sizeof(BITMAPINFO));

	if (m_hMemBmpPixel != NULL) {
		GlobalUnlock(m_hMemBmpPixel);
		GlobalFree(m_hMemBmpPixel);
		m_hMemBmpPixel = NULL;
	}
	m_pBmpPixcel = NULL;
}

//******************************************************************************
// 前ボタン押下
//******************************************************************************
int MTHowToViewDlg::_OnPreviousButton()
{
	int result = 0;

	//前の画像へ移動
	m_PageNo--;

	//念のためガードする
	if (m_PageNo < 0) {
		m_PageNo = 0;
	}

	//ボタン状態更新
	_UpdateButtonStatus();

	//画像表示
	result = _DrawHowToImage();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 次ボタン押下
//******************************************************************************
int MTHowToViewDlg::_OnNextButton()
{
	int result = 0;

	//次の画像へ移動
	m_PageNo++;

	//念のためガードする
	if (m_PageNo >= MT_HOWTOVIEW_IMAGE_NUM) {
		m_PageNo = MT_HOWTOVIEW_IMAGE_NUM - 1;
	}

	//ボタン状態更新
	_UpdateButtonStatus();

	//画像表示
	result = _DrawHowToImage();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 画像表示
//******************************************************************************
int MTHowToViewDlg::_DrawHowToImage()
{
	int result = 0;

	//HowToビットマップ読み込み
	result = _LoadHowToBmp();
	if (result != 0) goto EXIT;

	//再描画
	InvalidateRect(
			m_hWnd,	//ウィンドウハンドル
			NULL,	//更新リージョン指定：全体
			FALSE	//背景消去：なし
		);
	UpdateWindow(m_hWnd);

EXIT:;
	return result;
}

//******************************************************************************
// ボタン状態更新
//******************************************************************************
void MTHowToViewDlg::_UpdateButtonStatus()
{
	HWND hPreviousButton = NULL;
	HWND hNextButton = NULL;

	hPreviousButton = GetDlgItem(m_hWnd, IDC_BTN_PREVIOUS);
	hNextButton = GetDlgItem(m_hWnd, IDC_BTN_NEXT);

	EnableWindow(hPreviousButton, TRUE);
	EnableWindow(hNextButton, TRUE);

	//先頭画像表示：前ボタン不活性
	if (m_PageNo == 0) {
		EnableWindow(hPreviousButton, FALSE);
	}
	//最終画像表示：次ボタン不活性
	if (m_PageNo == (MT_HOWTOVIEW_IMAGE_NUM - 1)) {
		EnableWindow(hNextButton, FALSE);
	}
}


