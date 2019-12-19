//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail アプリケーションクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "imagehlp.h"
#include "shellapi.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MIDITrailApp.h"
#include "MTSceneTitle.h"
#include "MTScenePianoRoll3D.h"
#include "MTScenePianoRoll2D.h"
#include "MTScenePianoRollRain.h"
#include "MTScenePianoRollRain2D.h"
#include "MTScenePianoRollRing.h"
#include "MTScenePianoRoll3DLive.h"
#include "MTScenePianoRoll2DLive.h"
#include "MTScenePianoRollRainLive.h"
#include "MTScenePianoRollRain2DLive.h"
#include "MTScenePianoRollRingLive.h"

using namespace YNBaseLib;


//******************************************************************************
// ウィンドウプロシージャ制御用パラメータ設定
//******************************************************************************
MIDITrailApp* MIDITrailApp::m_pThis = NULL;

//******************************************************************************
// コンストラクタ
//******************************************************************************
MIDITrailApp::MIDITrailApp(void)
{
	m_pThis = this;
	m_hInstance = NULL;
	m_hAppMutex = NULL;
	m_hMailSlot = NULL;
	m_isExitApp = false;

	//ウィンドウ系
	m_hWnd = NULL;
	m_Accel = NULL;
	m_Title[0] = _T('\0');
	m_WndClassName[0] = _T('\0');
	m_isFullScreen = false;
	m_hMenu = NULL;

	//レンダリング系
	m_pScene = NULL;
	m_MultiSampleType = 0;

	//FPS表示系
	m_PrevTime = 0;
	m_FPSCount = 0;

	//演奏状態
	m_PlayStatus = NoData;
	m_isRepeat = false;
	m_isRewind = false;
	m_isOpenFileAfterStop = false;
	ZeroMemory(&m_SequencerLastMsg, sizeof(MTSequencerLastMsg));
	m_PlaySpeedRatio = 100;

	//表示状態
	m_isEnablePianoKeyboard = true;
	m_isEnableRipple = true;
	m_isEnablePitchBend = true;
	m_isEnableStars = true;
	m_isEnableCounter = true;
	m_isEnableFileName = false;
	m_isEnableBackgroundImage = true;

	//シーン種別
	m_SceneType = Title;
	m_SelectedSceneType = PianoRoll3D;

	//自動視点保存
	m_isAutoSaveViewpoint = false;

	//プレーヤー制御
	m_AllowMultipleInstances = 0;
	m_AutoPlaybackAfterOpenFile = 0;

	//リワインド／スキップ制御
	m_SkipBackTimeSpanInMsec = 10000;
	m_SkipForwardTimeSpanInMsec = 10000;

	//演奏スピード制御
	m_SpeedStepInPercent = 1;
	m_MaxSpeedInPercent = 400;

	//次回オープン対象ファイルパス
	m_NextFilePath[0] = _T('\0');

	//ゲームパッド用視点番号
	m_GamePadViewPointNo = 0;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MIDITrailApp::~MIDITrailApp(void)
{
	Terminate();
}

//******************************************************************************
// 初期化
//******************************************************************************
int MIDITrailApp::Initialize(
		HINSTANCE hInstance,
		LPTSTR pCmdLine,
		int nCmdShow
	)
{
	int result = 0;
	bool isExitApp = false;

	m_hInstance = hInstance;

	//文字列初期化
	LoadString(hInstance, IDS_APP_TITLE, m_Title, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MIDITRAIL, m_WndClassName, MAX_LOADSTRING);

	//設定ファイル初期化
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//グラフィック設定読み込み
	result = _LoadGraphicConf();
	if (result != 0) goto EXIT;

	//プレーヤー設定読み込み
	result = _LoadPlayerConf();
	if (result != 0) goto EXIT;

	//二重起動チェック
	result = _CheckMultipleInstances(&m_isExitApp);
	if (result != 0) goto EXIT;

	//二重起動抑止の場合
	if (m_isExitApp) {
		_PostFilePathToFirstMIDITrail(pCmdLine);
		goto EXIT;
	}

	//メールスロット作成
	result = _CreateMailSlot();
	if (result != 0) goto EXIT;

	//メッセージキュー初期化
	result = m_MsgQueue.Initialize(10000);
	if (result != 0) goto EXIT;

	//ウィンドウクラス登録
	result = _RegisterClass(hInstance);
	if (result != 0) goto EXIT;

	//メインウィンドウ生成
	result = _CreateWindow(hInstance, nCmdShow);
	if (result != 0) goto EXIT;

	//アクセラレータテーブル読み込み
	m_Accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIDITRAIL));
	if (m_Accel == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hInstance);
		goto EXIT;
	}

	//演奏状態変更
	result = _ChangePlayStatus(NoData);
	if (result != 0) goto EXIT;

	//レンダラ初期化
	result = m_Renderer.Initialize(m_hWnd, m_MultiSampleType);
	if (result != 0) goto EXIT;

	//シーンオブジェクト生成
	m_SceneType = Title;
	result = _CreateScene(m_SceneType, &m_SeqData);
	if (result != 0) goto EXIT;

	//シーン種別読み込み
	result = _LoadSceneType();
	if (result != 0) goto EXIT;

	//シーン設定読み込み
	result = _LoadSceneConf();
	if (result != 0) goto EXIT;

	//メニュー選択マーク更新
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//RCPファイルコンバータ初期化
	result = m_RcpConv.Initialize();
	if (result != 0) goto EXIT;

	//レンダラチェック
	result = _CheckRenderer();
	if (result != 0) goto EXIT;

	//MIDI OUT 自動設定
	result = _AutoConfigMIDIOUT();
	if (result != 0) goto EXIT;

	//コマンドライン解析と実行
	result = _ParseCmdLine(pCmdLine);
	if (result != 0) goto EXIT;

	//タイマー開始
	result = _StartTimer();
	if (result != 0) goto EXIT;

	//ゲームパッド制御：ユーザインデックス0固定
	result = m_GamePadCtrl.Initialize(0);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 終了処理
//******************************************************************************
int MIDITrailApp::Terminate()
{
	int result = 0;

	_StopTimer();

	m_Renderer.Terminate();

	if (m_pScene != NULL) {
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}

	if (m_hAppMutex != NULL) {
		CloseHandle(m_hAppMutex);
		m_hAppMutex = NULL;
	}

	if (m_hMailSlot != NULL) {
		CloseHandle(m_hMailSlot);
		m_hMailSlot = NULL;
	}

	return result;
}

//******************************************************************************
// 実行
//******************************************************************************
int MIDITrailApp::Run()
{
	int result = 0;
	int quitCode = 0;
	BOOL isExist = FALSE;
	MSG msg;
	WINDOWPLACEMENT wndpl;

	if (m_isExitApp) goto EXIT;

	m_PrevTime = timeGetTime();

	//メッセージループ
	while (TRUE) {
		isExist = PeekMessage(
						&msg,		//取得したメッセージ
						NULL,		//取得元ウィンドウハンドル
						0,			//取得対象メッセージ最小値
						0,			//取得対象メッセージ最大値
						PM_REMOVE	//メッセージ処理方法：キューから削除
					);
		if (isExist) {
			if (msg.message == WM_QUIT) {
				quitCode = (int)msg.wParam;
				break;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (m_pScene != NULL) {
			//シーケンサーメッセージ処理
			result = _SequencerMsgProc();
			if (result != 0) {
				YN_SHOW_ERR(m_hWnd);
			}

			//ゲームパッド操作処理
			result = _GamePadProc();
			if (result != 0) {
				YN_SHOW_ERR(m_hWnd);
			}

			//ウィンドウ表示状態でのみ描画を行う
			GetWindowPlacement(m_hWnd, &wndpl);
			if ((wndpl.showCmd != SW_HIDE) &&
				(wndpl.showCmd != SW_MINIMIZE) &&
				(wndpl.showCmd != SW_SHOWMINIMIZED) &&
				(wndpl.showCmd != SW_SHOWMINNOACTIVE)) {
				//描画
				result = m_Renderer.RenderScene(m_pScene);
				if (result != 0) {
					if (result == DXRENDERER_ERR_DEVICE_LOST) {
						//デバイスロスト
						//暫定的対策としてシーンを再生成する
						result = _RebuildScene();
						if (result != 0) {
							YN_SHOW_ERR(m_hWnd);
							PostMessage(m_hWnd, WM_DESTROY, 0, 0);
						}
					}
					else {
						YN_SHOW_ERR(m_hWnd);
						PostMessage(m_hWnd, WM_DESTROY, 0, 0);
					}
				}
				_UpdateFPS();
			}
		}
    }

EXIT:;
	//関数がWM_QUITメッセージを受け取って正常に終了する場合は
	//wParamに格納されている終了コードを返す
	//メッセージループに入る前に終了する場合は0を返す
	return quitCode;
}

//******************************************************************************
// ウィンドウクラス登録
//******************************************************************************
int MIDITrailApp::_RegisterClass(
		HINSTANCE hInstance
	)
{
	int result = 0;
	ATOM aresult = 0;
	WNDCLASSEX wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);				//構造体サイズ
	wcex.style			= CS_HREDRAW | CS_VREDRAW;			//クラススタイル
	wcex.lpfnWndProc	= _WndProc;							//ウィンドウプロシージャ
	wcex.cbClsExtra		= 0;								//追加情報のサイズ
	wcex.cbWndExtra		= 0;								//追加情報のサイズ
	wcex.hInstance		= hInstance;						//アプリケーションインスタンスハンドル
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIDITRAIL));
															//アイコンリソースハンドル
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);		//カーソルリソースハンドル
	wcex.hbrBackground  = CreateSolidBrush(RGB(0, 0, 0));	//背景用ブラシハンドル：黒
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MIDITRAIL);	//メニューリソース名称
	wcex.lpszClassName	= m_WndClassName;					//ウィンドウクラス名称
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
				 											//小アイコンリソースハンドル

	//移動やサイズ変更におけるウインドウ無効領域の再描画指定
	// CS_HREDRAW クライアント領域の幅が変化したときに再描画する
	// CS_VREDRAW クライアント領域の高さが変化したときに再描画する

	aresult = RegisterClassEx(&wcex);
	if (aresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メインウィンドウ生成
//******************************************************************************
int MIDITrailApp::_CreateWindow(
		HINSTANCE hInstance,
		int nCmdShow
	)
{
	int result = 0;

	m_hWnd = CreateWindow(
				m_WndClassName,			//ウィンドウクラス名
				m_Title,				//ウィンドウ名
				MIDITRAIL_WINDOW_STYLE,	//ウィンドウスタイル
				CW_USEDEFAULT,			//ウィンドウの横方向の位置：デフォルト
				0,						//ウィンドウの縦方向の位置
				CW_USEDEFAULT,			//ウィンドウの幅：デフォルト
				0,						//ウィンドウの高さ
				NULL,					//親またはオーナーのウィンドウハンドル
				NULL,					//メニューハンドルまたは子ウィンドウID
				hInstance,				//アプリケーションインスタンスハンドル
				NULL					//ウィンドウ作成データ
			);
	if (m_hWnd == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//メニューバー表示切替のためウィンドウ生成直後にハンドルを取得しておく
	m_hMenu = GetMenu(m_hWnd);

	//ユーザー設定ウィンドウサイズ変更
	result = _SetWindowSize();
	if (result != 0) goto EXIT;

	//ウィンドウ表示
	ShowWindow(m_hWnd, nCmdShow);

	//WM_PAINT呼び出しを止める
	ValidateRect(m_hWnd, 0);

	UpdateWindow(m_hWnd);

EXIT:;
	return result;
}

//******************************************************************************
// ウィンドウサイズ変更
//******************************************************************************
int MIDITrailApp::_SetWindowSize()
{
	int result = 0;
	BOOL bresult = FALSE;
	int width = 0;
	int height = 0;
	RECT wrect, crect;
	int ww, wh, cw, ch, framew, frameh;
	int applyToViewArea = 0;
	LONG apiresult = 0;

	if (m_isFullScreen) {
		result = _SetWindowSizeFullScreen();
		goto EXIT;
	}

	//ユーザ選択ウィンドウサイズ取得
	result = m_ViewConf.SetCurSection(_T("WindowSize"));
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Width"), &width, 0);
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Height"), &height, 0);
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("ApplyToViewArea"), &applyToViewArea, 0);
	if (result != 0) goto EXIT;

	//初回起動時のウィンドウサイズ
	if ((width <= 0) || (height <= 0)) {
		width = 800;
		height = 600;
	}

	//ウィンドウのサイズ
	GetWindowRect(m_hWnd, &wrect);
	ww = wrect.right - wrect.left;
	wh = wrect.bottom - wrect.top;

	//クライアント領域のサイズ
	GetClientRect(m_hWnd, &crect);
	cw = crect.right - crect.left;
	ch = crect.bottom - crect.top;

	//枠のサイズ
	framew = ww - cw;
	frameh = wh - ch;

	//描画領域に指定サイズを適用する場合
	if (applyToViewArea != 0) {
		width = width + framew;
		height = height + frameh;
	}
	
	//ウィンドウスタイル設定
	apiresult = SetWindowLong(m_hWnd, GWL_STYLE, MIDITRAIL_WINDOW_STYLE);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}
	
	//メニューバー表示
	result = _ShowMenu();
	if (result != 0) goto EXIT;
	
	//ウィンドウサイズ変更
	bresult = SetWindowPos(
					m_hWnd,			//ウィンドウハンドル
					HWND_TOP,		//配置順序：Zオーダー先頭
					0,				//横方向の位置
					0,				//縦方向の位置
					width,			//幅
					height,			//高さ
					SWP_NOMOVE | SWP_FRAMECHANGED | SWP_SHOWWINDOW	//ウィンドウ位置指定
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ウィンドウサイズ変更：フルスクリーン
//******************************************************************************
int MIDITrailApp::_SetWindowSizeFullScreen()
{
	int result = 0;
	BOOL bresult = FALSE;
	LONG apiresult = 0;
	POINT mouseCursorPoint;
	HMONITOR hMonitor = NULL;
	MONITORINFOEX monitorInfo;
	int width = 0;
	int height = 0;

	//マウスカーソル位置を取得
	bresult = GetCursorPos(&mouseCursorPoint);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//マウスカーソルの位置に該当するモニタを選択
	hMonitor = MonitorFromPoint(mouseCursorPoint, MONITOR_DEFAULTTONEAREST);

	//モニタ情報取得
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	bresult = GetMonitorInfo(hMonitor, &monitorInfo);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hMonitor);
		goto EXIT;
	}

	//ウィンドウ縦横サイズ
	width  = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	//ウィンドウスタイル設定
	apiresult = SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

	//メニューバー非表示
	result = _HideMenu();
	if (result != 0) goto EXIT;

	//ウィンドウサイズ変更
	bresult = SetWindowPos(
					m_hWnd,						//ウィンドウハンドル
					HWND_TOP,					//配置順序：Zオーダー先頭
					monitorInfo.rcMonitor.left,	//横方向の位置
					monitorInfo.rcMonitor.top,	//縦方向の位置
					width,						//幅
					height,						//高さ
					SWP_FRAMECHANGED | SWP_SHOWWINDOW	//ウィンドウ位置指定
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル初期化
//******************************************************************************
int MIDITrailApp::_InitConfFile()
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR userConfDirPath[_MAX_PATH] = {_T('\0')};
	TCHAR viewConfPath[_MAX_PATH] = {_T('\0')};
	TCHAR midiOutConfPath[_MAX_PATH] = {_T('\0')};
	TCHAR graphicConfPath[_MAX_PATH] = {_T('\0')};

	//ユーザ設定ファイル格納ディレクトリパス作成
	result = YNPathUtil::GetAppDataDirPath(userConfDirPath, _MAX_PATH);
	if (result != 0) goto EXIT;
	_tcscat_s(userConfDirPath, _MAX_PATH, MT_USER_CONFFILE_DIR);

	//ユーザ設定ファイル格納ディレクトリ作成
	bresult = MakeSureDirectoryPathExists(userConfDirPath);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//ビュー情報設定ファイル
	_tcscat_s(viewConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(viewConfPath, _MAX_PATH, MT_USER_CONFFILE_VIEW);
	result = m_ViewConf.Initialize(viewConfPath);
	if (result != 0) goto EXIT;

	//MIDI情報設定ファイル
	_tcscat_s(midiOutConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(midiOutConfPath, _MAX_PATH, MT_USER_CONFFILE_MIDI);
	result = m_MIDIConf.Initialize(midiOutConfPath);
	if (result != 0) goto EXIT;

	//グラフィック情報設定ファイル
	_tcscat_s(graphicConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(graphicConfPath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);
	result = m_GraphicConf.Initialize(graphicConfPath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メインウィンドウ：ウィンドウプロシージャ
//******************************************************************************
LRESULT CALLBACK MIDITrailApp::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// メインウィンドウ：ウィンドウプロシージャ：実装
//******************************************************************************
LRESULT MIDITrailApp::_WndProcImpl(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	LRESULT lresult = 0;
	int wmId = 0;
	int wmEvent = 0;
	HDC hdc = NULL;
	PAINTSTRUCT ps;

	switch (message) {
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch (wmId) {
				case IDM_OPEN_FILE:
					//ファイルオープン
					result = _OnMenuFileOpen();
					if (result != 0) goto EXIT;
					break;
				case IDM_EXIT:
					//終了
					DestroyWindow(hWnd);
					break;
				case IDM_PLAY:
					//演奏開始／一時停止／再開
					result = _OnMenuPlay();
					if (result != 0) goto EXIT;
					break;
				case IDM_STOP:
					//演奏停止
					result = _OnMenuStop();
					if (result != 0) goto EXIT;
					break;
				case IDM_REPEAT:
					//リピート
					result = _OnMenuRepeat();
					if (result != 0) goto EXIT;
					break;
				case IDM_SKIP_BACK:
					//再生スキップバック
					result = _OnMenuSkipBack();
					if (result != 0) goto EXIT;
					break;
				case IDM_SKIP_FORWARD:
					//再生スキップフォワード
					result = _OnMenuSkipForward();
					if (result != 0) goto EXIT;
					break;
				case IDM_PLAY_SPEED_DOWN:
					//再生スピードダウン
					result = _OnMenuPlaySpeedDown();
					if (result != 0) goto EXIT;
					break;
				case IDM_PLAY_SPEED_UP:
					//再生スピードアップ
					result = _OnMenuPlaySpeedUp();
					if (result != 0) goto EXIT;
					break;
				case IDM_START_MONITORING:
					//モニタリング開始
					result = _OnMenuStartMonitoring();
					if (result != 0) goto EXIT;
					break;
				case IDM_STOP_MONITORING:
					//モニタリング停止
					result = _OnMenuStopMonitoring();
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_3DPIANOROLL:
					//ビュー変更：3Dピアノロール
					result = _OnMenuSelectSceneType(PianoRoll3D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_2DPIANOROLL:
					//ビュー変更：2Dピアノロール
					result = _OnMenuSelectSceneType(PianoRoll2D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRAIN:
					//ビュー変更：ピアノロールレイン
					result = _OnMenuSelectSceneType(PianoRollRain);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRAIN2D:
					//ビュー変更：ピアノロールレイン2D
					result = _OnMenuSelectSceneType(PianoRollRain2D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRING:
					//ビュー変更：ピアノロールリング
					result = _OnMenuSelectSceneType(PianoRollRing);
					if (result != 0) goto EXIT;
					break;
				//TAG: シーン追加
				case IDM_ENABLE_PIANOKEYBOARD:
					//表示効果：ピアノキーボード
					result = _OnMenuEnableEffect(MTScene::EffectPianoKeyboard);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_RIPPLE:
					//表示効果：波紋
					result = _OnMenuEnableEffect(MTScene::EffectRipple);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_PITCHBEND:
					//表示効果：ピッチベンド
					result = _OnMenuEnableEffect(MTScene::EffectPitchBend);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_STARS:
					//表示効果：星
					result = _OnMenuEnableEffect(MTScene::EffectStars);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_COUNTER:
					//表示効果：カウンタ
					result = _OnMenuEnableEffect(MTScene::EffectCounter);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_BACKGROUNDIMAGE:
					//表示効果：背景画像
					result = _OnMenuEnableEffect(MTScene::EffectBackgroundImage);
					if (result != 0) goto EXIT;
					break;
				//自動視点保存と視点保存は廃止
				//case IDM_AUTO_SAVE_VIEWPOINT:
				//	//自動視点保存
				//	result = _OnMenuAutoSaveViewpoint();
				//	if (result != 0) goto EXIT;
				//	break;
				//case IDM_SAVE_VIEWPOINT:
				//	//視点保存
				//	result = _OnMenuSaveViewpoint();
				//	if (result != 0) goto EXIT;
				//	break;
				case IDM_RESET_VIEWPOINT:
					//静的視点1に移動（視点リセット）
					result = _OnMenuResetViewpoint();
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEWPOINT2:
					//静的視点2に移動
					result = _OnMenuViewpoint(2);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEWPOINT3:
					//静的視点3に移動
					result = _OnMenuViewpoint(3);
					if (result != 0) goto EXIT;
					break;
				case IDM_WINDOWSIZE:
					//ウィンドウサイズ設定
					result = _OnMenuWindowSize();
					if (result != 0) goto EXIT;
					break;
				case IDM_FULLSCREEN:
					//フルスクリーン
					result = _OnMenuFullScreen();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_MIDIOUT:
					//MIDI出力デバイス設定
					result = _OnMenuOptionMIDIOUT();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_MIDIIN:
					//MIDI入力デバイス設定
					result = _OnMenuOptionMIDIIN();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_GRAPHIC:
					//グラフィック設定
					result = _OnMenuOptionGraphic();
					if (result != 0) goto EXIT;
					break;
				case IDM_HOWTOVIEW:
					//操作方法ダイアログ表示
					m_HowToViewDlg.Show(m_hWnd);
					break;
				case IDM_MANUAL:
					//マニュアル表示
					result = _OnMenuManual();
					if (result != 0) goto EXIT;
					break;
				case IDM_ABOUT:
					//バージョン情報ダイアログ表示
					m_AboutDlg.Show(m_hWnd);
					break;
				default:
					lresult = DefWindowProc(hWnd, message, wParam, lParam);
					break;
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		case WM_KEYDOWN:
			//キー押下メッセージ
			result = _OnKeyDown(wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			//マウスボタン押下メッセージ
			result = _OnMouseButtonDown(message, wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_MOUSEMOVE:
			result = _OnMouseMove(message, wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_DROPFILES:
			//ファイルドロップ
			result = _OnDropFiles(wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_TIMER:
			//タイマー
			result = _OnTimer(wParam);
			if (result != 0) goto EXIT;
			break;
		case WM_DESTROY:
			//破棄
			result = _OnDestroy();
			//戻り値は無視する
			PostQuitMessage(0);
			break;
		case WM_FILEPATH_POSTED:
			//ファイルパスポスト通知
			result = _OnFilePathPosted();
			if (result != 0) goto EXIT;
			break;
		case WM_SIZE:
			//ウィンドウサイズ変更
			if (wParam == SIZE_MAXIMIZED) {
				//最大化：フルスクリーン
				result = _OnMenuFullScreen();
				if (result != 0) goto EXIT;
			}
			break;
		default:
			lresult = DefWindowProc(hWnd, message, wParam, lParam);
			break;
	}

EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(m_hWnd);
	}
	return lresult;
}

//******************************************************************************
// ファイルオープン
//******************************************************************************
int MIDITrailApp::_OnMenuFileOpen()
{
	int result = 0;
	TCHAR filePath[MAX_PATH] = {_T('\0')};
	bool isSelected = false;

	////演奏中はファイルオープンさせない
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	//ファイルオープンOK
	//}
	//else {
	//	//ファイルオープンNG
	//	goto EXIT;
	//}

	//演奏中でもファイルオープン可とする

	//ファイル選択ダイアログ表示
	result = _SelectMIDIFile(filePath, MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//ファイル選択時の処理
	if (isSelected) {
		//フルスクリーンでメニューからファイル選択した場合
		//  シーン生成処理でクライアントウィンドウのサイズを参照しているため
		//  一時的に表示したメニューを非表示に戻しておく
		if (m_isFullScreen) {
			_HideMenu();
		}

		//演奏/モニタ停止とファイルオープン処理
		result = _StopPlaybackAndOpenFile(filePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：再生／一時停止／再開
//******************************************************************************
int MIDITrailApp::_OnMenuPlay()
{
	int result = 0;

	if (m_PlayStatus == Stop) {
		//シーケンサ初期化
		result = m_Sequencer.Initialize(&m_MsgQueue);
		if (result != 0) goto EXIT;

		//シーケンサにポート情報を登録
		result = _SetPortDev(&m_Sequencer);
		if (result != 0) goto EXIT;

		//シーケンサにシーケンスデータを登録
		result = m_Sequencer.SetSeqData(&m_SeqData);
		if (result != 0) goto EXIT;

		//巻き戻し
		if (m_isRewind) {
			m_isRewind = false;
			result = m_pScene->Rewind();
			if (result != 0) goto EXIT;
		}

		//シーンに演奏開始を通知
		result = m_pScene->OnPlayStart(m_Renderer.GetDevice());
		if (result != 0) goto EXIT;

		//最新シーケンサメッセージクリア
		ZeroMemory(&m_SequencerLastMsg, sizeof(MTSequencerLastMsg));

		//演奏速度
		m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);

		//演奏開始
		result = m_Sequencer.Play();
		if (result != 0) goto EXIT;

		//演奏状態変更
		result = _ChangePlayStatus(Play);
		if (result != 0) goto EXIT;
	}
	else if (m_PlayStatus == Play) {
		//演奏一時停止
		m_Sequencer.Pause();

		//演奏状態変更
		result = _ChangePlayStatus(Pause);
		if (result != 0) goto EXIT;
	}
	else if (m_PlayStatus == Pause) {
		//演奏再開
		result = m_Sequencer.Resume();
		if (result != 0) goto EXIT;

		//演奏状態変更
		result = _ChangePlayStatus(Play);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：停止
//******************************************************************************
int MIDITrailApp::_OnMenuStop()
{
	int result = 0;

	if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		m_Sequencer.Stop();
		//演奏状態通知が届くまで再生中とみなす
		//ここでは演奏状態を変更しない

		//終了後に巻き戻す
		m_isRewind = true;
	}

	return result;
}

//******************************************************************************
// メニュー選択：リピート
//******************************************************************************
int MIDITrailApp::_OnMenuRepeat()
{
	int result = 0;

	//リピート切り替え
	if (m_isRepeat) {
		m_isRepeat = false;
	}
	else {
		m_isRepeat = true;
	}

	//メニュー選択マーク更新
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：スキップバック
//******************************************************************************
int MIDITrailApp::_OnMenuSkipBack()
{
	int result = 0;

	result = m_Sequencer.Skip((-1) * m_SkipBackTimeSpanInMsec);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：スキップフォワード
//******************************************************************************
int MIDITrailApp::_OnMenuSkipForward()
{
	int result = 0;

	result = m_Sequencer.Skip((+1) * m_SkipForwardTimeSpanInMsec);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：スピードダウン
//******************************************************************************
int MIDITrailApp::_OnMenuPlaySpeedDown()
{
	int result = 0;

	//演奏状態確認
	if ((m_PlayStatus == Stop) || (m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//変更OK
	}
	else {
		//変更NG
		goto EXIT;
	}

	//演奏速度ダウン
	m_PlaySpeedRatio -= m_SpeedStepInPercent;

	//リミット
	if (m_PlaySpeedRatio < m_SpeedStepInPercent) {
		m_PlaySpeedRatio = m_SpeedStepInPercent;
	}

	//演奏速度設定
	m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：スピードアップ
//******************************************************************************
int MIDITrailApp::_OnMenuPlaySpeedUp()
{
	int result = 0;

	//演奏状態確認
	if ((m_PlayStatus == Stop) || (m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//変更OK
	}
	else {
		//変更NG
		goto EXIT;
	}

	//演奏速度アップ
	m_PlaySpeedRatio += m_SpeedStepInPercent;

	//リミット 400%
	if (m_PlaySpeedRatio > m_MaxSpeedInPercent) {
		m_PlaySpeedRatio = m_MaxSpeedInPercent;
	}

	//演奏速度設定
	m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：ライブモニタ開始
//******************************************************************************
int MIDITrailApp::_OnMenuStartMonitoring()
{
	int result = 0;
	
	//演奏状態確認
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//モニタ開始OK
	}
	else {
		//モニタ開始NG
		goto EXIT;
	}
	
	//シーケンサ初期化
	//  シーケンサは再生終了時にデバイスをクローズしないため
	//  初期化することによってクローズさせる
	result = m_Sequencer.Initialize(&m_MsgQueue);
	if (result != 0) goto EXIT;
	
	//ライブモニタ用シーン生成
	if (m_PlayStatus != MonitorOFF) {
		//視点保存
		if (m_isAutoSaveViewpoint) {
			result = _OnMenuSaveViewpoint();
			if (result != 0) goto EXIT;
		}
		
		//シーン種別
		m_SceneType = m_SelectedSceneType;
		
		//シーン生成
		result = _CreateScene(m_SceneType, NULL);
		if (result != 0) goto EXIT;
	}
	
	//ライブモニタ初期化
	result = m_LiveMonitor.Initialize(&m_MsgQueue);
	if (result != 0) goto EXIT;
	result = _SetMonitorPortDev(&m_LiveMonitor, m_pScene);
	if (result != 0) goto EXIT;
	
	//シーンに演奏開始を通知
	result = m_pScene->OnPlayStart(m_Renderer.GetDevice());
	if (result != 0) goto EXIT;
	
	//ライブモニタ開始
	result = m_LiveMonitor.Start();
	if (result != 0) goto EXIT;
	
	//演奏状態変更
	result = _ChangePlayStatus(MonitorON);
	if (result != 0) goto EXIT;
		
EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：ライブモニタ停止
//******************************************************************************
int MIDITrailApp::_OnMenuStopMonitoring()
{
	int result = 0;
	
	//演奏状態確認
	if (m_PlayStatus == MonitorON) {
		//モニタ開始OK
	}
	else {
		//モニタ開始NG
		goto EXIT;
	}
	
	//ライブモニタ停止
	result = m_LiveMonitor.Stop();
	if (result != 0) goto EXIT;
	
	//演奏状態変更
	result = _ChangePlayStatus(MonitorOFF);
	if (result != 0) goto EXIT;
	
	//シーンに演奏終了を通知
	if (m_pScene != NULL) {
		result = m_pScene->OnPlayEnd(m_Renderer.GetDevice());
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：シーン種別
//******************************************************************************
int MIDITrailApp::_OnMenuSelectSceneType(
		MIDITrailApp::SceneType type
	)
{
	int result = 0;

	//演奏状態確認
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//シーンタイプ選択OK
	}
	else {
		//シーンタイプ選択NG
		goto EXIT;
	}

	//保存
	m_SelectedSceneType = type;
	result = _SaveSceneType();
	if (result != 0) goto EXIT;

	//メニュー選択マーク更新
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//停止中の場合はシーンを再構築
	if ((m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//視点保存
		if (m_isAutoSaveViewpoint) {
			result = _OnMenuSaveViewpoint();
			if (result != 0) goto EXIT;
		}

		m_SceneType = m_SelectedSceneType;
		if (m_PlayStatus == Stop) {
			//プレイヤのシーン種別切り替え
			result = _CreateScene(m_SceneType, &m_SeqData);
			if (result != 0) goto EXIT;
		}
		else {
			//ライブモニタのシーン種別切り替え
			result = _CreateScene(m_SceneType, NULL);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：自動視点保存
//******************************************************************************
int MIDITrailApp::_OnMenuAutoSaveViewpoint()
{
	int result = 0;

	m_isAutoSaveViewpoint = m_isAutoSaveViewpoint ? false : true;

	//メニュー選択マーク更新
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//シーン設定保存
	result = _SaveSceneConf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：静的視点移動
//******************************************************************************
int MIDITrailApp::_OnMenuViewpoint(
		unsigned long viewpointNo
	)
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//静的視点に移動
	m_pScene->MoveToStaticViewpoint(viewpointNo);

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：視点リセット
//******************************************************************************
int MIDITrailApp::_OnMenuResetViewpoint()
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//シーンの視点をリセット
	m_pScene->ResetViewpoint();

	//視点保存
	result = _SaveViewpoint();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：視点保存
//******************************************************************************
int MIDITrailApp::_OnMenuSaveViewpoint()
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//視点保存
	result = _SaveViewpoint();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：表示効果設定
//******************************************************************************
int MIDITrailApp::_OnMenuEnableEffect(
		MTScene::EffectType type
	)
{
	int result = 0;

	switch (type) {
		case MTScene::EffectPianoKeyboard:
			m_isEnablePianoKeyboard = m_isEnablePianoKeyboard ? false : true;
			break;
		case MTScene::EffectRipple:
			m_isEnableRipple = m_isEnableRipple ? false : true;
			break;
		case MTScene::EffectPitchBend:
			m_isEnablePitchBend = m_isEnablePitchBend ? false : true;
			break;
		case MTScene::EffectStars:
			m_isEnableStars = m_isEnableStars ? false : true;
			break;
		case MTScene::EffectCounter:
			m_isEnableCounter = m_isEnableCounter ? false : true;
			break;
		case MTScene::EffectBackgroundImage:
			m_isEnableBackgroundImage = m_isEnableBackgroundImage ? false : true;
			break;
		default:
			break;
	}

	_UpdateEffect();
	_UpdateMenuCheckmark();

//EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：ウィンドウサイズ変更
//******************************************************************************
int MIDITrailApp::_OnMenuWindowSize()
{
	int result = 0;

	//設定ダイアログ表示
	result = m_WindowSizeCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//変更された場合はウィンドウサイズを更新
	if (m_WindowSizeCfgDlg.IsChanged()) {
		result = _ChangeWindowSize();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：フルスクリーン
//******************************************************************************
int MIDITrailApp::_OnMenuFullScreen()
{
	int result = 0;

	//フルスクリーン切替
	result = _ToggleFullScreen();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：MIDI出力デバイス設定
//******************************************************************************
int MIDITrailApp::_OnMenuOptionMIDIOUT()
{
	int result = 0;

	//設定ダイアログ表示
	result = m_MIDIOUTCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// メニュー選択：MIDI入力デバイス設定
//******************************************************************************
int MIDITrailApp::_OnMenuOptionMIDIIN()
{
	int result = 0;

	//設定ダイアログ表示
	result = m_MIDIINCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：グラフィック設定
//******************************************************************************
int MIDITrailApp::_OnMenuOptionGraphic()
{
	int result = 0;
	unsigned long multiSampleType = 0;
	bool isSupport = false;

	//アンチエイリアスサポート情報をダイアログに設定
	for (multiSampleType = DX_MULTI_SAMPLE_TYPE_MIN; multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX; multiSampleType++) {
		result = m_Renderer.IsSupportAntialias(multiSampleType, &isSupport);
		if (result != 0) goto EXIT;
		m_GraphicCfgDlg.SetAntialiasSupport(multiSampleType, isSupport);
	}

	//設定ダイアログ表示
	result = m_GraphicCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//変更された場合はレンダラとシーンオブジェクトを再生成
	if (m_GraphicCfgDlg.IsChanged()) {
		result = _LoadGraphicConf();
		if (result != 0) goto EXIT;
		result = _ChangeWindowSize();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// マニュアル表示
//******************************************************************************
int MIDITrailApp::_OnMenuManual()
{
	int result = 0;
	HINSTANCE hresult = 0;
	TCHAR manualPath[_MAX_PATH] = {_T('\0')};

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPath(manualPath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//マニュアルファイルパス作成
	_tcscat_s(manualPath, _MAX_PATH, MT_MANUALFILE);

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
// シーケンサメッセージ処理
//******************************************************************************
int MIDITrailApp::_SequencerMsgProc()
{
	int result = 0;
	bool isExist = false;
	unsigned long param1 = 0;
	unsigned long param2 = 0;
	SMMsgParser parser;
	
	while (true) {
		//メッセージ取り出し
		result = m_MsgQueue.GetMessage(&isExist, &param1, &param2);
		if (result != 0) goto EXIT;
		
		//メッセージがなければ終了
		if (!isExist) break;
		
		//シーケンサメッセージ受信処理
		result = _OnRecvSequencerMsg(param1, param2);
		if (result != 0) goto EXIT;	
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MIDITrailApp::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	//シーンにシーケンサメッセージを渡す
	if (m_pScene != NULL) {
		result = m_pScene->OnRecvSequencerMsg(param1, param2);
		if (result != 0) goto EXIT;
	}

	//演奏状態変更通知への対応
	parser.Parse(param1, param2);
	if (parser.GetMsg() == SMMsgParser::MsgPlayStatus) {
		//一時停止
		if (parser.GetPlayStatus() == SMMsgParser::StatusPause) {
			result = _ChangePlayStatus(Pause);
			if (result != 0) goto EXIT;
		}
		//停止（演奏終了）
		if (parser.GetPlayStatus() == SMMsgParser::StatusStop) {
			result = _ChangePlayStatus(Stop);
			if (result != 0) goto EXIT;

			//シーンに演奏終了を通知
			if (m_pScene != NULL) {
				result = m_pScene->OnPlayEnd(m_Renderer.GetDevice());
				if (result != 0) goto EXIT;
			}

			//視点保存
			if (m_isAutoSaveViewpoint) {
				result = _OnMenuSaveViewpoint();
				if (result != 0) goto EXIT;
			}

			//ユーザーの要求によって停止した場合は巻き戻す
			if ((m_isRewind) && (m_pScene != NULL)) {
				m_isRewind = false;
				result = m_pScene->Rewind();
				if (result != 0) goto EXIT;
			}
			//停止後のファイルオープンが指定されている場合
			else if ((m_isOpenFileAfterStop) && (m_pScene != NULL)) {
				m_isOpenFileAfterStop = false;
				//ファイル読み込み処理
				result = _FileOpenProc(m_NextFilePath);
				if (result != 0) goto EXIT;
			}
			//通常の演奏終了の場合は次回の演奏時に巻き戻す
			else {
				m_isRewind = true;
				//リピート有効なら再生開始
				if (m_isRepeat) {
					result = _OnMenuPlay();
					if (result != 0) goto EXIT;
				}
			}

			//コマンドラインで終了指定されている場合
			if (m_CmdLineParser.GetSwitch(CMDSW_QUIET) == CMDSW_ON) {
				DestroyWindow(m_hWnd);
			}
		}
	}

	//デバイスロスト対策
	//シーンに渡した最新メッセージを記録しておく
	if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		//演奏チックタイム通知
		m_SequencerLastMsg.isRecvPlayTime = true;
		m_SequencerLastMsg.playTime.param1 = param1;
		m_SequencerLastMsg.playTime.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		//テンポ変更通知
		m_SequencerLastMsg.isRecvTempo = true;
		m_SequencerLastMsg.tempo.param1 = param1;
		m_SequencerLastMsg.tempo.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		//小節番号通知
		m_SequencerLastMsg.isRecvBar = true;
		m_SequencerLastMsg.bar.param1 = param1;
		m_SequencerLastMsg.bar.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		//拍子記号変更通知
		m_SequencerLastMsg.isRecvBeat = true;
		m_SequencerLastMsg.beat.param1 = param1;
		m_SequencerLastMsg.beat.param2 = param2;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ウィンドウクリックイベント
//******************************************************************************
int MIDITrailApp::_OnMouseButtonDown(
		UINT button,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	
	if ((m_pScene != NULL) && (m_PlayStatus != NoData)) {
		result = m_pScene->OnWindowClicked(button, wParam, lParam);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// マウス移動イベント
//******************************************************************************
int MIDITrailApp::_OnMouseMove(
		UINT button,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	LONG apiresult = 0;
	POINT point;

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);
	
	//フルスクリーンの場合
	if (m_isFullScreen) {
		//マウスカーソルがスクリーン上端に移動した場合
		if (point.y == 0) {
			//メニューバー表示
			result = _ShowMenu();
			if (result != 0) goto EXIT;
		}
		else {
			//メニューバー非表示
			result = _HideMenu();
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// キー入力イベント
//******************************************************************************
int MIDITrailApp::_OnKeyDown(
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	unsigned short keycode = 0;

	keycode = LOWORD((DWORD)wParam);

	switch (keycode) {
		case VK_SPACE:
		case VK_NUMPAD0:
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				//モニタリング開始
				result = _OnMenuStartMonitoring();
				if (result != 0) goto EXIT;
			}
			else {
				//演奏開始／一時停止
				result = _OnMenuPlay();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_ESCAPE:
			if (m_PlayStatus == MonitorON) {
				//モニタリング停止
				result = _OnMenuStopMonitoring();
				if (result != 0) goto EXIT;
			}
			else {
				//演奏停止
				result = _OnMenuStop();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_RETURN:
			//演奏停止：テンキーのENTERでかつNUMLOCKオンの場合
			if ((HIWORD((DWORD)lParam) & KF_EXTENDED) && (GetKeyState(VK_NUMLOCK) & 0x01)) {
				result = _OnMenuStop();
				if (result != 0) goto EXIT;
			}
			break;
		case '1':
		case VK_NUMPAD1:
			//再生スキップバック
			result = _OnMenuSkipBack();
			if (result != 0) goto EXIT;
			break;
		case '2':
		case VK_NUMPAD2:
			//再生スキップフォワード
			result = _OnMenuSkipForward();
			if (result != 0) goto EXIT;
			break;
		case '4':
		case VK_NUMPAD4:
			//再生スピードダウン
			result = _OnMenuPlaySpeedDown();
			if (result != 0) goto EXIT;
			break;
		case '5':
		case VK_NUMPAD5:
			//再生スピードアップ
			result = _OnMenuPlaySpeedUp();
			if (result != 0) goto EXIT;
			break;
		case '7':
		case VK_NUMPAD7:
			//視点リセット
			result = _OnMenuResetViewpoint();
			if (result != 0) goto EXIT;
			break;
		case '8':
		case VK_NUMPAD8:
			//静的視点2移動
			result = _OnMenuViewpoint(2);
			if (result != 0) goto EXIT;
			break;
		case '9':
		case VK_NUMPAD9:
			//静的視点3移動
			result = _OnMenuViewpoint(3);
			if (result != 0) goto EXIT;
			break;
		case 'O':
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				//ファイルオープン
				result = _OnMenuFileOpen();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_F11:
			//フルスクリーン
			result = _OnMenuFullScreen();
			if (result != 0) goto EXIT;
			break;
		default:
			break;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ファイルドロップイベント
//******************************************************************************
int MIDITrailApp::_OnDropFiles(
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	UINT fileNum = 0;
	UINT charNum = 0;
	HDROP hDrop = NULL;
	TCHAR path[_MAX_PATH] = {_T('\0')};
	bool isMIDIDataFile = false;

	////停止中でなければファイルドロップは無視する
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	//ファイルドロップOK
	//}
	//else {
	//	//ファイルドロップNG
	//	goto EXIT;
	//}

	//常にファイルドロップを許可する

	hDrop = (HDROP)wParam;

	//ファイル数確認
	fileNum = DragQueryFile(
					hDrop,		//wParam
					0xFFFFFFFF,	//ファイルインデックス
					NULL,		//ファイル名取得バッファ
					0			//バッファサイズ
				);

	//複数ファイルの場合は無視する
	if (fileNum != 1) goto EXIT;

	//ファイルパス取得
	charNum = DragQueryFile(
					hDrop,		//wParam
					0,			//ファイルインデックス
					path,		//ファイル名取得バッファ
					_MAX_PATH	//バッファサイズ
				);
	if (charNum == 0) {
		result = YN_SET_ERR("Windows API error.", wParam, lParam);
		goto EXIT;
	}

	//ファイル拡張子の確認
	if (YNPathUtil::IsFileExtMatch(path, _T(".mid"))) {
		isMIDIDataFile = true;
	}
	//rcpcv.dllが有効ならサポート対象ファイルであるか追加確認する
	else if (m_RcpConv.IsAvailable() && m_RcpConv.IsSupportFileExt(path)) {
		isMIDIDataFile = true;
	}

	//サポート対象ファイルでなければ何もしない
	if (!isMIDIDataFile) goto EXIT;

	//演奏/モニタ停止とファイルオープン処理
	result = _StopPlaybackAndOpenFile(path);
	if (result != 0) goto EXIT;

EXIT:;
	if (hDrop != NULL) {
		DragFinish(hDrop);
	}
	return result;
}

//******************************************************************************
// ファイル選択
//******************************************************************************
int MIDITrailApp::_SelectMIDIFile(
		TCHAR* pFilePath,
		unsigned long bufSize,
		bool* pIsSelected
	)
{
	int result = 0;
	BOOL apiresult = FALSE;
	OPENFILENAME ofn;

	if ((pFilePath == NULL) || (bufSize == 0) || (pIsSelected ==NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pFilePath[0] = _T('\0');
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = m_hWnd;
	ofn.lpstrFilter = _T("Standard MIDI File (*.mid)\0*.mid\0\0");
	ofn.lpstrFile   = pFilePath;
	ofn.nMaxFile    = bufSize;
	ofn.lpstrTitle  = _T("Select Standard MIDI File.");
	ofn.Flags       = OFN_FILEMUSTEXIST;  //OFN_HIDEREADONLY

	//rcpcv.dllが有効ならファイルフィルタを変更する
	if (m_RcpConv.IsAvailable()) {
		ofn.lpstrFilter = m_RcpConv.GetOpenFileNameFilter();
	}

	//ファイル選択ダイアログ表示
	apiresult = GetOpenFileName(&ofn);
	if (!apiresult) {
		//キャンセルまたはエラー発生：エラーはチェックしない
		*pIsSelected = false;
		goto EXIT;
	}

	*pIsSelected = true;

EXIT:;
	return result;
}

//******************************************************************************
// MIDIファイル読み込み
//******************************************************************************
int MIDITrailApp::_LoadMIDIFile(
		const TCHAR* pFilePath
	)
{
	int result = 0;
	TCHAR* pPath = NULL;
	TCHAR smfTempPath[_MAX_PATH] = {_T('\0')};
	TCHAR smfDumpPath[_MAX_PATH] = {_T('\0')};
	SMFileReader smfReader;

	//拡張子が*.midの場合
	if (YNPathUtil::IsFileExtMatch(pFilePath, _T(".mid"))) {
		pPath = (TCHAR*)pFilePath;
	}
	//拡張子が*.mid以外の場合
	else {
		//レコンポーザのデータファイルとみなしてSMFに変換する
		result = YNPathUtil::GetTempFilePath(smfTempPath, _MAX_PATH, _T("RCP"));
		if (result != 0) goto EXIT;
		result = m_RcpConv.Convert(pFilePath, smfTempPath);
		if (result != 0) goto EXIT;
		pPath = smfTempPath;
	}

	//デバッグモードであればMIDIファイル解析結果をダンプする
	if (m_CmdLineParser.GetSwitch(CMDSW_DEBUG) == CMDSW_ON) {
		_tcscat_s(smfDumpPath, _MAX_PATH, pPath);
		_tcscat_s(smfDumpPath, _MAX_PATH, _T(".dump.txt"));
		smfReader.SetLogPath(smfDumpPath);
	}

	//ファイル読み込み
	result = smfReader.Load(pPath, &m_SeqData);
	if (result != 0) goto EXIT;

	//ファイル読み込み時に再生スピードを100%に戻す：_CreateSceneでカウンタに反映
	m_PlaySpeedRatio = 100;

	//シーンオブジェクト生成
	m_SceneType = m_SelectedSceneType;
	result = _CreateScene(m_SceneType, &m_SeqData);
	if (result != 0) goto EXIT;

	//演奏状態変更
	result = _ChangePlayStatus(Stop);
	if (result != 0) goto EXIT;

	m_isRewind = false;

EXIT:;
	if (_tcslen(smfTempPath) != 0) {
		DeleteFile(smfTempPath);
	}
	return result;
}

//******************************************************************************
// FPS更新
//******************************************************************************
void MIDITrailApp::_UpdateFPS()
{
	unsigned long curTime = 0;
	unsigned long diffTime = 0;
	double fps = 0;
	TCHAR title[256];

	curTime = timeGetTime();
	m_FPSCount += 1;

	//1秒ごとにFPSを計算
	diffTime = curTime - m_PrevTime;
	if (diffTime > 1000) {

		//FPS
		fps = (double)m_FPSCount / ((double)diffTime / 1000.0f);
		m_PrevTime = curTime;
		m_FPSCount = 0;

		//ウィンドウタイトルに設定
		_stprintf_s(title, 256, _T("%s - FPS:%.1f"), m_Title, fps);
		SetWindowText(m_hWnd, title);
	}

	return;
}

//******************************************************************************
// ポート情報登録
//******************************************************************************
int MIDITrailApp::_SetPortDev(
		SMSequencer* pSequencer
	)
{
	int result = 0;
	unsigned char portNo = 0;
	TCHAR devName[MAXPNAMELEN];
	char* portName[] = {"PortA", "PortB", "PortC", "PortD", "PortE", "PortF"};

	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//設定ファイルからユーザ選択デバイス名を取得してシーケンサに登録
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		result = m_MIDIConf.GetStr(portName[portNo], devName, MAXPNAMELEN, _T(""));
		if (result != 0) goto EXIT;

		result = pSequencer->SetPortDev(portNo, devName);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN モニタ情報登録
//******************************************************************************
int MIDITrailApp::_SetMonitorPortDev(
		SMLiveMonitor* pLiveMonitor,
		MTScene* pScene
	)
{
	int result = 0;
	TCHAR devName[MAXPNAMELEN];
	int checkMIDITHRU = 0;
	bool isMIDITHRU = false;

	//--------------------------------------
	// MIDI IN
	//--------------------------------------
	//カテゴリ／セクション設定
	result = m_MIDIConf.SetCurSection(_T("MIDIIN"));
	if (result != 0) goto EXIT;

	//設定ファイルからユーザ選択デバイス名を取得してシーケンサに登録
	result = m_MIDIConf.GetStr("PortA", devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;
	result = m_MIDIConf.GetInt("MIDITHRU", &checkMIDITHRU, 1);
	if (result != 0) goto EXIT;

	if (checkMIDITHRU > 0) {
		isMIDITHRU = true;
	}
	if (_tcslen(devName) > 0) {
		result = pLiveMonitor->SetInPortDev(devName, isMIDITHRU);
		if (result != 0) goto EXIT;
	}

	//シーンに MIDI IN デバイス名を登録
	result = pScene->SetParam("MIDI_IN_DEVICE_NAME", devName);
	if (result != 0) goto EXIT;

	//--------------------------------------
	// MIDI OUT (MIDITHRU)
	//--------------------------------------
	//カテゴリ／セクション設定
	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//設定ファイルからユーザ選択デバイス名を取得してシーケンサに登録
	result = m_MIDIConf.GetStr("PortA", devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;

	if ((_tcslen(devName) > 0) && (isMIDITHRU)) {
		result = pLiveMonitor->SetOutPortDev(devName);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ウィンドウサイズ変更
//******************************************************************************
int MIDITrailApp::_ChangeWindowSize()
{
	int result = 0;
	bool isMonitor = false;

	//モニタ状態の確認
	if ((m_PlayStatus == MonitorOFF) || (m_PlayStatus == MonitorON)) {
		isMonitor = true;
	}

	//シーン破棄
	if (m_pScene != NULL) {
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}

	//レンダラ終了
	m_Renderer.Terminate();

	//ユーザー設定ウィンドウサイズ変更
	result = _SetWindowSize();
	if (result != 0) goto EXIT;

	//レンダラ初期化
	result = m_Renderer.Initialize(m_hWnd, m_MultiSampleType);
	if (result != 0) goto EXIT;

	//シーンオブジェクト生成
	if (!isMonitor) {
		//プレイヤのシーン生成
		result = _CreateScene(m_SceneType, &m_SeqData);
		if (result != 0) goto EXIT;
	}
	else {
		//ライブモニタのシーン生成
		result = _CreateScene(m_SceneType, NULL);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 演奏状態変更
//******************************************************************************
int MIDITrailApp::_ChangePlayStatus(
		PlayStatus status
	)
{
	int result = 0;

	//演奏状態変更
	m_PlayStatus = status;

	////ファイルドラック許可
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	DragAcceptFiles(m_hWnd, TRUE);
	//}
	//else {
	//	DragAcceptFiles(m_hWnd, FALSE);
	//}

	//常にファイルドラッグ許可
	DragAcceptFiles(m_hWnd, TRUE);

	//メニュースタイル更新
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュースタイル更新
//******************************************************************************
int MIDITrailApp::_ChangeMenuStyle()
{
	int result = 0;
	unsigned long menuIndex = 0;
	unsigned long statusIndex = 0;
	unsigned long style = 0;

	//メニューID一覧
	//TAG:シーン追加
	unsigned long menuID[MT_MENU_NUM] = {
		IDM_OPEN_FILE,
		IDM_EXIT,
		IDM_PLAY,
		IDM_STOP,
		IDM_REPEAT,
		IDM_SKIP_BACK,
		IDM_SKIP_FORWARD,
		IDM_PLAY_SPEED_DOWN,
		IDM_PLAY_SPEED_UP,
		IDM_START_MONITORING,
		IDM_STOP_MONITORING,
		IDM_VIEW_3DPIANOROLL,
		IDM_VIEW_2DPIANOROLL,
		IDM_VIEW_PIANOROLLRAIN,
		IDM_VIEW_PIANOROLLRAIN2D,
		IDM_VIEW_PIANOROLLRING,
		IDM_ENABLE_PIANOKEYBOARD,
		IDM_ENABLE_RIPPLE,
		IDM_ENABLE_PITCHBEND,
		IDM_ENABLE_STARS,
		IDM_ENABLE_COUNTER,
		IDM_ENABLE_BACKGROUNDIMAGE,
		IDM_RESET_VIEWPOINT,
		IDM_VIEWPOINT2,
		IDM_VIEWPOINT3,
		IDM_WINDOWSIZE,
		IDM_FULLSCREEN,
		IDM_OPTION_MIDIOUT,
		IDM_OPTION_MIDIIN,
		IDM_OPTION_GRAPHIC,
		IDM_HOWTOVIEW,
		IDM_MANUAL,
		IDM_ABOUT
	};

	//メニュースタイル一覧
	unsigned long menuStyle[MT_MENU_NUM][MT_PLAYSTATUS_NUM] = {
		//データ無, 停止, 再生中, 一時停止, モニタ停止, モニタ中
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_OPEN_FILE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_EXIT
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_PLAY
		{	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_STOP
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_REPEAT
		{	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_GRAYED	},	//IDM_SKIP_BACK
		{	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_GRAYED	},	//IDM_SKIP_FORWARD
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_PLAY_SPEED_DOWN
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_PLAY_SPEED_UP
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_START_MONITORING
		{	MF_GRAYED,	MF_GRAYED,	MF_GRAYED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED	},	//IDM_STOP_MONITORING
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_3DPIANOROLL
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_2DPIANOROLL
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_PIANOROLLRAIN
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_PIANOROLLRAIN2D
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_PIANOROLLRING
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_PIANOKEYBOARD
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_RIPPLE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_PITCHBEND
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_STARS
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_COUNTER
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_BACKGROUNDIMAGE
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_RESET_VIEWPOINT
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_VIEWPOINT2
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_VIEWPOINT3
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_WINDOWSIZE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_FULLSCREEN
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPTION_MIDIOUT
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPTION_MIDIIN
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPTION_GRAPHIC
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_HOWTOVIEW
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_MANUAL
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	}	//IDM_ABOUT
	};

	switch (m_PlayStatus) {
		case NoData: statusIndex = 0; break;
		case Stop:   statusIndex = 1; break;
		case Play:   statusIndex = 2; break;
		case Pause:  statusIndex = 3; break;
		case MonitorOFF: statusIndex = 4; break;
		case MonitorON:  statusIndex = 5; break;
	}

	//メニュースタイル更新
	for (menuIndex = 0; menuIndex < MT_MENU_NUM; menuIndex++) {
		style = menuStyle[menuIndex][statusIndex];
		EnableMenuItem(GetMenu(m_hWnd), menuID[menuIndex], style);
	}

	return result;
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MIDITrailApp::_CreateScene(
		SceneType type,
		SMSeqData* pSeqData  //ライブモニタ時はNULL
	)
{
	int result = 0;

	//シーン破棄
	if (m_pScene != NULL) {
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}

	//シーンオブジェクト生成
	//TAG:シーン追加
	try {
		if (type == Title) {
			m_pScene = new MTSceneTitle();
		}
		else {
			//プレイヤ用シーン生成
			if (pSeqData != NULL) {
				if (type == PianoRoll3D) {
					m_pScene = new MTScenePianoRoll3D();
				}
				else if (type == PianoRoll2D) {
					m_pScene = new MTScenePianoRoll2D();
				}
				else if (type == PianoRollRain) {
					m_pScene = new MTScenePianoRollRain();
				}
				else if (type == PianoRollRain2D) {
					m_pScene = new MTScenePianoRollRain2D();
				}
				else if (type == PianoRollRing) {
					m_pScene = new MTScenePianoRollRing();
				}
			}
			//ライブモニタ用シーン生成
			else {
				if (type == PianoRoll3D) {
					m_pScene = new MTScenePianoRoll3DLive();
				}
				else if (type == PianoRoll2D) {
					m_pScene = new MTScenePianoRoll2DLive();
				}
				else if (type == PianoRollRain) {
					m_pScene = new MTScenePianoRollRainLive();
				}
				else if (type == PianoRollRain2D) {
					m_pScene = new MTScenePianoRollRain2DLive();
				}
				else if (type == PianoRollRing) {
					m_pScene = new MTScenePianoRollRingLive();
				}
			}
		}
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", type, 0);
		goto EXIT;
	}

	if (m_pScene == NULL) {
		result = YN_SET_ERR("Program error.", type, 0);
		goto EXIT;
	}

	//シーンの生成
	result = m_pScene->Create(m_hWnd, m_Renderer.GetDevice(), pSeqData);
	if (result != 0) goto EXIT;

	//保存されている視点をシーンに反映する
	if (type != Title) {
		result = _LoadViewpoint();
		if (result != 0) goto EXIT;
	}

	//表示効果反映
	_UpdateEffect();

	//演奏速度設定
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// シーン種別読み込み
//******************************************************************************
int MIDITrailApp::_LoadSceneType()
{
	int result = 0;
	TCHAR type[256];

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	result = m_ViewConf.GetStr(_T("Type"), type, 256, _T(""));
	if (result != 0) goto EXIT;

	//TAG:シーン追加
	if (_tcscmp(type, _T("PianoRoll3D")) == 0) {
		m_SelectedSceneType = PianoRoll3D;
	}
	else if (_tcscmp(type, _T("PianoRoll2D")) == 0) {
		m_SelectedSceneType = PianoRoll2D;
	}
	else if (_tcscmp(type, _T("PianoRollRain")) == 0) {
		m_SelectedSceneType = PianoRollRain;
	}
	else if (_tcscmp(type, _T("PianoRollRain2D")) == 0) {
		m_SelectedSceneType = PianoRollRain2D;
	}
	else if (_tcscmp(type, _T("PianoRollRing")) == 0) {
		m_SelectedSceneType = PianoRollRing;
	}
	else {
		m_SelectedSceneType = PianoRoll3D;
	}

EXIT:;
	return result;
}

//******************************************************************************
// シーン種別保存
//******************************************************************************
int MIDITrailApp::_SaveSceneType()
{
	int result = 0;
	TCHAR* pType = _T("");

	//TAG:シーン追加
	switch (m_SelectedSceneType) {
		case PianoRoll3D:
			pType = _T("PianoRoll3D");
			break;
		case PianoRoll2D:
			pType = _T("PianoRoll2D");
			break;
		case PianoRollRain:
			pType = _T("PianoRollRain");
			break;
		case PianoRollRain2D:
			pType = _T("PianoRollRain2D");
			break;
		case PianoRollRing:
			pType = _T("PianoRollRing");
			break;
		default:
			result = YN_SET_ERR("Program error.", m_SelectedSceneType, 0);
			goto EXIT;
			break;
	}

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	result = m_ViewConf.SetStr(_T("Type"), pType);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// シーン設定読み込み
//******************************************************************************
int MIDITrailApp::_LoadSceneConf()
{
	int result = 0;
	int autoSaveViewpoint = 0;

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	//自動視点保存
	//result = m_ViewConf.GetInt(_T("AutoSaveViewpoint"), &autoSaveViewpoint, 0);
	//if (result != 0) goto EXIT;
	//
	//m_isAutoSaveViewpoint = false;
	//if (autoSaveViewpoint == 1) {
	//	m_isAutoSaveViewpoint = true;
	//}

	//自動視点保存：常に有効とする
	m_isAutoSaveViewpoint = true;

EXIT:;
	return result;
}

//******************************************************************************
// シーン設定保存
//******************************************************************************
int MIDITrailApp::_SaveSceneConf()
{
	int result = 0;
	int autoSaveViewpoint = 0;

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	//自動視点保存
	autoSaveViewpoint = m_isAutoSaveViewpoint ? 1 : 0;
	result = m_ViewConf.SetInt(_T("AutoSaveViewpoint"), autoSaveViewpoint);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 視点読み込み
//******************************************************************************
int MIDITrailApp::_LoadViewpoint()
{
	int result = 0;
	MTScene::MTViewParamMap defParamMap;
	MTScene::MTViewParamMap viewParamMap;
	MTScene::MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};
	float param = 0.0f;

	//シーンからデフォルトの視点を取得
	m_pScene->GetDefaultViewParam(&defParamMap);

	//セクション名
	_tcscat_s(section, 256, _T("Viewpoint-"));
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//パラメータを設定ファイルから取得
	for (itr = defParamMap.begin(); itr != defParamMap.end(); itr++) {
		result = m_ViewConf.GetFloat((itr->first).c_str(), &param, itr->second);
		if (result != 0) goto EXIT;
		viewParamMap.insert(MTScene::MTViewParamMapPair((itr->first).c_str(), param));
	}

	//シーンに視点を登録
	m_pScene->SetViewParam(&viewParamMap);

EXIT:;
	return result;
}

//******************************************************************************
// 視点保存
//******************************************************************************
int MIDITrailApp::_SaveViewpoint()
{
	int result = 0;
	MTScene::MTViewParamMap viewParamMap;
	MTScene::MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};

	//シーンから現在の視点を取得
	m_pScene->GetViewParam(&viewParamMap);

	//セクション名
	_tcscat_s(section, 256, _T("Viewpoint-"));
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//パラメータを設定ファイルに登録
	for (itr = viewParamMap.begin(); itr != viewParamMap.end(); itr++) {
		result = m_ViewConf.SetFloat((itr->first).c_str(), itr->second);
		if (result != 0) goto EXIT;
	}

	//視点が切り替えられたことをシーンに伝達
	m_pScene->SetViewParam(&viewParamMap);

EXIT:;
	return result;
}

//******************************************************************************
// グラフィック設定読み込み
//******************************************************************************
int MIDITrailApp::_LoadGraphicConf()
{
	int result = 0;
	int multiSampleType = 0;

	result = m_GraphicConf.SetCurSection(_T("Anti-aliasing"));
	if (result != 0) goto EXIT;

	result = m_GraphicConf.GetInt(
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
// プレーヤー設定読み込み
//******************************************************************************
int MIDITrailApp::_LoadPlayerConf()
{
	int result = 0;
	MTConfFile confFile;
	int timeSpan = 400;
	int speedStepInPercent = 1;
	int maxSpeedInPercent = 400;
	int showFileName = 0;

	result = confFile.Initialize("Player");
	if (result != 0) goto EXIT;

	//----------------------------------
	//プレーヤー制御
	//----------------------------------
	result = confFile.SetCurSection("PlayerControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("AllowMultipleInstances", &m_AllowMultipleInstances, 0);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("AutoPlaybackAfterOpenFile", &m_AutoPlaybackAfterOpenFile, 0);
	if (result != 0) goto EXIT;

	//----------------------------------
	//表示制御
	//----------------------------------
	result = confFile.SetCurSection("ViewControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("ShowFileName", &showFileName, 0);
	if (result != 0) goto EXIT;
	m_isEnableFileName = (showFileName > 0) ? true : false;

	//----------------------------------
	//リワインド／スキップ制御
	//----------------------------------
	result = confFile.SetCurSection("SkipControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SkipBackTimeSpanInMsec", &m_SkipBackTimeSpanInMsec, 10000);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SkipForwardTimeSpanInMsec", &m_SkipForwardTimeSpanInMsec, 10000);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("MovingTimeSpanInMsec", &timeSpan, 400);
	if (result != 0) goto EXIT;

	//シーケンサにリワインド／スキップ移動時間を設定
	m_Sequencer.SetMovingTimeSpanInMsec(timeSpan);

	//----------------------------------
	//演奏スピード制御
	//----------------------------------
	result = confFile.SetCurSection("PlaybackSpeedControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SpeedStepInPercent", &speedStepInPercent, 1);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("MaxSpeedInPercent", &maxSpeedInPercent, 400);
	if (result != 0) goto EXIT;

	m_SpeedStepInPercent = (unsigned long)speedStepInPercent;
	m_MaxSpeedInPercent = (unsigned long)maxSpeedInPercent;

EXIT:;
	return result;
}

//******************************************************************************
// ウィンドウ破棄
//******************************************************************************
int MIDITrailApp::_OnDestroy()
{
	int result = 0;

	//視点保存
	if (m_isAutoSaveViewpoint) {
		result = _OnMenuSaveViewpoint();
		//if (result != 0) goto EXIT;
		//エラーが発生しても処理を続行する
	}

	//演奏を止める
	if (m_PlayStatus == Play) {
		m_Sequencer.Stop();
		//シーケンサ側のスレッド終了を待ち合わせるべきだが手を抜く
		Sleep(100);
	}
	else if (m_PlayStatus == MonitorON) {
		m_LiveMonitor.Stop();
		//厳密にはコールバック関数終了を待ち合わせるべきだが手を抜く
		Sleep(100);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// シーン再生成
//******************************************************************************
int MIDITrailApp::_RebuildScene()
{
	int result = 0;
	int apiresult = 0;
	bool m_isResume = false;
	bool m_isResumeMonitoring = false;
	MTScene::MTViewParamMap viewParamMap;

	//暫定対策
	//  メッセージボックスを表示することにより
	//  ユーザーがOKボタンを押すまでの間に
	//  デバイスがリセット可能状態になることを期待する

	//現在の視点を退避
	if (m_pScene != NULL) {
		m_pScene->GetViewParam(&viewParamMap);
	}

	//演奏を一時停止する
	//  なぜか一時停止しないとデバイスを再生成しても
	//  デバイスロストから復帰できない
	if (m_PlayStatus == Play) {
		m_Sequencer.Pause();
		m_isResume = true;
	}
	else if (m_PlayStatus == MonitorON) {
		//モニタ停止
		result = _OnMenuStopMonitoring();
		if (result != 0) goto EXIT;
		m_isResumeMonitoring = true;
	}

	//メッセージボックス表示
	apiresult = MessageBox(
					m_hWnd,						//オーナーウィンドウ
					MIDITRAIL_MSG_DEVICELOST,	//メッセージ
					_T("WARNING"),				//タイトル
					MB_OK | MB_ICONWARNING		//フラグ
				);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//レンダラとシーンオブジェクトの再生成
	result = _ChangeWindowSize();
	if (result != 0) goto EXIT;

	//シーンの再設定
	if (m_pScene != NULL) {
		//視点を復帰
		m_pScene->SetViewParam(&viewParamMap);

		//演奏中の場合はシーンに演奏開始を通知
		if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
			result = m_pScene->OnPlayStart(m_Renderer.GetDevice());
			if (result != 0) goto EXIT;
		}
		//演奏チックタイム通知
		if (m_SequencerLastMsg.isRecvPlayTime) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.playTime.param1,
							m_SequencerLastMsg.playTime.param2
						);
			if (result != 0) goto EXIT;
		}
		//テンポ変更通知
		if (m_SequencerLastMsg.isRecvTempo) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.tempo.param1,
							m_SequencerLastMsg.tempo.param2
						);
			if (result != 0) goto EXIT;
		}
		//小節番号通知
		if (m_SequencerLastMsg.isRecvBar) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.bar.param1,
							m_SequencerLastMsg.bar.param2
						);
			if (result != 0) goto EXIT;
		}
		//拍子記号変更通知
		if (m_SequencerLastMsg.isRecvBeat) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.beat.param1,
							m_SequencerLastMsg.beat.param2
						);
			if (result != 0) goto EXIT;
		}
		//TODO: ノート数のカウンタ表示が復元できていない
		//TODO: ピッチベンドが復元できていない
	}

	//一時停止した場合は演奏を再開させる
	if (m_isResume) {
		result = m_Sequencer.Resume();
		if (result != 0) goto EXIT;
	}
	else if (m_isResumeMonitoring) {
		//モニタ再開
		result = _OnMenuStartMonitoring();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// HowToView表示
//******************************************************************************
int MIDITrailApp::_DispHowToView()
{
	int result = 0;
	int count = 0;

	result = m_ViewConf.SetCurSection(_T("HowToView"));
	if (result != 0) goto EXIT;

	result = m_ViewConf.GetInt(_T("DispCount"), &count, 0);
	if (result != 0) goto EXIT;

	if (count != 2) {
		//操作方法ダイアログ表示
		m_HowToViewDlg.Show(m_hWnd);
	}

	count = 2;
	result = m_ViewConf.SetInt(_T("DispCount"), count);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択マーク更新
//******************************************************************************
int MIDITrailApp::_UpdateMenuCheckmark()
{
	int result = 0;

	//リピート
	_CheckMenuItem(IDM_REPEAT, m_isRepeat);

	//シーン種別選択
	//TAG:シーン追加
	_CheckMenuItem(IDM_VIEW_3DPIANOROLL, false);
	_CheckMenuItem(IDM_VIEW_2DPIANOROLL, false);
	_CheckMenuItem(IDM_VIEW_PIANOROLLRAIN, false);
	_CheckMenuItem(IDM_VIEW_PIANOROLLRAIN2D, false);
	_CheckMenuItem(IDM_VIEW_PIANOROLLRING, false);
	switch (m_SelectedSceneType) {
		case PianoRoll3D:
			_CheckMenuItem(IDM_VIEW_3DPIANOROLL, true);
			break;
		case PianoRoll2D:
			_CheckMenuItem(IDM_VIEW_2DPIANOROLL, true);
			break;
		case PianoRollRain:
			_CheckMenuItem(IDM_VIEW_PIANOROLLRAIN, true);
			break;
		case PianoRollRain2D:
			_CheckMenuItem(IDM_VIEW_PIANOROLLRAIN2D, true);
			break;
		case PianoRollRing:
			_CheckMenuItem(IDM_VIEW_PIANOROLLRING, true);
			break;
		default:
			result = YN_SET_ERR("Program error.", m_SelectedSceneType, 0);
			goto EXIT;
			break;
	}

	//ピアノキーボード表示
	_CheckMenuItem(IDM_ENABLE_PIANOKEYBOARD, m_isEnablePianoKeyboard);

	//波紋効果
	_CheckMenuItem(IDM_ENABLE_RIPPLE, m_isEnableRipple);

	//ピッチベンド効果
	_CheckMenuItem(IDM_ENABLE_PITCHBEND, m_isEnablePitchBend);

	//星表示
	_CheckMenuItem(IDM_ENABLE_STARS, m_isEnableStars);

	//カウンタ表示
	_CheckMenuItem(IDM_ENABLE_COUNTER, m_isEnableCounter);

	//背景画像表示
	_CheckMenuItem(IDM_ENABLE_BACKGROUNDIMAGE, m_isEnableBackgroundImage);

	//自動視点保存
	_CheckMenuItem(IDM_AUTO_SAVE_VIEWPOINT, m_isAutoSaveViewpoint);

	//フルスクリーン
	_CheckMenuItem(IDM_FULLSCREEN, m_isFullScreen);
	
EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択マーク設定
//******************************************************************************
void MIDITrailApp::_CheckMenuItem(
		UINT uIDCheckItem,
		bool isEnable
	)
{
	UINT uCheck = 0;

	if (isEnable) {
		uCheck = MF_CHECKED;
	}
	else {
		uCheck = MF_UNCHECKED;
	}

	CheckMenuItem(GetMenu(m_hWnd), uIDCheckItem, MF_BYCOMMAND | uCheck);

	return;
}

//******************************************************************************
// 表示効果反映
//******************************************************************************
void MIDITrailApp::_UpdateEffect()
{
	if (m_pScene != NULL) {
		m_pScene->SetEffect(MTScene::EffectPianoKeyboard, m_isEnablePianoKeyboard);
		m_pScene->SetEffect(MTScene::EffectRipple, m_isEnableRipple);
		m_pScene->SetEffect(MTScene::EffectPitchBend, m_isEnablePitchBend);
		m_pScene->SetEffect(MTScene::EffectStars, m_isEnableStars);
		m_pScene->SetEffect(MTScene::EffectCounter, m_isEnableCounter);
		m_pScene->SetEffect(MTScene::EffectFileName, m_isEnableFileName);
		m_pScene->SetEffect(MTScene::EffectBackgroundImage, m_isEnableBackgroundImage);
	}
	return;
}

//******************************************************************************
// コマンドライン解析
//******************************************************************************
int MIDITrailApp::_ParseCmdLine(
		LPTSTR pCmdLine
	)
{
	int result = 0;

	//コマンドライン解析
	result = m_CmdLineParser.Initialize(pCmdLine);
	if (result != 0) goto EXIT;

	//コマンドラインでファイルを指定されている場合
	if (m_CmdLineParser.GetSwitch(CMDSW_FILE_PATH) == CMDSW_ON) {

		//ファイルを開く
		result = _LoadMIDIFile(m_CmdLineParser.GetFilePath());
		if (result != 0) goto EXIT;

		//再生指定されている場合は再生開始
		if ((m_CmdLineParser.GetSwitch(CMDSW_PLAY) == CMDSW_ON) ||
		    (m_AutoPlaybackAfterOpenFile > 0)) {
			result = _OnMenuPlay();
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// タイマー開始
//******************************************************************************
int MIDITrailApp::_StartTimer()
{
	int result = 0;
	UINT_PTR apiresult = 0;

	//キー状態確認タイマー
	apiresult = SetTimer(
						m_hWnd,			//通知先ウィンドウ
						MIDITRAIL_TIMER_CHECK_KEY,	//タイマーID
						200,			//タイムアウト値（ミリ秒）
						NULL			//タイマー関数
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// タイマー停止
//******************************************************************************
int MIDITrailApp::_StopTimer()
{
	int result = 0;

	KillTimer(m_hWnd, MIDITRAIL_TIMER_CHECK_KEY);

	return result;
}

//******************************************************************************
// タイマー呼び出し
//******************************************************************************
int MIDITrailApp::_OnTimer(
		WPARAM timerId
	)
{
	int result = 0;

	//キー状態確認タイマー
	if (timerId == MIDITRAIL_TIMER_CHECK_KEY) {
		//再生速度制御
		if ((GetKeyState(VK_F2) & 0x8000) && (GetForegroundWindow() == m_hWnd)) {
			m_Sequencer.SetPlaybackSpeed(2);  //2倍速
		}
		else {
			m_Sequencer.SetPlaybackSpeed(1);
		}
	}

	return result;
}

//******************************************************************************
// レンダラチェック
//******************************************************************************
int MIDITrailApp::_CheckRenderer()
{
	int result = 0;
	bool isSupport = true;
	unsigned long maxVertexIndex = 0;

	result = m_Renderer.IsSupportIndexBuffer(&isSupport, &maxVertexIndex);
	if (result != 0) goto EXIT;

	//インデックスバッファをサポートしていない場合は警告メッセージを表示
	if (!isSupport) {
		YN_SET_WARN("This PC does not have sufficient graphics capabilities.\n"
					"Therefore, MIDITrail will not work correctly.",
					maxVertexIndex, 0);
		YN_SHOW_ERR(NULL);
		//戻り値には反映せず処理を続行させる
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI OUT 自動設定
//******************************************************************************
int MIDITrailApp::_AutoConfigMIDIOUT()
{
	int result = 0;
	int apiresult = 0;
	TCHAR devName[MAXPNAMELEN];
	TCHAR message[512];
	int autoConfigConfirm = 0;
	std::string productName;

	//カテゴリ／セクション設定
	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//設定ファイルから MIDI OUT ユーザ選択デバイス名を取得
	result = m_MIDIConf.GetStr("PortA", devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;

	if (_tcslen(devName) == 0) {
		//設定なしの場合
		result = m_MIDIConf.GetInt("AutoConfigConfirm", &autoConfigConfirm, 0);
		if (result != 0) goto EXIT;

		if (autoConfigConfirm == 0) {
			//自動設定未確認の場合はMIDI OUTデバイスを自動設定する
			result = m_MIDIConf.SetInt("AutoConfigConfirm", 1);
			if (result != 0) goto EXIT;

			//Microsoft GS Wavetable Synthを検索
			result = _SearchMicrosoftWavetableSynth(productName);
			if (result != 0) goto EXIT;

			//見つかった場合はMIDI OUTデバイスに登録する
			if (productName.size() > 0) {
				result = m_MIDIConf.SetStr("PortA", productName.c_str());
				if (result != 0) goto EXIT;

				//自動設定確認アラートパネル表示
				_stprintf_s(
						message,
						512,
						_T("MIDITrail selected %s to MIDI OUT.\n")
						_T("If you have any other MIDI device, please configure MIDI OUT."),
						productName.c_str()
					);
				apiresult = MessageBox(
								m_hWnd,						//オーナーウィンドウ
								message,					//メッセージ
								_T("INFORMATION"),			//タイトル
								MB_OK | MB_ICONINFORMATION	//フラグ
							);
				if (apiresult == 0) {
					result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
					goto EXIT;
				}
			}
		}
		else {
			//自動設定確認済みのため何もしない
		}
	}
	else {
		//設定ありの場合
		result = m_MIDIConf.SetInt("AutoConfigConfirm", 1);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Microsoft GS Wavetable Synth検索
//******************************************************************************
int MIDITrailApp::_SearchMicrosoftWavetableSynth(
		std::string& productName
	)
{
	int result = 0;
	unsigned long index = 0;
	std::string name;
	std::string target;
	string::size_type pos;
	SMOutDevCtrl outDevCtrl;

	//検索対象MIDIデバイス
	//  Windows XP以前    : Microsoft GS Wavetable SW Synth
	//  Windows Vista以降 : Microsoft GS Wavetable Synth

	//検索対象文字列
	target = "Microsoft GS Wavetable";

	result = outDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	productName = "";
	for (index = 0; index < outDevCtrl.GetDevNum(); index++) {
		result = outDevCtrl.GetDevProductName(index, name);
		if (result != 0) goto EXIT;

		pos = name.find(target);
		if (pos != string::npos) {
			//見つかった
			productName = name;
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 二重起動チェック
//******************************************************************************
int MIDITrailApp::_CheckMultipleInstances(
		 bool* pIsExitApp
	)
{
	int result = 0;
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES secAttribute;

	*pIsExitApp = false;

	//二重起動を許可する場合は何もしない
	if (m_AllowMultipleInstances > 0) {
		goto EXIT;
	}

	//セキュリティ記述子初期化
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

	//セキュリティ記述子に随意アクセス制御リスト(DACL)を設定
	SetSecurityDescriptorDacl(
			&sd,	//セキュリティ記述子のアドレス
			TRUE,	//DACLの存在フラグ
			NULL,	//DACLのアドレス：オブジェクトへのすべてのアクセスを許可
			FALSE	//DACLの既定フラグ
		);

	//セキュリティ属性
	secAttribute.nLength = sizeof(SECURITY_ATTRIBUTES);	//構造体サイズ
	secAttribute.lpSecurityDescriptor = &sd;			//セキュリティ記述子
	secAttribute.bInheritHandle = TRUE; 				//継承フラグ

	//ミューテクス作成
	//  「別のユーザーとして実行」を選択したときミューテクス作成が失敗するため
	//  セキュリティ属性を指定する
	m_hAppMutex = CreateMutex(
						&secAttribute,	//セキュリティ属性
						FALSE,			//オブジェクトの所有権を取得しない
						MIDITRAIL_MUTEX	//オブジェクト名称
					);
	if (m_hAppMutex == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	else if (GetLastError() ==  ERROR_ALREADY_EXISTS) {
		//すでに存在する場合
		CloseHandle(m_hAppMutex);
		m_hAppMutex = NULL;
		*pIsExitApp = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メールスロット作成
//******************************************************************************
int MIDITrailApp::_CreateMailSlot()
{
	int result = 0;

	//二重起動を許可する場合は何もしない
	if (m_AllowMultipleInstances > 0) {
		goto EXIT;
	}

	//メールスロット作成
	m_hMailSlot = CreateMailslot(
						MIDITRAIL_MAILSLOT,	//メールスロット名称
						1024,				//最大メッセージサイズ(byte)：制限なし
						0,					//読み取りタイムアウト値(ms)：メッセージがなければ即座に制御を返す
						NULL				//継承オプション
					);
	if (m_hMailSlot == INVALID_HANDLE_VALUE) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 先行プロセスのMIDITrailへファイルパスをポスト
//******************************************************************************
int MIDITrailApp::_PostFilePathToFirstMIDITrail(
		LPTSTR pCmdLine
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	HWND hWnd = NULL;
	HANDLE hFile = NULL;
	size_t size = 0;
	DWORD written = 0;
	TCHAR* pFilePart = NULL;
	TCHAR filePath[_MAX_PATH] = {_T('\0')};

	//先行のMIDITrailのウィンドウを検索する
	hWnd = FindWindow(
				m_WndClassName,	//クラス名
				NULL			//ウィンドウ名
			);
	if (hWnd == NULL) {
		//ウィンドウが見つからない場合は何もしない
		goto EXIT;
	}

	//先行のMIDITrailのウィンドウを前面に移動
	SetForegroundWindow(hWnd);

	//コマンドライン解析
	result = m_CmdLineParser.Initialize(pCmdLine);
	if (result != 0) goto EXIT;

	//コマンドラインでファイルを指定されていなければ何もしない
	if (m_CmdLineParser.GetSwitch(CMDSW_FILE_PATH) != CMDSW_ON) {
		goto EXIT;
	}

	//ファイルパスをフルパスに変換
	written = GetFullPathName(
					m_CmdLineParser.GetFilePath(),	//ファイルパス
					_MAX_PATH,		//バッファサイズ：TCHAR単位
					filePath,		//バッファ位置
					&pFilePart		//ファイル名の位置
				);
	if (written == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	else if (written > _MAX_PATH) {
		result = YN_SET_ERR("File path is too long.", written, 0);
		goto EXIT;
	}

	//先行起動プロセスのメールスロットを開く
	hFile = CreateFile(
				MIDITRAIL_MAILSLOT,		//メールスロット名称
				GENERIC_WRITE,			//アクセスタイプ
				FILE_SHARE_READ,		//共有方法
				NULL,					//セキュリティ属性
				OPEN_EXISTING,			//生成指定
				FILE_ATTRIBUTE_NORMAL,	//ファイル属性とフラグ
				NULL					//テンプレートファイルハンドル
			);
	if (hFile == INVALID_HANDLE_VALUE) {
		//メールスロットが開けない場合は何もしない
		//先行プロセスの状態に依存するため失敗する可能性はある
		goto EXIT;
	}

	//メールスロットにファイルパスを書き込む
	//_tcscat_s(filePath, _MAX_PATH, m_CmdLineParser.GetFilePath());
	size = (_tcslen(filePath) + 1) * sizeof(TCHAR);
	bresult = WriteFile(
				hFile,		//ファイルハンドル
				filePath,	//データバッファ
				(DWORD)size,	//書き込みサイズ(byte)
				&written,	//書き込んだサイズ(byte)
				NULL		//オーバーラップ構造体
			);
	if (!bresult) {
		//書き込めなかった場合は何もしない
		//先行プロセスの状態に依存するため失敗する可能性はある
		goto EXIT;
	}

	//先行のMIDITrailのウィンドウにファイルパスポスト通知
	PostMessage(hWnd, WM_FILEPATH_POSTED, 0, 0);

EXIT:;
	if (hWnd != NULL) {
		CloseHandle(hWnd);
	}
	if (hFile != NULL) {
		CloseHandle(hFile);
	}
	return result;
}

//******************************************************************************
// 後続プロセスからのファイルパスポスト通知
//******************************************************************************
int MIDITrailApp::_OnFilePathPosted()
{
	int result = 0;
	BOOL bresult = FALSE;
	DWORD nextSize = 0;
	DWORD readSize = 0;
	DWORD count = 0;
	TCHAR filePath[_MAX_PATH + 4];

	ZeroMemory(filePath, sizeof(TCHAR)*(_MAX_PATH + 4));

	//メールスロットが存在しなければ何もしない
	if (m_hMailSlot == NULL) goto EXIT;

	//メールスロットからファイルパスを取得
	bresult = GetMailslotInfo(
					m_hMailSlot,	//メールスロット
					NULL,			//最大メッセージサイズ
					&nextSize,		//次メッセージサイズ
					&count,			//メッセージ数
					NULL			//読み取りタイムアウト時間
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//メッセージがなければ何もしない
	if (nextSize == MAILSLOT_NO_MESSAGE) goto EXIT;

	//メッセージサイズの整合性チェック
	if (nextSize > (sizeof(TCHAR)*1024)) {
		result = YN_SET_ERR("Program error.", nextSize, 0);
		goto EXIT;
	}

	//メッセージ読み込み
	bresult = ReadFile(
					m_hMailSlot,	//メールスロット
					filePath,		//バッファ
					nextSize,		//読み取りサイズ(byte)
					&readSize,		//読み取ったサイズ(byte)
					NULL			//オーバーラップ構造体
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//演奏/モニタ停止とファイルオープン処理
	result = _StopPlaybackAndOpenFile(filePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏/モニタ停止とMIDIファイルオープン処理
//******************************************************************************
int MIDITrailApp::_StopPlaybackAndOpenFile(
		TCHAR* pFilePath
	)
{
	int result = 0;

	//演奏ステータスごとの対応方式
	//  データ無   → すぐにファイルを開く
	//  停止       → すぐにファイルを開く
	//  再生中     → シーケンサに停止要求を出す → 停止通知を受けた後にファイルを開く
	//  一時停止   → シーケンサに停止要求を出す → 停止通知を受けた後にファイルを開く
	//  モニタ停止 → すぐにファイルを開く
	//  モニタ中   → モニタを停止してモニタ停止状態へ遷移 → すぐにファイルを開く

	//視点保存
	if (m_isAutoSaveViewpoint) {
		result = _OnMenuSaveViewpoint();
		if (result != 0) goto EXIT;
	}

	//モニタ中であれば停止する
	if (m_PlayStatus == MonitorON) {
		result = _OnMenuStopMonitoring();
		if (result != 0) goto EXIT;
		//この時点でモニタ停止に遷移済み
	}

	//停止中であればすぐにファイルを開く
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//ファイル読み込み処理
		result = _FileOpenProc(pFilePath);
		if (result != 0) goto EXIT;
	}
	//演奏中の場合は演奏停止後にファイルを開く
	else if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//演奏状態通知が届くまで再生中とみなす
		//ここでは演奏状態を変更しない
		m_Sequencer.Stop();
		
		//停止完了後にファイルを開く
		_tcscpy_s(m_NextFilePath, _MAX_PATH, pFilePath);
		m_isOpenFileAfterStop = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDIファイルオープン処理
//******************************************************************************
int MIDITrailApp::_FileOpenProc(
		TCHAR* pFilePath
	)
{
	int result = 0;

	//MIDIファイル読み込み
	result = _LoadMIDIFile(pFilePath);
	if (result != 0) goto EXIT;

	//HowToView表示
	result = _DispHowToView();
	if (result != 0) goto EXIT;

	//再生指定されている場合は再生開始
	if (m_AutoPlaybackAfterOpenFile > 0) {
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// フルスクリーン切替
//******************************************************************************
int MIDITrailApp::_ToggleFullScreen()
{
	int result = 0;
	
	m_isFullScreen = m_isFullScreen ? false : true;
	
	result = _ChangeWindowSize();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// メニュー表示
//******************************************************************************
int MIDITrailApp::_ShowMenu()
{
	int result = 0;
	LONG apiresult = 0;
	
	//メニューバー表示処理
	if (GetMenu(m_hWnd) == NULL) {
		apiresult = SetMenu(m_hWnd, m_hMenu);
		if (apiresult == 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
	}

	//メニュー選択マーク更新
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//メニュースタイル更新
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー非表示
//******************************************************************************
int MIDITrailApp::_HideMenu()
{
	int result = 0;
	LONG apiresult = 0;

	//メニューバー非表示処理
	//すでにメニューバー非表示なら何もしない
	if (GetMenu(m_hWnd) != NULL) {
		//GetMenuで取得したハンドルは破棄されない
		apiresult = SetMenu(m_hWnd, NULL);
		if (apiresult == 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ゲームパッド操作処理
//******************************************************************************
int MIDITrailApp::_GamePadProc()
{
	int result = 0;

	result = m_GamePadCtrl.UpdateState();
	if (result != 0) goto EXIT;
	
	//_RPTN(_CRT_WARN, "GamePad: %d %d\n", m_GamePadCtrl.DidPressNow_A(), m_GamePadCtrl.DidPressNow_B());

	//スタート 押下
	if (m_GamePadCtrl.DidPressNow_Start()) {
		//演奏開始／一時停止
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}

	//ボタンA 押下
	if (m_GamePadCtrl.DidPressNow_A()) {
		//演奏開始／一時停止
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}
	
	//ボタンB 押下
	if (m_GamePadCtrl.DidPressNow_B()) {
		//演奏停止
		result = _OnMenuStop();
		if (result != 0) goto EXIT;
	}
	
	//左ショルダー 押下
	if (m_GamePadCtrl.DidPressNow_LShoulder()) {
		//視点切り替え
		result = _ChangeViewPoint(-1);
		if (result != 0) goto EXIT;
	}
	
	//右ショルダー 押下
	if (m_GamePadCtrl.DidPressNow_RShoulder()) {
		//視点切り替え
		result = _ChangeViewPoint(+1);
		if (result != 0) goto EXIT;
	}
	
	//左トリガー 押下
	if (m_GamePadCtrl.DidPressNow_LTrigger()) {
		//再生リワインド
		result = _OnMenuSkipBack();
		if (result != 0) goto EXIT;
	}
	
	//右トリガー 押下
	if (m_GamePadCtrl.DidPressNow_RTrigger()) {
		//再生スキップ
		result = _OnMenuSkipForward();
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// 視点切り替え
//******************************************************************************
int MIDITrailApp::_ChangeViewPoint(int step)
{
	int result = 0;

	//ゲームパッド用視点番号更新
	m_GamePadViewPointNo += step;

	if (m_GamePadViewPointNo < 0) {
		m_GamePadViewPointNo = 2;
	}
	else if (m_GamePadViewPointNo > 2) {
		m_GamePadViewPointNo = 0;
	}

	//視点切り替え
	switch (m_GamePadViewPointNo) {
	case 0:
		result = _OnMenuResetViewpoint();
		if (result != 0) goto EXIT;
		break;
	case 1:
		result = _OnMenuViewpoint(2);
		if (result != 0) goto EXIT;
		break;
	case 2:
		result = _OnMenuViewpoint(3);
		if (result != 0) goto EXIT;
		break;
	default:
		break;
	}

EXIT:;
	return result;
}
