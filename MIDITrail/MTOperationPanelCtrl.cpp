//******************************************************************************
//
// MIDITrail / MTOperationPanelCtrl
//
// 操作パネル制御
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "resource.h"
#include "MTOperationPanelCtrl.h"
#include <dwmapi.h>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>


//##############################################################################
// パネルボタンクラス
//##############################################################################

//******************************************************************************
// パネルボタン：コンストラクタ
//******************************************************************************
MTOperationPanelCtrl::MTPanelButton::MTPanelButton(void)
{
	m_hPanel = NULL;
	m_hButton = NULL;
	m_hToolTip = NULL;

	m_isEnabled = false;
	m_isSwitchStatus = false;
}

//******************************************************************************
// パネルボタン：デストラクタ
//******************************************************************************
MTOperationPanelCtrl::MTPanelButton::~MTPanelButton(void)
{
}

//******************************************************************************
// パネルボタン：初期化
//******************************************************************************
void MTOperationPanelCtrl::MTPanelButton::Initialize(
		HWND hWnd,
		int buttonId
	)
{
	m_hPanel = hWnd;
	m_hButton = GetDlgItem(hWnd, buttonId);
}

//******************************************************************************
// パネルボタン：活性状態登録
//******************************************************************************
void MTOperationPanelCtrl::MTPanelButton::SetEnabled(bool isEnabled)
{
	m_isEnabled = isEnabled;
}

//******************************************************************************
// パネルボタン：活性状態取得
//******************************************************************************
bool MTOperationPanelCtrl::MTPanelButton::IsEnabled()
{
	return m_isEnabled;
}

//******************************************************************************
// パネルボタン：スイッチ状態登録
//******************************************************************************
void MTOperationPanelCtrl::MTPanelButton::SetSwitchStatus(bool isON)
{
	m_isSwitchStatus = isON;
}

//******************************************************************************
// パネルボタン：スイッチ状態取得
//******************************************************************************
bool MTOperationPanelCtrl::MTPanelButton::IsSwitchStatus()
{
	return m_isSwitchStatus;
}

//******************************************************************************
// パネルボタン：サイズ取得
//******************************************************************************
int MTOperationPanelCtrl::MTPanelButton::GetSize(
		unsigned long* pWidth,
		unsigned long* pHeight
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	RECT rect;

	bresult = GetClientRect(m_hButton, &rect);
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
// パネルボタン：画像登録
//******************************************************************************
void MTOperationPanelCtrl::MTPanelButton::SetImage(HBITMAP hBitmap)
{
	//ボタンにビットマップを設定
	//戻り値は以前にボタンに関連付けられた画像のハンドルのため無視する
	if (hBitmap != NULL) {
		SendMessage(m_hButton, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
	}

	return;
}

//******************************************************************************
// パネルボタン：ツールチップ作成
//******************************************************************************
int MTOperationPanelCtrl::MTPanelButton::CreateToolTip(LPTSTR pszText)
{
	int result = 0;
	TOOLINFO ti;

	if (m_hButton == NULL) {
		goto EXIT;
	}

	//ツールチップウィンドウ作成
	m_hToolTip = CreateWindowEx(
						WS_EX_TOPMOST,				//拡張ウィンドウスタイル：最上位以外のすべてのウィンドウの上に配置
						TOOLTIPS_CLASS,				//ウィンドウクラス名
						NULL,						//ウィンドウ名
						WS_POPUP | TTS_ALWAYSTIP,	//ウィンドウスタイル：ポップアップ ウィンドウ、
													//  所有者ウィンドウが非アクティブでもマウスオーバー時に表示
						CW_USEDEFAULT,				//ウィンドウ位置X：デフォルト
						CW_USEDEFAULT,				//ウィンドウ位置Y：デフォルト
						CW_USEDEFAULT,				//ウィンドウ幅   ：デフォルト
						CW_USEDEFAULT,				//ウィンドウ高さ ：デフォルト
						m_hPanel,					//親ウィンドウまたは所有者ウィンドウのハンドル
						NULL,						//メニューハンドル
						GetModuleHandle(NULL),		//モジュールインスタンスハンドル
						NULL						//ウィンドウ作成データ
					);
	if (m_hToolTip == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//ツール情報
	ZeroMemory(&ti, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);				//構造体のサイズ
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;	//ヒント表示制御フラグ：メッセージ転送自動化、uIdはツールのウィンドウハンドル
	ti.hwnd = m_hPanel;							//ツール（ボタン）を含むウィンドウのハンドル
	ti.uId = (UINT_PTR)m_hButton;				//ツール（ボタン）のウィンドウハンドル
	ti.lpszText = pszText;						//表示テキスト

	//ツールウィンドウにツール情報を登録
	SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

EXIT:;
	return result;
}


//##############################################################################
// 操作パネルクラス
//##############################################################################

//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MTOperationPanelCtrl* MTOperationPanelCtrl::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTOperationPanelCtrl::MTOperationPanelCtrl(void)
{
	m_hInstance = NULL;
	m_hMainWnd = NULL;
	m_hWnd = NULL;
	m_hBackgroundBrush = NULL;
	m_PlayStatus = NoData;
	m_isDisplay = false;
	m_isFullScreen = false;

	m_hImgButtonOpenFileN = NULL;
	m_hImgButtonOpenFileD = NULL;
	m_hImgButtonOpenFolderN = NULL;
	m_hImgButtonOpenFolderD = NULL;
	m_hImgButtonPreviousFileN = NULL;
	m_hImgButtonPreviousFileD = NULL;
	m_hImgButtonNextFileN = NULL;
	m_hImgButtonNextFileD = NULL;
	m_hImgButtonFolderPlaybackN = NULL;
	m_hImgButtonFolderPlaybackD = NULL;
	m_hImgButtonFolderPlaybackS = NULL;
	m_hImgButtonRepeatN = NULL;
	m_hImgButtonRepeatD = NULL;
	m_hImgButtonRepeatS = NULL;
	m_hImgButtonPlayN = NULL;
	m_hImgButtonPlayD = NULL;
	m_hImgButtonPauseN = NULL;
	m_hImgButtonPauseD = NULL;
	m_hImgButtonStopN = NULL;
	m_hImgButtonStopD = NULL;
	m_hImgButtonSkipBackN = NULL;
	m_hImgButtonSkipBackD = NULL;
	m_hImgButtonSkipForwardN = NULL;
	m_hImgButtonSkipForwardD = NULL;
	m_hImgButtonSpeedDownN = NULL;
	m_hImgButtonSpeedDownD = NULL;
	m_hImgButtonSpeedUpN = NULL;
	m_hImgButtonSpeedUpD = NULL;
	m_hImgButtonViewpoint1N = NULL;
	m_hImgButtonViewpoint1D = NULL;
	m_hImgButtonViewpoint2N = NULL;
	m_hImgButtonViewpoint2D = NULL;
	m_hImgButtonViewpoint3N = NULL;
	m_hImgButtonViewpoint3D = NULL;
	m_hImgButtonMyViewpoint1N = NULL;
	m_hImgButtonMyViewpoint1D = NULL;
	m_hImgButtonMyViewpoint2N = NULL;
	m_hImgButtonMyViewpoint2D = NULL;
	m_hImgButtonMyViewpoint3N = NULL;
	m_hImgButtonMyViewpoint3D = NULL;
	m_hImgButtonViewModeN = NULL;
	m_hImgButtonViewModeD = NULL;
	m_hImgButtonMIDIOUTN = NULL;
	m_hImgButtonMIDIOUTD = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTOperationPanelCtrl::~MTOperationPanelCtrl(void)
{
	_ReleaseBitmap(m_hImgButtonOpenFileN);
	_ReleaseBitmap(m_hImgButtonOpenFileD);
	_ReleaseBitmap(m_hImgButtonOpenFolderN);
	_ReleaseBitmap(m_hImgButtonOpenFolderD);
	_ReleaseBitmap(m_hImgButtonPreviousFileN);
	_ReleaseBitmap(m_hImgButtonPreviousFileD);
	_ReleaseBitmap(m_hImgButtonNextFileN);
	_ReleaseBitmap(m_hImgButtonNextFileD);
	_ReleaseBitmap(m_hImgButtonFolderPlaybackN);
	_ReleaseBitmap(m_hImgButtonFolderPlaybackD);
	_ReleaseBitmap(m_hImgButtonFolderPlaybackS);
	_ReleaseBitmap(m_hImgButtonRepeatN);
	_ReleaseBitmap(m_hImgButtonRepeatD);
	_ReleaseBitmap(m_hImgButtonRepeatS);
	_ReleaseBitmap(m_hImgButtonPlayN);
	_ReleaseBitmap(m_hImgButtonPlayD);
	_ReleaseBitmap(m_hImgButtonPauseN);
	_ReleaseBitmap(m_hImgButtonPauseD);
	_ReleaseBitmap(m_hImgButtonStopN);
	_ReleaseBitmap(m_hImgButtonStopD);
	_ReleaseBitmap(m_hImgButtonSkipBackN);
	_ReleaseBitmap(m_hImgButtonSkipBackD);
	_ReleaseBitmap(m_hImgButtonSkipForwardN);
	_ReleaseBitmap(m_hImgButtonSkipForwardD);
	_ReleaseBitmap(m_hImgButtonSpeedDownN);
	_ReleaseBitmap(m_hImgButtonSpeedDownD);
	_ReleaseBitmap(m_hImgButtonSpeedUpN);
	_ReleaseBitmap(m_hImgButtonSpeedUpD);
	_ReleaseBitmap(m_hImgButtonViewpoint1N);
	_ReleaseBitmap(m_hImgButtonViewpoint1D);
	_ReleaseBitmap(m_hImgButtonViewpoint2N);
	_ReleaseBitmap(m_hImgButtonViewpoint2D);
	_ReleaseBitmap(m_hImgButtonViewpoint3N);
	_ReleaseBitmap(m_hImgButtonViewpoint3D);
	_ReleaseBitmap(m_hImgButtonMyViewpoint1N);
	_ReleaseBitmap(m_hImgButtonMyViewpoint1D);
	_ReleaseBitmap(m_hImgButtonMyViewpoint2N);
	_ReleaseBitmap(m_hImgButtonMyViewpoint2D);
	_ReleaseBitmap(m_hImgButtonMyViewpoint3N);
	_ReleaseBitmap(m_hImgButtonMyViewpoint3D);
	_ReleaseBitmap(m_hImgButtonViewModeN);
	_ReleaseBitmap(m_hImgButtonViewModeD);
	_ReleaseBitmap(m_hImgButtonMIDIOUTN);
	_ReleaseBitmap(m_hImgButtonMIDIOUTD);
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTOperationPanelCtrl::Initialize(
		HWND hMainWnd,
		bool isDisplay
	)
{
	int result = 0;
	LONG lresult = 0;
	BOOL bresult = FALSE;
	HINSTANCE hInstance = NULL;
	COLORREF backgroundColor;
	HBRUSH hBrush = NULL;

	m_pThis = this;
	m_hMainWnd = hMainWnd;
	m_isDisplay = isDisplay;

	//背景ブラシ
	backgroundColor = RGB(0, 255, 255);
	m_hBackgroundBrush = CreateSolidBrush(backgroundColor);

	//パネルウィンドウ生成
	m_hWnd = CreateDialog(
					GetModuleHandle(NULL),					//モジュールハンドル
					MAKEINTRESOURCE(IDD_OPERATION_PANEL),	//ダイアログボックステンプレート
					m_hMainWnd,								//親ウィンドウハンドル
					_WndProc								//ウィンドウプロシージャ
				);
	if (m_hWnd == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//レイヤードウィンドウの属性を追加
	lresult = SetWindowLong(
					m_hWnd,				//ウィンドウハンドル
					GWL_EXSTYLE,		//オフセット：拡張ウィンドウ スタイルを更新
					GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED
										//新たに設定する値
				);
	if (lresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//パネル背景色を透明化
	bresult = SetLayeredWindowAttributes(
					m_hWnd,				//ウィンドウハンドル
					backgroundColor,	//カラーキー
					0,					//アルファ値：完全に透明
					LWA_COLORKEY		//フラグ：カラーキーを透明にする
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//ボタン群初期化
	result = _InitButtons();
	if (result != 0) goto EXIT;

	//パネル位置更新
	result = UpdatePanelPosition();
	if (result != 0) goto EXIT;

	SetDisplay(m_isDisplay);

EXIT:;
	return result;
}

//******************************************************************************
// ボタン群初期化
//******************************************************************************
int MTOperationPanelCtrl::_InitButtons()
{
	int result = 0;

	//ボタンオブジェクト初期化
	result = _InitButtonObjects();
	if (result != 0) goto EXIT;

	//ボタン画像初期化
	result = _InitButtonImages();
	if (result != 0) goto EXIT;

	//ツールチップ初期化
	result = _InitButtonToolTips();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ボタンオブジェクト初期化
//******************************************************************************
int MTOperationPanelCtrl::_InitButtonObjects()
{
	int result = 0;

	//ボタンオブジェクト初期化
	m_ButtonOpenFile.Initialize(m_hWnd, IDC_BTN_OPEN_FILE);
	m_ButtonOpenFolder.Initialize(m_hWnd, IDC_BTN_OPEN_FOLDER);
	m_ButtonPreviousFile.Initialize(m_hWnd, IDC_BTN_PREVIOUS_FILE);
	m_ButtonNextFile.Initialize(m_hWnd, IDC_BTN_NEXT_FILE);
	m_ButtonFolderPlayback.Initialize(m_hWnd, IDC_BTN_FOLDER_PLAYBACK);
	m_ButtonRepeat.Initialize(m_hWnd, IDC_BTN_REPEAT);
	m_ButtonPlay.Initialize(m_hWnd, IDC_BTN_PLAY);
	m_ButtonStop.Initialize(m_hWnd, IDC_BTN_STOP);
	m_ButtonSkipBack.Initialize(m_hWnd, IDC_BTN_SKIP_BACK);
	m_ButtonSkipForward.Initialize(m_hWnd, IDC_BTN_SKIP_FORWARD);
	m_ButtonPlaySpeedDown.Initialize(m_hWnd, IDC_BTN_PLAY_SPEED_DOWN);
	m_ButtonPlaySpeedUp.Initialize(m_hWnd, IDC_BTN_PLAY_SPEED_UP);
	m_ButtonViewpoint1.Initialize(m_hWnd, IDC_BTN_VIEW_POINT_1);
	m_ButtonViewpoint2.Initialize(m_hWnd, IDC_BTN_VIEW_POINT_2);
	m_ButtonViewpoint3.Initialize(m_hWnd, IDC_BTN_VIEW_POINT_3);
	m_ButtonMyViewpoint1.Initialize(m_hWnd, IDC_BTN_MY_VIEW_POINT_1);
	m_ButtonMyViewpoint2.Initialize(m_hWnd, IDC_BTN_MY_VIEW_POINT_2);
	m_ButtonMyViewpoint3.Initialize(m_hWnd, IDC_BTN_MY_VIEW_POINT_3);
	m_ButtonViewMode.Initialize(m_hWnd, IDC_BTN_VIEW_MODE);
	m_ButtonMIDIOUT.Initialize(m_hWnd, IDC_BTN_MIDIOUT);

	return result;
}

//******************************************************************************
// ボタン画像初期化
//******************************************************************************
int MTOperationPanelCtrl::_InitButtonImages()
{
	int result = 0;

	//ボタン画像初期化の戻り値は無視する
	//ボタン画像初期化が失敗した場合はボタンに画像を反映しない

	//ファイルオープン
	_CreateButtonImage(&m_ButtonOpenFile, L"Button-OpenFile-N@2x.png", &m_hImgButtonOpenFileN);
	_CreateButtonImage(&m_ButtonOpenFile, L"Button-OpenFile-D@2x.png", &m_hImgButtonOpenFileD);
	//フォルダオープン
	_CreateButtonImage(&m_ButtonOpenFolder, L"Button-OpenFolder-N@2x.png", &m_hImgButtonOpenFolderN);
	_CreateButtonImage(&m_ButtonOpenFolder, L"Button-OpenFolder-D@2x.png", &m_hImgButtonOpenFolderD);
	//前ファイル
	_CreateButtonImage(&m_ButtonPreviousFile, L"Button-PrevFile-N@2x.png", &m_hImgButtonPreviousFileN);
	_CreateButtonImage(&m_ButtonPreviousFile, L"Button-PrevFile-D@2x.png", &m_hImgButtonPreviousFileD);
	//次ファイル
	_CreateButtonImage(&m_ButtonNextFile, L"Button-NextFile-N@2x.png", &m_hImgButtonNextFileN);
	_CreateButtonImage(&m_ButtonNextFile, L"Button-NextFile-D@2x.png", &m_hImgButtonNextFileD);
	//フォルダ演奏
	_CreateButtonImage(&m_ButtonFolderPlayback, L"Button-FolderPlayback-N@2x.png", &m_hImgButtonFolderPlaybackN);
	_CreateButtonImage(&m_ButtonFolderPlayback, L"Button-FolderPlayback-D@2x.png", &m_hImgButtonFolderPlaybackD);
	_CreateButtonImage(&m_ButtonFolderPlayback, L"Button-FolderPlayback-S@2x.png", &m_hImgButtonFolderPlaybackS);
	//リピート
	_CreateButtonImage(&m_ButtonRepeat, L"Button-Repeat-N@2x.png", &m_hImgButtonRepeatN);
	_CreateButtonImage(&m_ButtonRepeat, L"Button-Repeat-D@2x.png", &m_hImgButtonRepeatD);
	_CreateButtonImage(&m_ButtonRepeat, L"Button-Repeat-S@2x.png", &m_hImgButtonRepeatS);
	//再生/一時停止
	_CreateButtonImage(&m_ButtonPlay, L"Button-Play-N@2x.png", &m_hImgButtonPlayN);
	_CreateButtonImage(&m_ButtonPlay, L"Button-Play-D@2x.png", &m_hImgButtonPlayD);
	_CreateButtonImage(&m_ButtonPlay, L"Button-Pause-N@2x.png", &m_hImgButtonPauseN);
	_CreateButtonImage(&m_ButtonPlay, L"Button-Pause-D@2x.png", &m_hImgButtonPauseD);
	//停止
	_CreateButtonImage(&m_ButtonStop, L"Button-Stop-N@2x.png", &m_hImgButtonStopN);
	_CreateButtonImage(&m_ButtonStop, L"Button-Stop-D@2x.png", &m_hImgButtonStopD);
	//後方スキップ
	_CreateButtonImage(&m_ButtonSkipBack, L"Button-Skip-backward-N@2x.png", &m_hImgButtonSkipBackN);
	_CreateButtonImage(&m_ButtonSkipBack, L"Button-Skip-backward-D@2x.png", &m_hImgButtonSkipBackD);
	//前方スキップ
	_CreateButtonImage(&m_ButtonSkipForward, L"Button-Skip-forward-N@2x.png", &m_hImgButtonSkipForwardN);
	_CreateButtonImage(&m_ButtonSkipForward, L"Button-Skip-forward-D@2x.png", &m_hImgButtonSkipForwardD);
	//スピードダウン
	_CreateButtonImage(&m_ButtonPlaySpeedDown, L"Button-Play-speed-down-N@2x.png", &m_hImgButtonSpeedDownN);
	_CreateButtonImage(&m_ButtonPlaySpeedDown, L"Button-Play-speed-down-D@2x.png", &m_hImgButtonSpeedDownD);
	//スピードアップ
	_CreateButtonImage(&m_ButtonPlaySpeedUp, L"Button-Play-speed-up-N@2x.png", &m_hImgButtonSpeedUpN);
	_CreateButtonImage(&m_ButtonPlaySpeedUp, L"Button-Play-speed-up-D@2x.png", &m_hImgButtonSpeedUpD);
	//視点1
	_CreateButtonImage(&m_ButtonViewpoint1, L"Button-View1-N@2x.png", &m_hImgButtonViewpoint1N);
	_CreateButtonImage(&m_ButtonViewpoint1, L"Button-View1-D@2x.png", &m_hImgButtonViewpoint1D);
	//視点2
	_CreateButtonImage(&m_ButtonViewpoint2, L"Button-View2-N@2x.png", &m_hImgButtonViewpoint2N);
	_CreateButtonImage(&m_ButtonViewpoint2, L"Button-View2-D@2x.png", &m_hImgButtonViewpoint2D);
	//視点3
	_CreateButtonImage(&m_ButtonViewpoint3, L"Button-View3-N@2x.png", &m_hImgButtonViewpoint3N);
	_CreateButtonImage(&m_ButtonViewpoint3, L"Button-View3-D@2x.png", &m_hImgButtonViewpoint3D);
	//私の視点1
	_CreateButtonImage(&m_ButtonMyViewpoint1, L"Button-MyView1-N@2x.png", &m_hImgButtonMyViewpoint1N);
	_CreateButtonImage(&m_ButtonMyViewpoint1, L"Button-MyView1-D@2x.png", &m_hImgButtonMyViewpoint1D);
	//私の視点2
	_CreateButtonImage(&m_ButtonMyViewpoint2, L"Button-MyView2-N@2x.png", &m_hImgButtonMyViewpoint2N);
	_CreateButtonImage(&m_ButtonMyViewpoint2, L"Button-MyView2-D@2x.png", &m_hImgButtonMyViewpoint2D);
	//私の視点3
	_CreateButtonImage(&m_ButtonMyViewpoint3, L"Button-MyView3-N@2x.png", &m_hImgButtonMyViewpoint3N);
	_CreateButtonImage(&m_ButtonMyViewpoint3, L"Button-MyView3-D@2x.png", &m_hImgButtonMyViewpoint3D);
	//ビューモード
	_CreateButtonImage(&m_ButtonViewMode, L"Button-ViewMode-N@2x.png", &m_hImgButtonViewModeN);
	_CreateButtonImage(&m_ButtonViewMode, L"Button-ViewMode-D@2x.png", &m_hImgButtonViewModeD);
	//MIDI OUT
	_CreateButtonImage(&m_ButtonMIDIOUT, L"Button-MIDIOUT-N@2x.png", &m_hImgButtonMIDIOUTN);
	_CreateButtonImage(&m_ButtonMIDIOUT, L"Button-MIDIOUT-D@2x.png", &m_hImgButtonMIDIOUTD);

	return result;
}

//******************************************************************************
// ボタンツールチップ初期化
//******************************************************************************
int MTOperationPanelCtrl::_InitButtonToolTips()
{
	int result = 0;

	//ボタンツールチップ作成の戻り値は無視する

	m_ButtonOpenFile.CreateToolTip(_T("Open File..."));
	m_ButtonOpenFolder.CreateToolTip(_T("Open Folder..."));
	m_ButtonPreviousFile.CreateToolTip(_T("Previous File"));
	m_ButtonNextFile.CreateToolTip(_T("Next File"));
	m_ButtonFolderPlayback.CreateToolTip(_T("Folder Playback"));
	m_ButtonRepeat.CreateToolTip(_T("Repeat"));
	m_ButtonPlay.CreateToolTip(_T("Play / Pause"));
	m_ButtonStop.CreateToolTip(_T("Stop"));
	m_ButtonSkipBack.CreateToolTip(_T("Skip Back"));
	m_ButtonSkipForward.CreateToolTip(_T("Skip Forward"));
	m_ButtonPlaySpeedDown.CreateToolTip(_T("Speed Down"));
	m_ButtonPlaySpeedUp.CreateToolTip(_T("Speed Up"));
	m_ButtonViewpoint1.CreateToolTip(_T("Viewpoint 1"));
	m_ButtonViewpoint2.CreateToolTip(_T("Viewpoint 2"));
	m_ButtonViewpoint3.CreateToolTip(_T("Viewpoint 3"));
	m_ButtonMyViewpoint1.CreateToolTip(_T("My Viewpoint 1"));
	m_ButtonMyViewpoint2.CreateToolTip(_T("My Viewpoint 2"));
	m_ButtonMyViewpoint3.CreateToolTip(_T("My Viewpoint 3"));
	m_ButtonViewMode.CreateToolTip(_T("View Mode..."));
	m_ButtonMIDIOUT.CreateToolTip(_T("MIDI OUT..."));

	return result;
}

//******************************************************************************
// ボタン画像初期化
//******************************************************************************
int MTOperationPanelCtrl::_CreateButtonImage(
		MTPanelButton* pButton,
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

	*phBitmap = NULL;

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPathW(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//画像ファイルパス作成
	wcscat_s(imgFilePath, _MAX_PATH, MT_IMG_PANEL_DIR);
	wcscat_s(imgFilePath, _MAX_PATH, pImgFileName);

	//画像読み込み
	result = _LoadImage((void**)&pSrcBitmap, imgFilePath);
	if (result != 0) goto EXIT;

	//ボタンサイズを取得
	result = pButton->GetSize(&width, &height);
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

	//GDI+のBitmapからHBITMAPに変換
	// 第1引数は背景色（透明な部分を何色で埋めるか。Color(0,0,0,0)は透明）
	status = pDestBitmap->GetHBITMAP(
					Gdiplus::Color::Color(0, 0, 0, 0),	//背景色
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
// 画像読み込み
//******************************************************************************
int MTOperationPanelCtrl::_LoadImage(
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
void MTOperationPanelCtrl::_ReleaseBitmap(HBITMAP hBitmap)
{
	if (hBitmap != NULL) {
		DeleteObject(hBitmap);
	}
	return;
}

//******************************************************************************
// ウィンドウハンドル取得
//******************************************************************************
HWND MTOperationPanelCtrl::GetWindowHandle()
{
	return m_hWnd;
}

//******************************************************************************
// 表示状態設定
//******************************************************************************
void MTOperationPanelCtrl::SetDisplay(bool isDisplay)
{
	m_isDisplay = isDisplay;
	
	if (m_isDisplay) {
		ShowWindow(m_hWnd, SW_SHOW);
	}
	else {
		ShowWindow(m_hWnd, SW_HIDE);
	}
}

//******************************************************************************
// 表示状態取得
//******************************************************************************
bool MTOperationPanelCtrl::IsDisplay()
{
	return m_isDisplay;
}

//******************************************************************************
// 演奏状態設定
//******************************************************************************
void MTOperationPanelCtrl::SetPlayStatus(PlayStatus status)
{
	m_PlayStatus = status;
}

//******************************************************************************
// ボタン活性状態設定
//******************************************************************************
void MTOperationPanelCtrl::SetEnabledForMenu(int menuId, bool isEnabled)
{
	MTPanelButton* pButton = NULL;
	MTOperationPanelButtonImage buttonImage;
	
	//ボタンオブジェクト取得
	pButton = _GetButtonObjectForMenu(menuId);
	
	//ボタン画像参照
	buttonImage = _GetButtonImageForMenu(menuId);
	
	//ボタン画像設定
	if (pButton != NULL) {
		//ボタン状態更新
		pButton->SetEnabled(isEnabled);
		
		//ボタン画像更新
		if (isEnabled) {
			if (pButton->IsSwitchStatus()) {
				pButton->SetImage(buttonImage.hImgS); //スイッチONボタン
			}
			else {
				pButton->SetImage(buttonImage.hImgN); //通常ボタン
			}
		}
		else {
			pButton->SetImage(buttonImage.hImgD); //不活性ボタン
		}
	}

	return;
}

//******************************************************************************
// マーク設定
//******************************************************************************
void MTOperationPanelCtrl::SetMenuMark(int menuId, bool isON)
{
	MTPanelButton* pButton = NULL;
	MTOperationPanelButtonImage buttonImage;
	
	//ボタンオブジェクト取得
	pButton = _GetButtonObjectForMenu(menuId);
	
	//ボタン画像参照
	buttonImage = _GetButtonImageForMenu(menuId);
	
	//ボタン画像設定
	if (pButton != NULL) {
		//ボタン状態更新
		pButton->SetSwitchStatus(isON);
		
		//ボタン画像更新
		if (pButton->IsEnabled()) {
			if (isON) {
				pButton->SetImage(buttonImage.hImgS); //スイッチONボタン
			}
			else {
				pButton->SetImage(buttonImage.hImgN); //通常ボタン
			}
		}
		else {
			pButton->SetImage(buttonImage.hImgD); //不活性ボタン
		}
	}

	return;
}

//******************************************************************************
// フルスクリーン状態設定
//******************************************************************************
void MTOperationPanelCtrl::SetFullScreenStatus(bool isFullScreen)
{
	m_isFullScreen = isFullScreen;
}

//******************************************************************************
// パネル位置更新
//******************************************************************************
int MTOperationPanelCtrl::UpdatePanelPosition()

{
	int result = 0;
	BOOL bresult = FALSE;
	HRESULT hresult = 0;
	RECT rectMainWindow;
	RECT rectOperationPanel;
	int mw, mh;
	int pw, ph, px, py;
	UINT showWindow = 0;
	BOOL isDwmEnabled = FALSE;

	//初期化処理実施前は何もしない
	if (m_hMainWnd == NULL) goto EXIT;

	//Windwos 7向け対応：Aeroが有効であるか確認
	hresult = DwmIsCompositionEnabled(&isDwmEnabled);
	if (hresult != S_OK) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}

	//メインウィンドウのサイズ（影を含まないサイズ）
	if (isDwmEnabled) {
		hresult = DwmGetWindowAttribute(
							m_hMainWnd,						//ウィンドウハンドル
							DWMWA_EXTENDED_FRAME_BOUNDS,	//取得値を示すフラグ：拡張フレーム境界
							&rectMainWindow, 				//値の格納先
							sizeof(RECT)					//値のサイズ
						);
		if (hresult != S_OK) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
			goto EXIT;
		}
	}
	else {
		GetWindowRect(m_hMainWnd, &rectMainWindow);
	}
	mw = rectMainWindow.right - rectMainWindow.left;
	mh = rectMainWindow.bottom - rectMainWindow.top;
	
	//操作パネルのサイズ（影を含まないサイズ）
	if (isDwmEnabled) {
		hresult = DwmGetWindowAttribute(
							m_hWnd,							//ウィンドウハンドル
							DWMWA_EXTENDED_FRAME_BOUNDS,	//取得値を示すフラグ：拡張フレーム境界
							&rectOperationPanel, 			//値の格納先
							sizeof(RECT)					//値のサイズ
						);
		if (hresult != S_OK) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
			goto EXIT;
		}
	}
	else {
		GetWindowRect(m_hWnd, &rectOperationPanel);
	}
	pw = rectOperationPanel.right - rectOperationPanel.left;
	ph = rectOperationPanel.bottom - rectOperationPanel.top;

	//操作パネル表示位置
	if (m_isFullScreen) {
		px = rectMainWindow.left + (mw / 2) - (pw / 2);
		py = rectMainWindow.bottom - ph - 40;
	}
	else {
		px = rectMainWindow.left + (mw / 2) - (pw / 2);
		py = rectMainWindow.bottom + 10;
	}

	//操作パネル表示位置変更
	if (m_isDisplay) {
		showWindow = SWP_SHOWWINDOW;
	}
	bresult = SetWindowPos(
		m_hWnd,			//ウィンドウハンドル
		NULL,			//配置順序：SWP_NOZORDER指定により無視される
		px,				//横方向の位置
		py,				//縦方向の位置
		0,				//幅  ：SWP_NOSIZE指定により無視される
		0,				//高さ：SWP_NOSIZE指定により無視される
		SWP_NOSIZE | SWP_NOZORDER | showWindow	//ウィンドウ位置指定
	);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ボタンオブジェクト参照
//******************************************************************************
MTOperationPanelCtrl::MTPanelButton* MTOperationPanelCtrl::_GetButtonObjectForMenu(int menuId)
{
	MTPanelButton* pButton = NULL;
	
	switch (menuId) {
		case IDM_OPEN_FILE:
			pButton = &m_ButtonOpenFile;
			break;
		case IDM_OPEN_FOLDER:
			pButton = &m_ButtonOpenFolder;
			break;
		case IDM_PREVIOUS_FILE:
			pButton = &m_ButtonPreviousFile;
			break;
		case IDM_NEXT_FILE:
			pButton = &m_ButtonNextFile;
			break;
		case IDM_PLAY:
			pButton = &m_ButtonPlay;
			break;
		case IDM_STOP:
			pButton = &m_ButtonStop;
			break;
		case IDM_REPEAT:
			pButton = &m_ButtonRepeat;
			break;
		case IDM_FOLDER_PLAYBACK:
			pButton = &m_ButtonFolderPlayback;
			break;
		case IDM_SKIP_BACK:
			pButton = &m_ButtonSkipBack;
			break;
		case IDM_SKIP_FORWARD:
			pButton = &m_ButtonSkipForward;
			break;
		case IDM_PLAY_SPEED_DOWN:
			pButton = &m_ButtonPlaySpeedDown;
			break;
		case IDM_PLAY_SPEED_UP:
			pButton = &m_ButtonPlaySpeedUp;
			break;
		case IDM_RESET_VIEWPOINT:
			pButton = &m_ButtonViewpoint1;
			break;
		case IDM_VIEWPOINT2:
			pButton = &m_ButtonViewpoint2;
			break;
		case IDM_VIEWPOINT3:
			pButton = &m_ButtonViewpoint3;
			break;
		case IDM_MYVIEWPOINT1:
			pButton = &m_ButtonMyViewpoint1;
			break;
		case IDM_MYVIEWPOINT2:
			pButton = &m_ButtonMyViewpoint2;
			break;
		case IDM_MYVIEWPOINT3:
			pButton = &m_ButtonMyViewpoint3;
			break;
		case IDM_VIEW_3DPIANOROLL:
			pButton = &m_ButtonViewMode;
			break;
		case IDM_OPTION_MIDIOUT:
			pButton = &m_ButtonMIDIOUT;
			break;
		default:
			//該当ボタンなし
			break;
	}
	
	return pButton;
}

//******************************************************************************
// ボタン画像参照
//******************************************************************************
MTOperationPanelButtonImage MTOperationPanelCtrl::_GetButtonImageForMenu(int menuId)
{
	MTOperationPanelButtonImage buttonImage;
	
	buttonImage.hImgN = NULL;
	buttonImage.hImgD = NULL;
	buttonImage.hImgS = NULL;
	
	switch (menuId) {
		case IDM_OPEN_FILE:
			buttonImage.hImgN = m_hImgButtonOpenFileN;
			buttonImage.hImgD = m_hImgButtonOpenFileD;
			break;
		case IDM_OPEN_FOLDER:
			buttonImage.hImgN = m_hImgButtonOpenFolderN;
			buttonImage.hImgD = m_hImgButtonOpenFolderD;
			break;
		case IDM_PREVIOUS_FILE:
			buttonImage.hImgN = m_hImgButtonPreviousFileN;
			buttonImage.hImgD = m_hImgButtonPreviousFileD;
			break;
		case IDM_NEXT_FILE:
			buttonImage.hImgN = m_hImgButtonNextFileN;
			buttonImage.hImgD = m_hImgButtonNextFileD;
			break;
		case IDM_PLAY:
			buttonImage.hImgN = m_hImgButtonPlayN;
			buttonImage.hImgD = m_hImgButtonPlayD;
			if (m_PlayStatus == Play) {
				buttonImage.hImgN = m_hImgButtonPauseN;
				buttonImage.hImgD = m_hImgButtonPauseD;
			}
			break;
		case IDM_STOP:
			buttonImage.hImgN = m_hImgButtonStopN;
			buttonImage.hImgD = m_hImgButtonStopD;
			break;
		case IDM_REPEAT:
			buttonImage.hImgN = m_hImgButtonRepeatN;
			buttonImage.hImgD = m_hImgButtonRepeatD;
			buttonImage.hImgS = m_hImgButtonRepeatS;
			break;
		case IDM_FOLDER_PLAYBACK:
			buttonImage.hImgN = m_hImgButtonFolderPlaybackN;
			buttonImage.hImgD = m_hImgButtonFolderPlaybackD;
			buttonImage.hImgS = m_hImgButtonFolderPlaybackS;
			break;
		case IDM_SKIP_BACK:
			buttonImage.hImgN = m_hImgButtonSkipBackN;
			buttonImage.hImgD = m_hImgButtonSkipBackD;
			break;
		case IDM_SKIP_FORWARD:
			buttonImage.hImgN = m_hImgButtonSkipForwardN;
			buttonImage.hImgD = m_hImgButtonSkipForwardD;
			break;
		case IDM_PLAY_SPEED_DOWN:
			buttonImage.hImgN = m_hImgButtonSpeedDownN;
			buttonImage.hImgD = m_hImgButtonSpeedDownD;
			break;
		case IDM_PLAY_SPEED_UP:
			buttonImage.hImgN = m_hImgButtonSpeedUpN;
			buttonImage.hImgD = m_hImgButtonSpeedUpD;
			break;
		case IDM_RESET_VIEWPOINT:
			buttonImage.hImgN = m_hImgButtonViewpoint1N;
			buttonImage.hImgD = m_hImgButtonViewpoint1D;
			break;
		case IDM_VIEWPOINT2:
			buttonImage.hImgN = m_hImgButtonViewpoint2N;
			buttonImage.hImgD = m_hImgButtonViewpoint2D;
			break;
		case IDM_VIEWPOINT3:
			buttonImage.hImgN = m_hImgButtonViewpoint3N;
			buttonImage.hImgD = m_hImgButtonViewpoint3D;
			break;
		case IDM_MYVIEWPOINT1:
			buttonImage.hImgN = m_hImgButtonMyViewpoint1N;
			buttonImage.hImgD = m_hImgButtonMyViewpoint1D;
			break;
		case IDM_MYVIEWPOINT2:
			buttonImage.hImgN = m_hImgButtonMyViewpoint2N;
			buttonImage.hImgD = m_hImgButtonMyViewpoint2D;
			break;
		case IDM_MYVIEWPOINT3:
			buttonImage.hImgN = m_hImgButtonMyViewpoint3N;
			buttonImage.hImgD = m_hImgButtonMyViewpoint3D;
			break;
		case IDM_VIEW_3DPIANOROLL:
			buttonImage.hImgN = m_hImgButtonViewModeN;
			buttonImage.hImgD = m_hImgButtonViewModeD;
			break;
		case IDM_OPTION_MIDIOUT:
			buttonImage.hImgN = m_hImgButtonMIDIOUTN;
			buttonImage.hImgD = m_hImgButtonMIDIOUTD;
			break;
		default:
			break;
	}
	
	return buttonImage;
}

//******************************************************************************
// メインウィンドウ：ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTOperationPanelCtrl::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// メインウィンドウ：ウィンドウプロシージャ：実装
//******************************************************************************
INT_PTR MTOperationPanelCtrl::_WndProcImpl(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	BOOL bresult = TRUE;
	int wmId = 0;
	int wmEvent = 0;
	HDC hdc = NULL;
	PAINTSTRUCT ps;

	switch (message) {
		case WM_INITDIALOG:
			//ダイアログ初期化メッセージ（表示前）
			break;
		case WM_CTLCOLORDLG:
			//ダイアログボックス描画前メッセージ
			//背景用ブラシを返す
			return (INT_PTR)m_hBackgroundBrush;
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch (wmId) {
				case IDC_BTN_OPEN_FILE:
					//ファイルオープン
					result = _OnBtnOpenFile();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_OPEN_FOLDER:
					//フォルダオープン
					result = _OnBtnOpenFolder();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PREVIOUS_FILE:
					//前ファイル
					result = _OnBtnPreviousFile();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_NEXT_FILE:
					//次ファイル
					result = _OnBtnNextFile();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_FOLDER_PLAYBACK:
					//フォルダ演奏
					result = _OnBtnFolderPlayback();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_REPEAT:
					//リピート
					result = _OnBtnRepeat();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PLAY:
					//再生/一時停止
					result = _OnBtnPlay();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_STOP:
					//停止
					result = _OnBtnStop();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_SKIP_BACK:
					//後方スキップ
					result = _OnBntSkipBack();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_SKIP_FORWARD:
					//前方スキップ
					result = _OnBtnSkipForward();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PLAY_SPEED_DOWN:
					//スピードダウン
					result = _OnBtnSpeedDown();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_PLAY_SPEED_UP:
					//スピードアップ
					result = _OnBtnSpeedUp();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_VIEW_POINT_1:
					//視点1
					result = _OnBtnViewPoint1();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_VIEW_POINT_2:
					//視点2
					result = _OnBtnViewPoint2();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_VIEW_POINT_3:
					//視点3
					result = _OnBtnViewPoint3();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_MY_VIEW_POINT_1:
					//私の視点1
					result = _OnBtnMyViewPoint1();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_MY_VIEW_POINT_2:
					//私の視点2
					result = _OnBtnMyViewPoint2();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_MY_VIEW_POINT_3:
					//私の視点3
					result = _OnBtnMyViewPoint3();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_VIEW_MODE:
					//ビューモード
					result = _OnBtnViewMode();
					if (result != 0) goto EXIT;
					break;
				case IDC_BTN_MIDIOUT:
					//MIDI出力設定
					result = _OnBtnMIDIOUT();
					if (result != 0) goto EXIT;
					break;
				default:
					bresult = FALSE;
					break;
			}
			break;
		case WM_PAINT:
			//描画
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		case WM_KEYDOWN:
			//キー押下メッセージ
			//メッセージループのIsDialogMessageでダイアグのキー入力イベントが処理されてしまうため
			//ここでキー入力イベントを拾うことはできない
			break;
		case WM_TIMER:
			//タイマー
			break;
		case WM_DESTROY:
			//破棄
			break;
		default:
			bresult = FALSE;
			break;
	}

EXIT:;
	if (bresult) {
		//操作パネルがフォアグラウンドの場合
		//メインウィンドウをフォアグラウンドウィンドウに変更することで
		//メインウィンドウでキー入力操作を処理できるようにする
		if (GetForegroundWindow() == m_hWnd) {
			SetForegroundWindow(m_hMainWnd);
		}
	}
	if (result != 0) {
		YN_SHOW_ERR(m_hWnd);
	}
	return bresult;
}

//******************************************************************************
// ファイルオープン
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnOpenFile()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonOpenFile.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_OPEN_FILE, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// フォルダオープン
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnOpenFolder()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonOpenFolder.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_OPEN_FOLDER, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 前ファイル
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnPreviousFile()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonPreviousFile.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_PREVIOUS_FILE, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 次ファイル
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnNextFile()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonNextFile.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_NEXT_FILE, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// フォルダ演奏
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnFolderPlayback()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonFolderPlayback.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_FOLDER_PLAYBACK, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// リピート
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnRepeat()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonRepeat.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_REPEAT, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 再生/一時停止
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnPlay()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonPlay.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_PLAY, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 停止
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnStop()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonStop.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_STOP, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 後方スキップ
//******************************************************************************
int MTOperationPanelCtrl::_OnBntSkipBack()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonSkipBack.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_SKIP_BACK, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 前方スキップ
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnSkipForward()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonSkipForward.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_SKIP_FORWARD, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// スピードダウン
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnSpeedDown()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonPlaySpeedDown.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_PLAY_SPEED_DOWN, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// スピードアップ
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnSpeedUp()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonPlaySpeedUp.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_PLAY_SPEED_UP, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 視点1
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnViewPoint1()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonViewpoint1.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_RESET_VIEWPOINT, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 視点2
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnViewPoint2()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonViewpoint2.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_VIEWPOINT2, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;}

//******************************************************************************
// 視点3
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnViewPoint3()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonViewpoint3.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_VIEWPOINT3, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;}

//******************************************************************************
// 私の視点1
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnMyViewPoint1()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonMyViewpoint1.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_MYVIEWPOINT1, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 私の視点2
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnMyViewPoint2()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonMyViewpoint2.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_MYVIEWPOINT2, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 私の視点3
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnMyViewPoint3()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonMyViewpoint3.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_MYVIEWPOINT3, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ビューモード
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnViewMode()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonViewMode.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_VIEWMODE, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI出力設定
//******************************************************************************
int MTOperationPanelCtrl::_OnBtnMIDIOUT()
{
	int result = 0;
	BOOL bresult = FALSE;

	if (m_ButtonMIDIOUT.IsEnabled()) {
		bresult = PostMessage(m_hMainWnd, WM_COMMAND, IDM_OPTION_MIDIOUT, 0);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}


