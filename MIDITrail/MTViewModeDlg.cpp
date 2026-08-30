//******************************************************************************
//
// MIDITrail / MTViewModeDlg
//
// ビューモード選択ダイアログクラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "MTParam.h"
#include "MTDlgLib.h"
#include "MTViewModeDlg.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTViewModeDlg* MTViewModeDlg::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTViewModeDlg::MTViewModeDlg(void)
{
	m_pThis = this;
	m_hWnd = NULL;
	m_SceneType = PianoRoll3D;
	m_isCanceled = false;

	m_hCheckPianoRoll3D = NULL;
	m_hCheckPianoRoll2D = NULL;
	m_hCheckPianoRollRain = NULL;
	m_hCheckPianoRollRain2D = NULL;
	m_hCheckPianoRollRing = NULL;

	m_hBtnImgPianoRoll3D = NULL;
	m_hBtnImgPianoRoll2D = NULL;
	m_hBtnImgPianoRollRain = NULL;
	m_hBtnImgPianoRollRain2D = NULL;
	m_hBtnImgPianoRollRing = NULL;

	m_hImgCheckmark = NULL;
	m_hImgNoCheckmark = NULL;

	m_hImgPianoRoll3D = NULL;
	m_hImgPianoRoll2D = NULL;
	m_hImgPianoRollRain = NULL;
	m_hImgPianoRollRain2D = NULL;
	m_hImgPianoRollRing = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTViewModeDlg::~MTViewModeDlg(void)
{
	_ReleaseBitmap(m_hImgCheckmark);
	_ReleaseBitmap(m_hImgNoCheckmark);
	_ReleaseBitmap(m_hImgPianoRoll3D);
	_ReleaseBitmap(m_hImgPianoRoll2D);
	_ReleaseBitmap(m_hImgPianoRollRain);
	_ReleaseBitmap(m_hImgPianoRollRain2D);
	_ReleaseBitmap(m_hImgPianoRollRing);
}

//******************************************************************************
// シーン種別登録（ビューモード）
//******************************************************************************
void MTViewModeDlg::SetSceneType(SceneType type)
{
	m_SceneType = type;
}

//******************************************************************************
// シーン種別取得（ビューモード）
//******************************************************************************
SceneType MTViewModeDlg::GetSceneType()
{
	return m_SceneType;
}

//******************************************************************************
// キャンセルフラグ参照
//******************************************************************************
bool MTViewModeDlg::IsCanceled()
{
	return m_isCanceled;
}

//******************************************************************************
// 表示
//******************************************************************************
int MTViewModeDlg::Show(
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
					MAKEINTRESOURCE(IDD_VIEWMODE),		//ダイアログボックステンプレート
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
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTViewModeDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTViewModeDlg::_WndProcImpl(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	BOOL bresult = TRUE;
	HDC hdc = NULL;
	PAINTSTRUCT ps;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG:
			result = _OnInitDlg(hDlg);
			if (result != 0) goto EXIT;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_BTN_IMG_PIANO_ROLL_3D:
					result = _OnButtonPianoRoll3D();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PIANO_ROLL_3D:
					result = _OnButtonPianoRoll3D();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_IMG_PIANO_ROLL_2D:
					result = _OnButtonPianoRoll2D();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PIANO_ROLL_2D:
					result = _OnButtonPianoRoll2D();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_IMG_PIANO_ROLL_RAIN:
					result = _OnButtonPianoRollRain();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PIANO_ROLL_RAIN:
					result = _OnButtonPianoRollRain();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_IMG_PIANO_ROLL_RAIN_2D:
					result = _OnButtonPianoRollRain2D();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PIANO_ROLL_RAIN_2D:
					result = _OnButtonPianoRollRain2D();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_IMG_PIANO_ROLL_RING:
					result = _OnButtonPianoRollRing();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PIANO_ROLL_RING:
					result = _OnButtonPianoRollRing();
					if (result != 0) goto EXIT;
					break;
				case IDOK:
					result = _OnOK();
					if (result != 0) goto EXIT;
					break;
				case IDCANCEL:
					result = _OnCancel();
					if (result != 0) goto EXIT;
					break;
				default:
					bresult = FALSE;
					break;
			}
			break;
		case WM_PAINT:
			//描画
			hdc = BeginPaint(hDlg, &ps);
			EndPaint(hDlg, &ps);
			break;
		default:
			bresult = FALSE;
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(hDlg);
	}
	return (INT_PTR)bresult;
}

//******************************************************************************
// ダイアログ表示直前初期化
//******************************************************************************
int MTViewModeDlg::_OnInitDlg(
		HWND hDlg
	)
{
	int result = 0;

	m_hWnd = hDlg;

	//ボタン初期化
	result = _InitButtons();
	if (result != 0) goto EXIT;

	//親ウィンドウの中央に表示（スクリーン内側）
	MTDlgLib::SetWindowPositionToCenter(hDlg, true);

EXIT:;
	return result;
}

//******************************************************************************
// ボタン群初期化
//******************************************************************************
int MTViewModeDlg::_InitButtons()
{
	int result = 0;

	//ボタン画像初期化の戻り値は無視する
	//ボタン画像初期化が失敗した場合はボタンに画像を反映しない

	//チェックマーク領域のハンドル
	m_hCheckPianoRoll3D = GetDlgItem(m_hWnd, IDC_CHECK_PIANO_ROLL_3D);
	m_hCheckPianoRoll2D = GetDlgItem(m_hWnd, IDC_CHECK_PIANO_ROLL_2D);
	m_hCheckPianoRollRain = GetDlgItem(m_hWnd, IDC_CHECK_PIANO_ROLL_RAIN);
	m_hCheckPianoRollRain2D = GetDlgItem(m_hWnd, IDC_CHECK_PIANO_ROLL_RAIN_2D);
	m_hCheckPianoRollRing = GetDlgItem(m_hWnd, IDC_CHECK_PIANO_ROLL_RING);

	//チェックマーク画像生成
	_CreateButtonImage(m_hCheckPianoRoll3D, L"Button-Checkmark@2x.png", &m_hImgCheckmark);
	_CreateButtonImage(m_hCheckPianoRoll3D, L"Button-NoCheckmark@2x.png", &m_hImgNoCheckmark);

	//ビューモード画像ボタンのハンドル
	m_hBtnImgPianoRoll3D = GetDlgItem(m_hWnd, IDC_BTN_IMG_PIANO_ROLL_3D);
	m_hBtnImgPianoRoll2D = GetDlgItem(m_hWnd, IDC_BTN_IMG_PIANO_ROLL_2D);
	m_hBtnImgPianoRollRain = GetDlgItem(m_hWnd, IDC_BTN_IMG_PIANO_ROLL_RAIN);
	m_hBtnImgPianoRollRain2D = GetDlgItem(m_hWnd, IDC_BTN_IMG_PIANO_ROLL_RAIN_2D);
	m_hBtnImgPianoRollRing = GetDlgItem(m_hWnd, IDC_BTN_IMG_PIANO_ROLL_RING);

	//ビューモード画像作成
	_CreateButtonImage(m_hBtnImgPianoRoll3D, L"ViewMode-PianoRoll3D@2x.png", &m_hImgPianoRoll3D);
	_CreateButtonImage(m_hBtnImgPianoRoll2D, L"ViewMode-PianoRoll2D@2x.png", &m_hImgPianoRoll2D);
	_CreateButtonImage(m_hBtnImgPianoRollRain, L"ViewMode-PianoRollRain@2x.png", &m_hImgPianoRollRain);
	_CreateButtonImage(m_hBtnImgPianoRollRain2D, L"ViewMode-PianoRollRain2D@2x.png", &m_hImgPianoRollRain2D);
	_CreateButtonImage(m_hBtnImgPianoRollRing, L"ViewMode-PianoRollRing@2x.png", &m_hImgPianoRollRing);

	//ビューモード画像ボタンに画像登録
	SendMessage(m_hBtnImgPianoRoll3D, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hImgPianoRoll3D);
	SendMessage(m_hBtnImgPianoRoll2D, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hImgPianoRoll2D);
	SendMessage(m_hBtnImgPianoRollRain, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hImgPianoRollRain);
	SendMessage(m_hBtnImgPianoRollRain2D, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hImgPianoRollRain2D);
	SendMessage(m_hBtnImgPianoRollRing, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hImgPianoRollRing);

	//チェックマーク更新
	_UpdateCheckMarks();

	return result;
}

//******************************************************************************
// ボタン画像初期化
//******************************************************************************
int MTViewModeDlg::_CreateButtonImage(
		HWND hButton,
		WCHAR* pImgFileName,
		HBITMAP* phBitmap
	)
{
	int result = 0;
	WCHAR imgFilePath[_MAX_PATH] = { L'\0' };
	unsigned long height = 0;
	unsigned long width = 0;
	Gdiplus::Status status = Gdiplus::Status::Ok;
	Gdiplus::Bitmap* pSrcBitmap = NULL;
	Gdiplus::Bitmap* pDestBitmap = NULL;
	Gdiplus::Graphics* pGraphics = NULL;
	HBITMAP hDestBitmap = NULL;
	COLORREF dlgBkColor;
	Gdiplus::Color bkColor;

	*phBitmap = NULL;

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPathW(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//画像ファイルパス作成
	wcscat_s(imgFilePath, _MAX_PATH, MT_IMG_VIEWMODE_DIR);
	wcscat_s(imgFilePath, _MAX_PATH, pImgFileName);

	//画像読み込み
	result = _LoadImage((void**)&pSrcBitmap, imgFilePath);
	if (result != 0) goto EXIT;

	//ボタンサイズを取得
	result = _GetWindowSize(hButton, &width, &height);
	if (result != 0) goto EXIT;

	//ボタンサイズに合わせた空のビットマップを作製
	pDestBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (pDestBitmap == NULL) {
		result = YN_SET_ERR("Windows API Error.", width, height);
		goto EXIT;
	}

	//変換後ビットマップのグラフィックオブジェクトを作成
	pGraphics = Gdiplus::Graphics::FromImage(pDestBitmap);
	if (pGraphics == NULL) {
		result = YN_SET_ERR("Windows API Error.", 0, 0);
		goto EXIT;
	}

	//補間モードを設定
	pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

	//読み込んだ画像を変換後ビットマップに描画
	status = pGraphics->DrawImage(pSrcBitmap, 0, 0, width, height);
	if (status != Gdiplus::Status::Ok) {
		result = YN_SET_ERR("Windows API Error.", status, 0);
		goto EXIT;
	}

	//ダイアログボックス背景色を取得
	dlgBkColor = GetSysColor(COLOR_3DFACE);
	bkColor = Gdiplus::Color(
					GetRValue(dlgBkColor),
					GetGValue(dlgBkColor),
					GetBValue(dlgBkColor)
				);

	//GDI+のBitmapからHBITMAPに変換
	// 第1引数は背景色（透明な部分を何色で埋めるか。Color(0,0,0,0)は透明）
	status = pDestBitmap->GetHBITMAP(
					bkColor,							//背景色：ダイアログボックス背景色
					&hDestBitmap						//GDIビットマップハンドル受け取り
				);
	if (status != Gdiplus::Status::Ok) {
		result = YN_SET_ERR("Windows API Error.", status, 0);
		goto EXIT;
	}
	else if (hDestBitmap == NULL) {
		result = YN_SET_ERR("Windows API Error.", 0, 0);
		goto EXIT;
	}

	*phBitmap = hDestBitmap;

EXIT:;
	if (pGraphics != NULL) {
		delete pGraphics;
	}
	if (pDestBitmap != NULL) {
		delete pDestBitmap;
	}
	if (pSrcBitmap != NULL) {
		delete pSrcBitmap;
	}
	return result;
}

//******************************************************************************
// ウィンドウサイズ取得
//******************************************************************************
int MTViewModeDlg::_GetWindowSize(
		HWND hWnd,
		unsigned long* pWidth,
		unsigned long* pHeight
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	RECT rect;

	bresult = GetClientRect(hWnd, &rect);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	*pHeight = rect.bottom - rect.top;
	*pWidth = rect.right - rect.left;

EXIT:;
	return result;
}

//******************************************************************************
// 画像読み込み
//******************************************************************************
int MTViewModeDlg::_LoadImage(
		void** pPtrBitmap,
		WCHAR* pFilePath
	)
{
	int result = 0;
	Gdiplus::Bitmap* pBitmap = NULL;
	
	*pPtrBitmap = NULL;
	
	pBitmap = Gdiplus::Bitmap::FromFile(pFilePath);
	if (pBitmap == NULL) {
		result = YN_SET_ERR("Image file loading error.", GetLastError(), 0);
		goto EXIT;
	}
	else if (pBitmap->GetLastStatus() != Gdiplus::Status::Ok) {
		result = YN_SET_ERR("Image file loading error.", pBitmap->GetLastStatus(), 0);
		goto EXIT;
	}
	
	*pPtrBitmap = (void*)pBitmap;
	
EXIT:;
	return result;
}

//******************************************************************************
// 画像破棄
//******************************************************************************
void MTViewModeDlg::_ReleaseBitmap(HBITMAP hBitmap)
{
	if (hBitmap != NULL) {
		DeleteObject(hBitmap);
	}
	return;
}

//******************************************************************************
// チェックマーク更新
//******************************************************************************
void MTViewModeDlg::_UpdateCheckMarks()
{
	_SetCheckMark(m_hCheckPianoRoll3D, (m_SceneType == PianoRoll3D));
	_SetCheckMark(m_hCheckPianoRoll2D, (m_SceneType == PianoRoll2D));
	_SetCheckMark(m_hCheckPianoRollRain, (m_SceneType == PianoRollRain));
	_SetCheckMark(m_hCheckPianoRollRain2D, (m_SceneType == PianoRollRain2D));
	_SetCheckMark(m_hCheckPianoRollRing, (m_SceneType == PianoRollRing));
}

//******************************************************************************
// チェックマーク画像設定
//******************************************************************************
void MTViewModeDlg::_SetCheckMark(
		HWND hWnd,
		bool isCheck
	)
{
	if (isCheck) {
		SendMessage(hWnd, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hImgCheckmark);
	}
	else {
		SendMessage(hWnd, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hImgNoCheckmark);
	}
}

//******************************************************************************
// ボタン押下：Piano Roll 3D
//******************************************************************************
int MTViewModeDlg::_OnButtonPianoRoll3D()
{
	int result = 0;

	m_SceneType = PianoRoll3D;
	_UpdateCheckMarks();

	return result;
}

//******************************************************************************
// ボタン押下：Piano Roll 2D
//******************************************************************************
int MTViewModeDlg::_OnButtonPianoRoll2D()
{
	int result = 0;

	m_SceneType = PianoRoll2D;
	_UpdateCheckMarks();

	return result;
}

//******************************************************************************
// ボタン押下：Piano Roll Rain
//******************************************************************************
int MTViewModeDlg::_OnButtonPianoRollRain()
{
	int result = 0;

	m_SceneType = PianoRollRain;
	_UpdateCheckMarks();

	return result;
}

//******************************************************************************
// ボタン押下：Piano Roll Rain 2D
//******************************************************************************
int MTViewModeDlg::_OnButtonPianoRollRain2D()
{
	int result = 0;

	m_SceneType = PianoRollRain2D;
	_UpdateCheckMarks();

	return result;
}

//******************************************************************************
// ボタン押下：Piano Roll Ring
//******************************************************************************
int MTViewModeDlg::_OnButtonPianoRollRing()
{
	int result = 0;

	m_SceneType = PianoRollRing;
	_UpdateCheckMarks();

	return result;
}

//******************************************************************************
// OKボタン押下
//******************************************************************************
int MTViewModeDlg::_OnOK()
{
	int result = 0;

	m_isCanceled = false;
	EndDialog(m_hWnd, LOWORD(IDOK));

	return result;
}

//******************************************************************************
// Cancelボタン押下：またはESCキー押下
//******************************************************************************
int MTViewModeDlg::_OnCancel()
{
	int result = 0;

	m_isCanceled = true;
	EndDialog(m_hWnd, LOWORD(IDCANCEL));

	return result;
}



