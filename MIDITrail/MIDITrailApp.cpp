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
#include "ShObjIdl.h"   // IFileOpenDialog (folder select) - 1.4.1 feature port
#include "imgui.h"      // ImGui IO (config manager input capture guard)
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "DXColorUtil.h"
#include "MTVideoExporter.h"
#include "MTVideoExportDlg.h"
#include "SMSimpleList.h"
#include "MIDITrailApp.h"
#include <vector>
// M5: DX9 scene tree removed from the DX11 build. _CreateScene is a stub
// (m_pScene stays NULL), so the concrete DX9 scene classes are no longer
// referenced and their headers are not included here.
#include "MIDITrailVersion.h"
// >>> add 20231016 c30 begin
#include "imgui.h"
#include "imgui_impl_win32.h"
#include <locale.h>
// <<< add 20231016 c30 end

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
	m_isLoading = false;
	m_isExporting = false;
	m_ExportErrorMsg[0] = _T('\0');
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
	m_LoadFilePathW[0] = L'\0';
	m_MultiSampleType = 0;
	m_SuperSample = 1;   //ced 20260628

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
	m_isEnableLyrics = true;
	m_isEnablePitchBend = true;
	m_isEnablePitchBendAllNotes = false;
	m_isEnableStars = true;
	m_isEnableCounter = true;
	m_isEnableFileName = false;
	m_isEnableBackgroundImage = true;
// >>> add 20180404 yossiepon begin
	m_isEnableTimeIndicator = true;
	m_isEnableGridBox = true;
// <<< add 20180404 yossiepon end

	//シーン種別
	m_SceneType = Title;
	m_SelectedSceneType = PianoRoll3D;

	//M3 (DX11) camera toggles
	m_IsMouseCamMode11 = false;
	m_IsAutoRollMode11 = false;
	m_CfgWasVisible = false;        //ced 20260629
	m_MouseCamBeforeCfg = false;    //ced 20260629
	m_HasPrevView = false;
	m_PrevViewSceneType = PianoRoll3D;
	m_PrevViewIsLive = false;

	//M4 (DX11) dashboard NPS
	m_NpsNoteCount = 0;
	m_NpsLastSec = 0;
	m_DX11Family = DX11_FAMILY_NONE;
	m_CurSongBPM = 120;
	m_LiveNoteCount = 0;
	m_IsSingleKeyboard11 = true;   //M4.6c: default to a single keyboard

	//自動視点保存
	m_isAutoSaveViewpoint = false;
	m_isAutoSaveViewSettings = false;  //ced 20260628

	//フォルダ演奏 / メニューバー（1.4.1 移植）
	m_isFolderPlayback = false;
	m_isEnableMenuBar = true;
	m_DelayBetweenSongsInMsec = 0;

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

	// Data
	static LPDIRECT3D9              g_pD3D = nullptr;
	static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
	static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
	static D3DPRESENT_PARAMETERS    g_d3dpp = {};
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

	setlocale(LC_ALL, "Japanese");

	//文字列初期化
	LoadString(hInstance, IDS_APP_TITLE, m_Title, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MIDITRAIL, m_WndClassName, MAX_LOADSTRING);

//>>> add yossiepon 20190828
	TCHAR* pVersion = NULL;

	//バージョン文字列
#ifdef _WIN64
	//64bit
	pVersion = MIDITRAIL_VERSION_STRING_X64;
#else
	//32bit
	pVersion = MIDITRAIL_VERSION_STRING_X86;
#endif
	_tcscat_s(m_Title, MAX_LOADSTRING, _T(" "));
	_tcscat_s(m_Title, MAX_LOADSTRING, pVersion);
//<<< add yossiepon 20190828

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
	result = m_Renderer11.Initialize(m_hWnd, m_MultiSampleType, false, m_SuperSample);
	if (result != 0) goto EXIT;

	//M4: DX11 dashboard (created now, but only shown once a song is loaded -
	//not on the startup / title screen; SetDashboard11 happens on load)
	if (m_Renderer11.GetDevice() != NULL) {
		m_Dashboard11.Create(m_Renderer11.GetDevice(), m_Renderer11.GetContext());
	}

	//設定マネージャ（conf/*.ini を GUI 編集：Mod Mod 独自）を初期化しレンダラへ登録
	m_ConfigMgr11.Initialize();
	m_Renderer11.SetConfigManager11(&m_ConfigMgr11);

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

	m_Renderer11.Terminate();


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
		else {
			if (true) {  /* DX11: always pump sequencer msgs */
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
			}

			//M3 (DX11): poll the live playback tick straight from the sequencer
			//(independent of the message queue, so heavy-load message drops
			//cannot stall the scroll or the keyboard key-press state)
			{
				//live monitor: the live components are timeGetTime-driven and the
				//camera must NOT scroll to the (stale) sequencer position left by the
				//previous playback, so feed tick/ms 0 while monitoring.
				//Playback/Stop both follow the sequencer's reported position. The "weird
				//position after stop+load" case is fixed at the source: loading a new song
				//(and the manual Stop) rewinds the sequencer to 0 (m_Sequencer.Rewind),
				//while a natural end leaves it at the last tick (DX9 keeps the view there
				//and only rewinds on the next Play).
				bool liveMon = (m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF);
				unsigned long tick = liveMon ? 0 : m_Sequencer.GetCurrentTickTime();
				unsigned long playMs = liveMon ? 0 : m_Sequencer.GetCurrentPlayTimeMSec();
				_FeedDX11Tick(tick, playMs);
			}

			GetWindowPlacement(m_hWnd, &wndpl);
			if ((wndpl.showCmd != SW_HIDE) &&
				(wndpl.showCmd != SW_MINIMIZE) &&
				(wndpl.showCmd != SW_SHOWMINIMIZED) &&
				(wndpl.showCmd != SW_SHOWMINNOACTIVE)) {
				//Config Manager(ImGui) 表示中は DirectInput によるカメラ操作を止める
				//（マウス/キー/パッドが裏で効くのを防ぐ。自動スクロール/ロールは継続）。
				//マウスカメラモードが ON なら解除してカーソルを ImGui 操作可能に戻す。
				{
					bool cfgVisible = m_ConfigMgr11.IsVisible();
					m_FpCam11.SetInputEnabled(!cfgVisible);
					//ced 20260629: 開いた瞬間はマウスカメラを退避して解除（カーソルを使えるように）、
					//閉じた瞬間に元の状態へ復元する。これで「開いて閉じると見回せない」を防ぐ。
					if (cfgVisible && !m_CfgWasVisible) {
						m_MouseCamBeforeCfg = m_IsMouseCamMode11;
						if (m_IsMouseCamMode11) {
							m_IsMouseCamMode11 = false;
							m_FpCam11.SetMouseCamMode(false);
						}
					}
					else if (!cfgVisible && m_CfgWasVisible) {
						if (m_MouseCamBeforeCfg && (m_PlayStatus != NoData)) {
							m_IsMouseCamMode11 = true;
							m_FpCam11.SetMouseCamMode(true);
						}
					}
					m_CfgWasVisible = cfgVisible;
				}
				//描画
				result = m_Renderer11.RenderScene(m_pScene);
				if (result != 0) {
					if (result == DXRENDERER11_ERR_DEVICE_LOST) {
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

				//Config Manager で .ini を保存したら現シーンを再構築して反映する
				if (m_ConfigMgr11.ConsumeApplyRequest()) {
					result = _ChangeWindowSize();
					if (result != 0) {
						YN_SHOW_ERR(m_hWnd);
						PostMessage(m_hWnd, WM_DESTROY, 0, 0);
					}
				}
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
// >>> modify 20120728 yossiepon begin
	// COLOR+WINDOW+1だとちらつき時に白一色になって目立つので、背景色を黒にしてちらつきを抑える
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH); //(COLOR_WINDOW+1);			//背景用ブラシハンドル
// <<< modify 20120728 yossiepon end
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
	
	//メニューバー表示（ユーザがメニューバーを無効化していれば隠す：1.4.1 移植）
	if (m_isEnableMenuBar) {
		result = _ShowMenu();
	}
	else {
		result = _HideMenu();
	}
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

	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return false;

	//Config Manager (ImGui) を操作中だけ、その入力をアプリ側（カメラ操作・ショート
	//カット）に渡さない。非表示時は一切ガードしない（マウス視点移動などを阻害しない）。
	if (m_ConfigMgr11.IsVisible()) {
		ImGuiIO& io = ImGui::GetIO();
		switch (message) {
			case WM_LBUTTONDOWN: case WM_LBUTTONUP:
			case WM_RBUTTONDOWN: case WM_RBUTTONUP:
			case WM_MBUTTONDOWN: case WM_MBUTTONUP:
			case WM_MOUSEMOVE:   case WM_MOUSEWHEEL:
				if (io.WantCaptureMouse) return DefWindowProc(hWnd, message, wParam, lParam);
				break;
			case WM_KEYDOWN: case WM_KEYUP: case WM_CHAR: case WM_SYSKEYDOWN:
				if (io.WantCaptureKeyboard) return DefWindowProc(hWnd, message, wParam, lParam);
				break;
			default:
				break;
		}
	}

	//Ignore user input and scene-affecting messages while a file is loading
	if (m_isLoading) {
		switch (message) {
			case WM_COMMAND:
			case WM_KEYDOWN:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_MOUSEMOVE:
			case WM_DROPFILES:
			case WM_SIZE:
			case WM_TIMER:
			case WM_FILEPATH_POSTED:
				return DefWindowProc(hWnd, message, wParam, lParam);
			default:
				break;
		}
	}

	switch (message) {
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			//M6: ignore menu commands while a video export is running (the export
			//pumps messages for its progress UI, so a menu action would otherwise
			//run mid-export and corrupt the scene/sequencer state).
			if (m_isExporting) break;
			switch (wmId) {
				case IDM_OPEN_FILE:
					//ファイルオープン
					result = _OnMenuFileOpen();
					if (result != 0) goto EXIT;
					break;
// >>> add 20120728 yossiepon begin
				case IDM_ADD_FILE:
					//ファイル追加
					result = _OnMenuFileAdd();
					if (result != 0) goto EXIT;
					break;
// <<< add 20120728 yossiepon end
// >>> add ced 20260627: 1.4.1 features
				case IDM_OPEN_FOLDER:
					result = _OnMenuOpenFolder();
					if (result != 0) goto EXIT;
					break;
				case IDM_PREVIOUS_FILE:
					result = _OnMenuPreviousFile();
					if (result != 0) goto EXIT;
					break;
				case IDM_NEXT_FILE:
					result = _OnMenuNextFile();
					if (result != 0) goto EXIT;
					break;
				case IDM_FOLDER_PLAYBACK:
					result = _OnMenuFolderPlayback();
					if (result != 0) goto EXIT;
					break;
				case IDM_MENUBAR:
					result = _OnMenuMenuBar();
					if (result != 0) goto EXIT;
					break;
				case IDM_MYVIEWPOINT1:
					result = _OnMenuMyViewpoint(1);
					if (result != 0) goto EXIT;
					break;
				case IDM_MYVIEWPOINT2:
					result = _OnMenuMyViewpoint(2);
					if (result != 0) goto EXIT;
					break;
				case IDM_MYVIEWPOINT3:
					result = _OnMenuMyViewpoint(3);
					if (result != 0) goto EXIT;
					break;
				case IDM_SAVE_MYVIEWPOINT1:
					result = _OnMenuSaveMyViewpoint(1);
					if (result != 0) goto EXIT;
					break;
				case IDM_SAVE_MYVIEWPOINT2:
					result = _OnMenuSaveMyViewpoint(2);
					if (result != 0) goto EXIT;
					break;
				case IDM_SAVE_MYVIEWPOINT3:
					result = _OnMenuSaveMyViewpoint(3);
					if (result != 0) goto EXIT;
					break;
// <<< add ced 20260627
				case IDM_EXPORT_VIDEO:
					//M6: �����o��
					result = _OnMenuExportVideo();
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
				case IDM_VIEW_SINGLEKEYBOARD:
					//M4.6c (DX11): toggle single vs per-port keyboard, rebuild scene
					result = _OnMenuToggleSingleKeyboard();
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
				case IDM_ENABLE_LYRICS:
					//表示効果：歌詞
					result = _OnMenuEnableEffect(MTScene::EffectLyrics);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_PITCHBEND:
					//表示効果：ピッチベンド
					result = _OnMenuEnableEffect(MTScene::EffectPitchBend);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_PITCHBEND_ALLNOTES:
					//M4.22: pitch bend the whole channel (all notes, not just sounding)
					result = _OnMenuTogglePitchBendAllNotes();
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
// >>> add 20180404 yossiepon begin
				case IDM_ENABLE_TIMEINDICATOR:
					//表示効果：タイムインジケータ
					result = _OnMenuEnableEffect(MTScene::EffectTimeIndicator);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_GRIDBOX:
					//表示効果：グリッドボックス
					result = _OnMenuEnableEffect(MTScene::EffectGridBox);
					if (result != 0) goto EXIT;
					break;
// <<< add 20180404 yossiepon end
				case IDM_AUTO_SAVE_VIEWPOINT:
					//自動視点保存の有効／無効を切り替え
					result = _OnMenuAutoSaveViewpoint();
					if (result != 0) goto EXIT;
					break;
// >>> add ced 20260628: View 表示設定の自動保存の有効／無効を切り替え
				case IDM_AUTO_SAVE_VIEWSETTINGS:
					result = _OnMenuAutoSaveViewSettings();
					if (result != 0) goto EXIT;
					break;
// <<< add ced 20260628
				//視点保存（手動）は廃止
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
// >>> add ced 20260627: 1.4.1 colour palette config
				case IDM_OPTION_COLOR:
					//カラー設定
					result = _OnMenuOptionColor();
					if (result != 0) goto EXIT;
					break;
// <<< add ced 20260627
// >>> add ced 20260627: config manager (conf/*.ini GUI editor)
				case IDM_OPTION_CONFIGMANAGER:
					result = _OnMenuConfigManager();
					if (result != 0) goto EXIT;
					break;
// <<< add ced 20260627
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

// >>> add 20120728 yossiepon begin

//******************************************************************************
// ファイル追加
//******************************************************************************
int MIDITrailApp::_OnMenuFileAdd()
{
	int result = 0;
	TCHAR filePath[MAX_PATH] = {_T('\0')};
	bool isSelected = false;

	//演奏中はファイルオープンさせない
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//ファイルオープンOK
	}
	else {
		//ファイルオープンNG
		goto EXIT;
	}

	//ファイル選択ダイアログ表示
	result = _SelectMIDIFile(filePath, MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//ファイル選択時の処理
	if (isSelected) {
		//MIDIファイル読み込み処理
		result = _AddMIDIFile(filePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

// <<< add 20120728 yossiepon end

// >>> add ced 20260627: upstream 1.4.1 features ported to the DX11 / MBCS app
//******************************************************************************
// wide path -> DX11 char load pipeline bridge
//   The DX11 app is MBCS (char). Folder/file navigation yields Unicode (WCHAR)
//   paths; route them via m_LoadFilePathW so _LoadMIDIFile uses LoadW (Unicode
//   preserved), with a CP_ACP char copy for the existing char-based open chain.
//******************************************************************************
int MIDITrailApp::_OpenFileW(const WCHAR* pFilePathW)
{
	int result = 0;
	TCHAR filePath[_MAX_PATH] = {_T('\0')};

	if (pFilePathW == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	wcscpy_s(m_LoadFilePathW, MAX_PATH, pFilePathW);
	WideCharToMultiByte(CP_ACP, 0, pFilePathW, -1, filePath, _MAX_PATH, NULL, NULL);
	result = _StopPlaybackAndOpenFile(filePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：フォルダを開く
//******************************************************************************
int MIDITrailApp::_OnMenuOpenFolder()
{
	int result = 0;
	WCHAR folderPath[_MAX_PATH] = { L'\0' };
	bool isSelected = false;

	result = _SelectFolder(folderPath, _MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	if (isSelected) {
		//フルスクリーン時はシーン生成がクライアント領域を参照するため一旦メニューを隠す
		if (m_isFullScreen) {
			_HideMenu();
		}
		result = _StopPlaybackAndOpenFolder(folderPath);
		if (result != 0) goto EXIT;
	}

	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：前ファイル
//******************************************************************************
int MIDITrailApp::_OnMenuPreviousFile()
{
	int result = 0;
	bool isExist = false;
	const WCHAR* pFilePath = NULL;

	if (m_MIDIFileList.GetFileCount() == 0) goto EXIT;

	m_MIDIFileList.SelectPreviousFile(&isExist);
	if (isExist) {
		pFilePath = m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex());
		result = _OpenFileW(pFilePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：次ファイル
//******************************************************************************
int MIDITrailApp::_OnMenuNextFile()
{
	int result = 0;
	bool isExist = false;
	const WCHAR* pFilePath = NULL;

	if (m_MIDIFileList.GetFileCount() == 0) goto EXIT;

	m_MIDIFileList.SelectNextFile(&isExist);
	if (isExist) {
		pFilePath = m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex());
		result = _OpenFileW(pFilePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：フォルダ演奏切替
//******************************************************************************
int MIDITrailApp::_OnMenuFolderPlayback()
{
	int result = 0;
	m_isFolderPlayback = m_isFolderPlayback ? false : true;
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;
EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：メニューバー表示切替
//******************************************************************************
int MIDITrailApp::_OnMenuMenuBar()
{
	int result = 0;
	result = _ToggleMenuBar();
	if (result != 0) goto EXIT;
EXIT:;
	return result;
}

//******************************************************************************
// メニューバー表示切替
//******************************************************************************
int MIDITrailApp::_ToggleMenuBar()
{
	int result = 0;
	m_isEnableMenuBar = m_isEnableMenuBar ? false : true;
	if (m_isEnableMenuBar) {
		result = _ShowMenu();
	}
	else {
		result = _HideMenu();
	}
	if (result != 0) goto EXIT;
	result = _ChangeWindowSize();
	if (result != 0) goto EXIT;
EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：MyViewpoint へ移動
//******************************************************************************
int MIDITrailApp::_OnMenuMyViewpoint(unsigned long viewpointNo)
{
	int result = 0;
	if (m_PlayStatus == NoData) goto EXIT;
	result = _MoveToMyViewpoint(viewpointNo);
	if (result != 0) goto EXIT;
EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：MyViewpoint 保存
//******************************************************************************
int MIDITrailApp::_OnMenuSaveMyViewpoint(unsigned long viewpointNo)
{
	int result = 0;
	if (m_PlayStatus == NoData) goto EXIT;
	result = _SaveMyViewpoint(viewpointNo);
	if (result != 0) goto EXIT;
EXIT:;
	return result;
}

//******************************************************************************
// MyViewpoint へ移動（設定ファイルから視点を復元）
//******************************************************************************
int MIDITrailApp::_MoveToMyViewpoint(unsigned long viewpointNo)
{
	int result = 0;
	const float SENT = -1.0e30f;
	const TCHAR* pName = NULL;
	TCHAR section[256] = {_T('\0')};

	//M3 (DX11): no MTScene -> drive the live camera (m_FpCam11) directly, mirroring
	//the per-scene auto viewpoint (section "MyViewpoint-<N>-<sceneName>", now-line rel).
	pName = _DX11SceneName();
	if ((m_DX11Family == DX11_FAMILY_NONE) || (pName == NULL)) goto EXIT;

	_stprintf_s(section, 256, _T("MyViewpoint-%d-"), viewpointNo);
	_tcscat_s(section, 256, pName);
	//live monitor keeps its own viewpoint, separate from playback
	if ((m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF)) {
		_tcscat_s(section, 256, _T("Live"));
	}
	if (m_ViewConf.SetCurSection(section) == 0) {
		float x = SENT, y = 0, z = 0, phi = 0, theta = 0, roll = 0, autoRoll = 0;
		m_ViewConf.GetFloat(_T("X"), &x, SENT);
		if (x != SENT) {   // a viewpoint was saved for this slot
			m_ViewConf.GetFloat(_T("Y"), &y, 0.0f);
			m_ViewConf.GetFloat(_T("Z"), &z, 0.0f);
			m_ViewConf.GetFloat(_T("Phi"), &phi, 0.0f);
			m_ViewConf.GetFloat(_T("Theta"), &theta, 0.0f);
			m_ViewConf.GetFloat(_T("ManualRollAngle"), &roll, 0.0f);
			m_ViewConf.GetFloat(_T("AutoRollVelocity"), &autoRoll, 0.0f);
			m_FpCam11.SetViewpointParam(x, y, z, phi, theta, roll, autoRoll);
			m_IsAutoRollMode11 = (autoRoll != 0.0f);
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// MyViewpoint 保存（現在の視点を設定ファイルへ）
//******************************************************************************
int MIDITrailApp::_SaveMyViewpoint(unsigned long viewpointNo)
{
	int result = 0;
	const TCHAR* pName = NULL;
	TCHAR section[256] = {_T('\0')};

	//M3 (DX11): no MTScene -> snapshot the live camera (m_FpCam11) into m_ViewConf
	//(section "MyViewpoint-<N>-<sceneName>", now-line relative position).
	pName = _DX11SceneName();
	if ((m_DX11Family == DX11_FAMILY_NONE) || (pName == NULL)) goto EXIT;

	{
		float x = 0, y = 0, z = 0, phi = 0, theta = 0, roll = 0;
		m_FpCam11.GetViewpointParam(&x, &y, &z, &phi, &theta, &roll);
		float autoRoll = m_IsAutoRollMode11 ? m_FpCam11.GetAutoRollVelocity() : 0.0f;
		_stprintf_s(section, 256, _T("MyViewpoint-%d-"), viewpointNo);
		_tcscat_s(section, 256, pName);
		//live monitor keeps its own viewpoint, separate from playback
		if ((m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF)) {
			_tcscat_s(section, 256, _T("Live"));
		}
		if (m_ViewConf.SetCurSection(section) == 0) {
			m_ViewConf.SetFloat(_T("X"), x);
			m_ViewConf.SetFloat(_T("Y"), y);
			m_ViewConf.SetFloat(_T("Z"), z);
			m_ViewConf.SetFloat(_T("Phi"), phi);
			m_ViewConf.SetFloat(_T("Theta"), theta);
			m_ViewConf.SetFloat(_T("ManualRollAngle"), roll);
			m_ViewConf.SetFloat(_T("AutoRollVelocity"), autoRoll);
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// フォルダ選択ダイアログ（IFileOpenDialog, FOS_PICKFOLDERS）
//******************************************************************************
int MIDITrailApp::_SelectFolder(WCHAR* pFolderPath, unsigned long bufSize, bool* pIsSelected)
{
	int result = 0;
	errno_t eresult = 0;
	HRESULT hresult = 0;
	HRESULT hrInit = 0;
	DWORD options = 0;
	IFileOpenDialog* pFileOpenDialog = NULL;
	LPWSTR pFolderPathW = NULL;
	IShellItem* pShellItem = NULL;

	if ((pFolderPath == NULL) || (bufSize == 0) || (pIsSelected == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	*pIsSelected = false;

	//COM 初期化（このapp本体はCOMを初期化していないためスコープ内で行う）
	hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	hresult = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpenDialog));
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}

	pFileOpenDialog->GetOptions(&options);
	pFileOpenDialog->SetOptions(options | FOS_PICKFOLDERS);

	//m_hWndを指定すると演奏開始後のダイアログ表示でハングするためNULL指定
	hresult = pFileOpenDialog->Show(NULL);
	if (hresult == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		goto EXIT;
	}
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}

	hresult = pFileOpenDialog->GetResult(&pShellItem);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}
	hresult = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pFolderPathW);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}

	eresult = wcscpy_s(pFolderPath, bufSize, pFolderPathW);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", eresult, 0);
		goto EXIT;
	}
	*pIsSelected = true;

EXIT:;
	if (pFolderPathW != NULL) CoTaskMemFree(pFolderPathW);
	if (pShellItem != NULL) pShellItem->Release();
	if (pFileOpenDialog != NULL) pFileOpenDialog->Release();
	if (SUCCEEDED(hrInit)) CoUninitialize();
	return result;
}

//******************************************************************************
// 指定フォルダ直下の MIDI ファイルリストを作成
//******************************************************************************
int MIDITrailApp::_MakeFileListWithFolder(const WCHAR* pFolderPath, MTFileList* pFileList)
{
	int result = 0;
	if ((pFolderPath == NULL) || (pFileList == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	result = pFileList->MakeFileListWithDirectory(pFolderPath, &m_RcpConv);
	if (result != 0) goto EXIT;
EXIT:;
	return result;
}

//******************************************************************************
// 演奏停止とフォルダオープン（先頭ファイルを開く）
//******************************************************************************
int MIDITrailApp::_StopPlaybackAndOpenFolder(const WCHAR* pFolderPath)
{
	int result = 0;
	MTFileList midiFileList;
	const WCHAR* pFilePath = NULL;

	//事前確認用の一時リストで MIDI ファイル有無を確認
	result = _MakeFileListWithFolder(pFolderPath, &midiFileList);
	if (result != 0) goto EXIT;

	if (midiFileList.GetFileCount() == 0) {
		MessageBox(m_hWnd, _T("MIDI data file is not found in the folder."), _T("WARNING"), MB_OK | MB_ICONWARNING);
		goto EXIT;
	}

	//本番のファイルリストを作成し、先頭ファイルを開く
	result = _MakeFileListWithFolder(pFolderPath, &m_MIDIFileList);
	if (result != 0) goto EXIT;

	m_MIDIFileList.SelectFirstFile();
	pFilePath = m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex());
	result = _OpenFileW(pFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}
// <<< add ced 20260627

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
			if (m_pScene != NULL) result = m_pScene->Rewind();
			if (result != 0) goto EXIT;
		}

		//シーンに演奏開始を通知
		if (m_pScene != NULL) result = m_pScene->OnPlayStart(NULL /*M5: DX9 device removed; m_pScene is always NULL here*/);
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
	if (m_pScene != NULL) m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);
	//ダッシュボードの速度表示（"SPEED:NNN%"）を更新（100%以外で表示）
	m_Dashboard11.SetPlaySpeedRatio(m_PlaySpeedRatio);

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
	if (m_pScene != NULL) m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);
	//ダッシュボードの速度表示（"SPEED:NNN%"）を更新（100%以外で表示）
	m_Dashboard11.SetPlaySpeedRatio(m_PlaySpeedRatio);

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
	//M5/DX11�F���̓f�o�C�X�ݒ�̓V�[���ɔ�ˑ��Bm_pScene �� NULL �ł����s����
	//�i���O�̓���ł� m_pScene != NULL �Q�[�g�̂��� SetInPortDev ���Ă΂ꂸ�A
	//  ���j�^�����͂��󂯎��Ȃ�������j
	result = _SetMonitorPortDev(&m_LiveMonitor, m_pScene);
	if (result != 0) goto EXIT;
	
	//シーンに演奏開始を通知
	if (m_pScene != NULL) result = m_pScene->OnPlayStart(NULL /*M5: DX9 device removed; m_pScene is always NULL here*/);
	if (result != 0) goto EXIT;
	
	//ライブモニタ開始
	result = m_LiveMonitor.Start();
	if (result != 0) goto EXIT;
	
	//演奏状態変更
	result = _ChangePlayStatus(MonitorON);
	if (result != 0) goto EXIT;

	//DX11: build the live scene components (dynamic note boxes + keyboard + grid)
	m_NoteBoxLive11.Reset();
	m_NoteRainLive11.Reset();
	_SetupDX11Scene();

	//dashboard: live monitor mode (MIDI IN device name + played-note count)
	{
		TCHAR devName[MAXPNAMELEN] = { _T('\0') };
		if (m_MIDIConf.SetCurSection(_T("MIDIIN")) == 0) {
			m_MIDIConf.GetStr(_T("PortA"), devName, MAXPNAMELEN, _T(""));
		}
		m_LiveNoteCount = 0;
		m_Dashboard11.SetMonitorMode(true, devName);
		m_Dashboard11.SetCurNotes(0);
	}

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
		result = m_pScene->OnPlayEnd(NULL /*M5: DX9 device removed; m_pScene is always NULL here*/);
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

		//M3 (DX11): _CreateScene is a stub in the DX11 path, so rebuild the DX11
		//scene components for the newly selected scene type (3D/2D supported;
		//Rain/Ring detach until ported).
		_SetupDX11Scene();
	}

EXIT:;
	return result;
}

//******************************************************************************
// Attach/detach the DX11 components per the View-menu effect toggles (DX11 has
// no MTScene, so SetEffect can't reach it; we toggle the renderer pointers).
// Notes are always shown; only the optional effects honor the flags.
//******************************************************************************
void MIDITrailApp::_ApplyDX11Visibility()
{
	//M4.8: pitch bend on/off (View > Pitch Bend). Disabling makes the bend
	//state report 0 so neither the keyboards nor the note boxes shift.
	m_NotePitchBend11.SetEnable(m_isEnablePitchBend);

	//M4.22: bend the whole channel (all notes) vs only the sounding notes.
	//Member objects always exist, so it is safe to set on all three.
	m_NoteBox11.SetPitchBendAllNotes(m_isEnablePitchBendAllNotes);
	m_NoteRain11.SetPitchBendAllNotes(m_isEnablePitchBendAllNotes);
	m_NoteBoxRing11.SetPitchBendAllNotes(m_isEnablePitchBendAllNotes);

	//M4.15: background image shows in every scene family (honors the toggle)
	m_Renderer11.SetBackgroundImage11(
			((m_DX11Family != DX11_FAMILY_NONE) && m_isEnableBackgroundImage) ? &m_BackgroundImage11 : NULL);

	//M4.16: starfield shows in every scene family (honors the toggle)
	m_Renderer11.SetStars11(
			((m_DX11Family != DX11_FAMILY_NONE) && m_isEnableStars) ? &m_Stars11 : NULL);

	if (m_DX11Family == DX11_FAMILY_BOX) {
		bool live = (m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF);
		if (live) {
			//live: the dynamic note box + keyboard + the real-time ripple (note-on
			//driven). Lyrics/grid/time-indicator need a pre-built note list - re-enabling
			//the stale playback ones would draw garbage, so force those off.
			m_Renderer11.SetKeyboard11(m_isEnablePianoKeyboard ? &m_Kbd11 : NULL);
			m_Renderer11.SetNoteRipple11(m_isEnableRipple ? &m_NoteRipple11 : NULL);
			m_Renderer11.SetNoteLyrics11(NULL);
			m_Renderer11.SetGridBox11(NULL);
			m_Renderer11.SetTimeIndicator11(NULL);
			m_Renderer11.SetDashboard11(m_isEnableCounter ? &m_Dashboard11 : NULL);
		}
		else {
			m_Renderer11.SetKeyboard11(m_isEnablePianoKeyboard ? &m_Kbd11 : NULL);
			m_Renderer11.SetNoteRipple11(m_isEnableRipple ? &m_NoteRipple11 : NULL);
			m_Renderer11.SetNoteLyrics11(m_isEnableLyrics ? &m_NoteLyrics11 : NULL);
			m_Renderer11.SetGridBox11(m_isEnableGridBox ? &m_GridBox11 : NULL);
			m_Renderer11.SetTimeIndicator11(m_isEnableTimeIndicator ? &m_TimeIndicator11 : NULL);
			m_Renderer11.SetDashboard11(m_isEnableCounter ? &m_Dashboard11 : NULL);
		}
	}
	else if (m_DX11Family == DX11_FAMILY_RAIN) {
		//rain keyboard is the same object for live + playback (live builds it with no
		//song and lights keys via SetNoteOnLive); always route through SetKeyboardRain11.
		m_Renderer11.SetKeyboard11(NULL);
		m_Renderer11.SetKeyboardRain11(m_isEnablePianoKeyboard ? &m_KbdRain11 : NULL);
		m_Renderer11.SetNoteLyrics11(NULL);   //rain scene has no lyrics
		m_Renderer11.SetDashboard11(m_isEnableCounter ? &m_Dashboard11 : NULL);
	}
	else if (m_DX11Family == DX11_FAMILY_RING) {
		bool live = (m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF);
		//live ring shows the dynamic circular notes + textured picture board + the
		//real-time ripple + dashboard; grid/time need a pre-built note list, off in live.
		m_Renderer11.SetGridRing11((!live && m_isEnableGridBox) ? &m_GridRing11 : NULL);
		m_Renderer11.SetTimeIndicatorRing11((!live && m_isEnableTimeIndicator) ? &m_TimeIndicatorRing11 : NULL);
		m_Renderer11.SetNoteRipple11(m_isEnableRipple ? &m_NoteRipple11 : NULL);
		m_Renderer11.SetPictBoardRing11(m_isEnablePianoKeyboard ? &m_PictBoardRing11 : NULL);
		//ring lyrics (1.4.1 ported): playback only (needs the pre-built note list)
		m_Renderer11.SetNoteLyrics11((!live && m_isEnableLyrics) ? &m_NoteLyrics11 : NULL);
		m_Renderer11.SetDashboard11(m_isEnableCounter ? &m_Dashboard11 : NULL);
	}
}

//******************************************************************************
// conf / scene name for the current SceneType (DX11 path)
//******************************************************************************
const TCHAR* MIDITrailApp::_DX11SceneName()
{
	switch (m_SceneType) {
		case PianoRoll3D:     return _T("PianoRoll3D");
		case PianoRoll2D:     return _T("PianoRoll2D");
		case PianoRollRain:   return _T("PianoRollRain");
		case PianoRollRain2D: return _T("PianoRollRain2D");
		case PianoRollRing:   return _T("PianoRollRing");
		default:              return NULL;
	}
}

//******************************************************************************
// M4.22 Menu: toggle "bend the whole channel" (all notes vs sounding only)
//******************************************************************************
int MIDITrailApp::_OnMenuTogglePitchBendAllNotes()
{
	int result = 0;

	m_isEnablePitchBendAllNotes = m_isEnablePitchBendAllNotes ? false : true;

	//update the menu checkmark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//reflect it in the DX11 note renderers (live)
	_ApplyDX11Visibility();

EXIT:;
	return result;
}

//******************************************************************************
// Menu: toggle single vs per-port keyboard (DX11 box scenes), then rebuild
//******************************************************************************
int MIDITrailApp::_OnMenuToggleSingleKeyboard()
{
	int result = 0;

	m_IsSingleKeyboard11 = m_IsSingleKeyboard11 ? false : true;

	//update the menu checkmark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//rebuild the DX11 scene so the keyboard is recreated in the new mode
	//(only meaningful while a song is loaded and stopped/paused)
	if (m_PlayStatus != NoData) {
		_SetupDX11Scene();
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
// メニュー選択：View 表示設定の自動保存（ced 20260628）
//******************************************************************************
int MIDITrailApp::_OnMenuAutoSaveViewSettings()
{
	int result = 0;

	m_isAutoSaveViewSettings = m_isAutoSaveViewSettings ? false : true;

	//メニュー選択マーク更新
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//シーン設定保存（フラグと、有効時は現在の View 表示トグルを書き出す）
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
	if (m_pScene != NULL) {
		m_pScene->MoveToStaticViewpoint(viewpointNo);
	}
	else {
		//M3 (DX11): apply the static viewpoint directly to the camera, using the
		//ACTUAL scene's conf (Rain/Ring have their own [Viewpoint-N], and the
		//camera reads the scroll offset on the right axis per progress direction).
		if (viewpointNo == 1) {
			if (m_SceneType == PianoRollRing) m_FpCam11.SetDefaultViewpointRing();
			else m_FpCam11.SetDefaultViewpoint();
		}
		else {
			const TCHAR* pSceneName = _DX11SceneName();
			if (pSceneName != NULL) m_FpCam11.SetViewpointFromConf(pSceneName, viewpointNo);
		}
	}

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
	if (m_pScene != NULL) m_pScene->ResetViewpoint();
	else if (m_SceneType == PianoRollRing) m_FpCam11.SetDefaultViewpointRing();  //M3 (DX11)
	else m_FpCam11.SetDefaultViewpoint();

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
		case MTScene::EffectLyrics:
			m_isEnableLyrics = m_isEnableLyrics ? false : true;
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
// >>> add 20180404 yossiepon begin
		case MTScene::EffectTimeIndicator:
			m_isEnableTimeIndicator = !m_isEnableTimeIndicator;
			break;
		case MTScene::EffectGridBox:
			m_isEnableGridBox = !m_isEnableGridBox;
			break;
// <<< add 20180404 yossiepon end
		default:
			break;
	}

	_UpdateEffect();
	_ApplyDX11Visibility();   //M4.10: reflect the toggle in the DX11 renderer
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
	//M4 (DX11): query actual MSAA support from the D3D11 device
	for (multiSampleType = DX_MULTI_SAMPLE_TYPE_MIN; multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX; multiSampleType++) {
		isSupport = m_Renderer11.IsMultiSampleSupported(multiSampleType);
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
// メニュー選択：カラー設定（1.4.1 カラーパレット移植）
//******************************************************************************
int MIDITrailApp::_OnMenuOptionColor()
{
	int result = 0;

	//設定ダイアログ表示
	result = m_ColorCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//変更された場合はシーンを再生成（MTNoteDesign が選択パレットを再読込する）
	if (m_ColorCfgDlg.IsChanged()) {
		result = _ChangeWindowSize();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// メニュー選択：設定マネージャ（conf/*.ini を GUI 編集：Mod Mod 独自）
//   ImGui ウィンドウの表示/非表示をトグルする。実描画はレンダラの ImGui フレーム内。
//******************************************************************************
int MIDITrailApp::_OnMenuConfigManager()
{
	m_ConfigMgr11.Toggle();
	return 0;
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
				result = m_pScene->OnPlayEnd(NULL /*M5: DX9 device removed; m_pScene is always NULL here*/);
				if (result != 0) goto EXIT;
			}

			//視点保存
			if (m_isAutoSaveViewpoint) {
				result = _OnMenuSaveViewpoint();
				if (result != 0) goto EXIT;
			}

			//��~��̃t�@�C���I�[�v�����w�肳��Ă���ꍇ�i�Đ����ɕʃt�@�C����
			//�h���b�v/Open�����ꍇ�j�B M3 (DX11): m_pScene �� NULL �Ȃ̂ŁA���̕���
			//�� m_pScene �Q�[�g���O���i�O���ƃL���[���ꂽ�t�@�C�����J����Ȃ��j�B
			if (m_isOpenFileAfterStop) {
				m_isOpenFileAfterStop = false;
				result = _FileOpenProc(m_NextFilePath);
				if (result != 0) goto EXIT;
			}
			//ユーザーが停止ボタンで止めた場合は曲の先頭へ巻き戻す（DX9 と同じ。自然
			//終了時は下の else に入り、巻き戻さずビューを末尾に残す＝次回再生時に戻る）
			else if (m_isRewind) {
				m_isRewind = false;
				//DX11: m_pScene は NULL。シーケンサ位置を先頭へ戻し、Run ループの
				//tick フィードでビューが先頭へ戻る（DX9 の scene->Rewind 相当）。
				m_Sequencer.Rewind();
				if (m_pScene != NULL) result = m_pScene->Rewind();
				if (result != 0) goto EXIT;
				//ced 20260630: DX9 互換 — 手動停止時はマウスで動かした視点を戻す。
				//ced 20260703: 戻し先はハードコード既定ではなく「保存済み(なければ既定)」視点。
				//（Auto save viewpoint ON なら直前の保存で現在位置＝保存視点となり実質維持、
				//　OFF でも初期値ではなくユーザーが保存した視点へ戻る）
				_ResetViewpointToSaved();
			}
			//通常の演奏終了の場合は次回の演奏時に巻き戻す
			else {
				m_isRewind = true;
				//フォルダ演奏が有効なら次ファイルへ自動送り（1.4.1 移植。最後のファイル
				//なら送らずに停止のまま）。リピートよりフォルダ演奏を優先する。
				if (m_isFolderPlayback && (m_MIDIFileList.GetFileCount() > 0)) {
					bool isExist = false;
					m_MIDIFileList.SelectNextFile(&isExist);
					if (isExist) {
						const WCHAR* pFilePath = m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex());
						result = _OpenFileW(pFilePath);
						if (result != 0) goto EXIT;
						result = _OnMenuPlay();
						if (result != 0) goto EXIT;
					}
				}
				//リピート有効なら再生開始
				else if (m_isRepeat) {
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

	//DX11 live monitor: route real-time MIDI note on/off to the dynamic note
	//boxes (and the keyboard's live key-press state). Only while monitoring.
	if (m_PlayStatus == MonitorON) {
		SMMsgParser::Message msg = parser.GetMsg();
		//the active live renderer (box/ring or rain) is the only one created; the
		//others no-op (NULL note buffer), so route to both unconditionally.
		if (msg == SMMsgParser::MsgNoteOn) {
			if (parser.GetVelocity() > 0) {
				m_NoteBoxLive11.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
				m_NoteRainLive11.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
				m_NoteRipple11.SetNoteOnLive(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
				m_Kbd11.SetNoteOnLive(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
				m_KbdRain11.SetNoteOnLive(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
				m_LiveNoteCount++;
				m_Dashboard11.SetCurNotes(m_LiveNoteCount);
			} else {
				m_NoteBoxLive11.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
				m_NoteRainLive11.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
				m_Kbd11.SetNoteOffLive(parser.GetNoteNo());
				m_KbdRain11.SetNoteOffLive(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
			}
		}
		else if (msg == SMMsgParser::MsgNoteOff) {
			m_NoteBoxLive11.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
			m_NoteRainLive11.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
			m_Kbd11.SetNoteOffLive(parser.GetNoteNo());
			m_KbdRain11.SetNoteOffLive(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
		}
		else if (msg == SMMsgParser::MsgAllNoteOff) {
			m_NoteBoxLive11.AllNoteOff();
			m_NoteRainLive11.AllNoteOff();
			m_Kbd11.AllNoteOffLive();
			m_KbdRain11.AllNoteOffLive();
		}
	}

	//デバイスロスト対策
	//シーンに渡した最新メッセージを記録しておく
	//M3 (DX11): on a skip, rebuild the DX11 active-note windows so the keyboard
	//and ripple don't stay stuck on the pre-skip state. The skip runs
	//synchronously in the sequencer timer callback that posts this message, so by
	//the time we read it the playback tick is already at the target; Reset() makes
	//the next per-frame poll re-fold the window at the new position.
	if ((parser.GetMsg() == SMMsgParser::MsgSkipStart)
	 || (parser.GetMsg() == SMMsgParser::MsgSkipEnd)) {
		// reset only the components created for the active scene family
		if (m_DX11Family == DX11_FAMILY_RAIN) {
			m_KbdRain11.Reset();
			m_NoteRain11.Reset();
		}
		else if (m_DX11Family == DX11_FAMILY_RING) {
			m_NoteBoxRing11.Reset();
			m_NoteRipple11.Reset();
			m_NoteLyrics11.Reset();   //ring lyrics (1.4.1 ported)
		}
		else if (m_DX11Family == DX11_FAMILY_BOX) {
			m_Kbd11.Reset();
			m_NoteRipple11.Reset();
			m_NoteLyrics11.Reset();
			m_TimeIndicator11.Reset();
		}
		m_NotePitchBend11.Reset();   //M4.8: clear pitch bend on skip (DX9 resets on skip-back)
		m_NpsNoteCount = 0;
		//the smooth scroll to the target is driven by the sequencer's slide
		//(_SlidePlaybackTime updates the polled tick); the component resets above rebuild
		//their forward windows from 0 so they re-fold correctly as the tick sweeps across.
	}

	//M4 (DX11): the played-note count is derived per-frame from the tick (see Run
	//loop), so it stays accurate even when note-on messages drop under load.

	if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		//演奏チックタイム通知
		m_SequencerLastMsg.isRecvPlayTime = true;
		m_SequencerLastMsg.playTime.param1 = param1;
		m_SequencerLastMsg.playTime.param2 = param2;
		//M3 (DX11): the playback tick + dashboard time are polled per-frame
		//straight from the sequencer (see Run loop), so heavy-load message drops
		//cannot stall the scroll / keyboard / dashboard time.
	}
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		//テンポ変更通知
		m_SequencerLastMsg.isRecvTempo = true;
		m_SequencerLastMsg.tempo.param1 = param1;
		m_SequencerLastMsg.tempo.param2 = param2;
		m_Dashboard11.SetTempo(parser.GetTempoBPM());
		if (parser.GetTempoBPM() > 0) m_CurSongBPM = parser.GetTempoBPM();   //keyboard envelope scale
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		//小節番号通知
		m_SequencerLastMsg.isRecvBar = true;
		m_SequencerLastMsg.bar.param1 = param1;
		m_SequencerLastMsg.bar.param2 = param2;
		m_Dashboard11.SetCurBar(parser.GetBarNo());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		//拍子記号変更通知
		m_SequencerLastMsg.isRecvBeat = true;
		m_SequencerLastMsg.beat.param1 = param1;
		m_SequencerLastMsg.beat.param2 = param2;
		m_Dashboard11.SetBeat(parser.GetBeatNumerator(), parser.GetBeatDenominator());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgPitchBend) {
		//M4.8 (DX11): per-channel pitch bend -> shifts the keyboard (and notes)
		m_NotePitchBend11.SetPitchBend(
				parser.GetPortNo(), parser.GetChNo(),
				parser.GetPitchBendValue(), parser.GetPitchBendSensitivity());
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
	//M3 (DX11, scene is NULL): toggle mouse-look (L) / auto-roll (M) on the camera.
	//Allowed whenever a song is LOADED (Stop/Play/Pause/Monitor) - matching DX9 where
	//the scene handled the click in any non-title state. Only the title/no-song state
	//(NoData) is excluded so a click there doesn't grab the mouse (hidden+clipped cursor).
	//(ced 20260628: was also excluding Stop, which wrongly disabled mouse-look while
	// paused/stopped on a loaded song; _ChangePlayStatus still auto-releases on stop.)
	else if (m_PlayStatus != NoData) {
		if (button == WM_LBUTTONDOWN) {
			m_IsMouseCamMode11 = !m_IsMouseCamMode11;
			m_FpCam11.SetMouseCamMode(m_IsMouseCamMode11);
		}
		else if (button == WM_MBUTTONDOWN) {
			m_IsAutoRollMode11 = !m_IsAutoRollMode11;
			m_FpCam11.SetAutoRollMode(m_IsAutoRollMode11);
			if (m_IsAutoRollMode11) m_FpCam11.SwitchAutoRllDirecton();
		}
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
	//ウィンドウ表示の場合（1.4.1 移植：Menu Bar を非表示にしている時は、
	//マウスを上端近くに乗せた時だけ一時的にメニューを表示する）
	else if (!m_isEnableMenuBar) {
		if (point.y <= 5) {
			result = _ShowMenu();
			if (result != 0) goto EXIT;
		}
		else {
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
//******************************************************************************
// Convert an ANSI double-null-terminated filter string to wide
//******************************************************************************
static void _AnsiFilterToWide(const char* pAnsi, wchar_t* pWide, int wideCount)
{
	int len = 0;
	if (pAnsi == NULL) { pWide[0] = L'\0'; pWide[1] = L'\0'; return; }
	while (!(pAnsi[len] == '\0' && pAnsi[len + 1] == '\0')) len++;
	len += 2;
	MultiByteToWideChar(CP_ACP, 0, pAnsi, len, pWide, wideCount);
}

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
	{
		//get the true (Unicode) path so file names with Unicode chars open correctly
		wchar_t pathW[_MAX_PATH];
		pathW[0] = L'\0';
		charNum = DragQueryFileW(hDrop, 0, pathW, _MAX_PATH);
		if (charNum == 0) {
			result = YN_SET_ERR("Windows API error.", wParam, lParam);
			goto EXIT;
		}
		wcscpy_s(m_LoadFilePathW, MAX_PATH, pathW);
		WideCharToMultiByte(CP_ACP, 0, pathW, -1, path, _MAX_PATH, NULL, NULL);
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
	OPENFILENAMEW ofn;
	wchar_t fileW[MAX_PATH];
	wchar_t filterW[1024];

	if ((pFilePath == NULL) || (bufSize == 0) || (pIsSelected ==NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pFilePath[0] = _T('\0');
	fileW[0] = L'\0';
	ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner   = m_hWnd;
	ofn.lpstrFilter = L"Standard MIDI File (*.mid)\0*.mid\0\0";
	ofn.lpstrFile   = fileW;
	ofn.nMaxFile    = MAX_PATH;
	ofn.lpstrTitle  = L"Select Standard MIDI File.";
	ofn.Flags       = OFN_FILEMUSTEXIST;  //OFN_HIDEREADONLY

	//rcpcv.dllが有効ならファイルフィルタを変更する
	if (m_RcpConv.IsAvailable()) {
		_AnsiFilterToWide(m_RcpConv.GetOpenFileNameFilter(), filterW, 1024);
		ofn.lpstrFilter = filterW;
	}

	//ファイル選択ダイアログ表示
	apiresult = GetOpenFileNameW(&ofn);
	if (!apiresult) {
		//キャンセルまたはエラー発生：エラーはチェックしない
		*pIsSelected = false;
		goto EXIT;
	}

	wcscpy_s(m_LoadFilePathW, MAX_PATH, fileW);
	WideCharToMultiByte(CP_ACP, 0, fileW, -1, pFilePath, bufSize, NULL, NULL);
	*pIsSelected = true;

EXIT:;
	return result;
}

//******************************************************************************
// MIDIファイル読み込み
//******************************************************************************
//******************************************************************************
// Loading progress callback (drives the loading screen during note build)
//******************************************************************************
void MIDITrailApp::_LoadProgressCallback(
		unsigned long current,
		unsigned long total,
		void* user
	)
{
	MIDITrailApp* pThis = (MIDITrailApp*)user;
	float local = 0.0f;
	char message[128];

	if (pThis == NULL) return;
	if (total > 0) local = (float)current / (float)total;

	// the note-field build occupies the 35%..85% band of the overall progress
	sprintf_s(message, sizeof(message), "Building note field:  %lu / %lu notes", current, total);
	pThis->m_Renderer11.DrawLoadingScreen(message, 0.35f + local * 0.50f);
}

//******************************************************************************
// Parse progress callback (drives the loading screen during SMF parsing)
//******************************************************************************
void MIDITrailApp::_ParseProgressCallback(
		unsigned long current,
		unsigned long total,
		void* user
	)
{
	MIDITrailApp* pThis = (MIDITrailApp*)user;
	float local = 0.0f;
	char message[128];

	if (pThis == NULL) return;
	if (total > 0) local = (float)current / (float)total;

	// current/total are whole-file units of 10000 per track (see SMFileReader), so the
	// bar no longer restarts per track. Show the track being read for context.
	unsigned long trackCount = (total >= 10000) ? (total / 10000) : 1;
	unsigned long curTrack   = (current / 10000) + 1;
	if (curTrack > trackCount) curTrack = trackCount;
	sprintf_s(message, sizeof(message), "Reading MIDI file...  (track %lu / %lu)", curTrack, trackCount);
	pThis->m_Renderer11.DrawLoadingScreen(message, local * 0.30f);   // parse = 0%..30% band
}

int MIDITrailApp::_LoadMIDIFile(
		const TCHAR* pFilePath
	)
{
	int result = 0;
	TCHAR* pPath = NULL;
	TCHAR smfTempPath[_MAX_PATH] = {_T('\0')};
	TCHAR smfDumpPath[_MAX_PATH] = {_T('\0')};
	SMFileReader smfReader;

	//Begin loading: show loading screen and ignore input until done. Draw it BEFORE the
	//file open/memory-map (a single blocking call that can't report % mid-way) so the
	//window is already up and labelled when a large file takes a moment to open.
	m_isLoading = true;
	m_Renderer11.DrawLoadingScreen("Opening MIDI file...", 0.0f);

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
		WCHAR smfDumpPathW[_MAX_PATH] = { L'\0' };
		MultiByteToWideChar(CP_ACP, 0, smfDumpPath, -1, smfDumpPathW, _MAX_PATH);
		smfReader.SetLogPath(smfDumpPathW);
	}

	//ファイル読み込み
	//32bit上限(約42.9億イベント/約21億ノート)を超える巨大ファイルは全部は読めない。
	//フラグをリセットしておき、読み込み後に切り捨てが起きたか確認する。
	SMSimpleList::ResetTruncatedFlag();
	SMFileReader::SetLoadProgressCallback(&MIDITrailApp::_ParseProgressCallback, this);
	if ((m_LoadFilePathW[0] != L'\0') && (pPath == pFilePath)) {
		result = smfReader.LoadW(m_LoadFilePathW, &m_SeqData);
	}
	else {
		result = smfReader.Load(pPath, &m_SeqData);
	}
	if (result != 0) goto EXIT;

	//ファイルが大きすぎて一部のノートを読み込めなかった場合は、読めた分を表示するか確認
	if (SMSimpleList::WasTruncated()) {
		int answer = MessageBox(m_hWnd,
				_T("This file is too large to load completely (over the 32-bit limit of\n")
				_T("~2.1 billion notes); some notes could not be read.\n\n")
				_T("Display the portion that was loaded anyway?"),
				_T("MIDITrail - File too large"),
				MB_YESNO | MB_ICONWARNING);
		if (answer != IDYES) {
			//表示しない：読み込みを中止し、空シーン(NoData)へ戻す（この読み込みで
			//前の曲の m_SeqData は上書き済みなので、安全側として未読込状態にする）。
			m_SeqData.Clear();
			_ChangePlayStatus(NoData);
			_SetupDX11Scene();   //NoData 経路：全コンポーネントを切り離す
			result = 0;
			goto EXIT;
		}
		//Yes：読み込めた分でこのまま続行
	}

	//ファイル読み込み時に再生スピードを100%に戻す：_CreateSceneでカウンタに反映
	m_PlaySpeedRatio = 100;

	//シーンオブジェクト生成
	m_SceneType = m_SelectedSceneType;
	m_Renderer11.DrawLoadingScreen("Building scene...", 0.32f);

	result = _CreateScene(m_SceneType, &m_SeqData);
	if (result != 0) goto EXIT;

	//M3 (DX11 migration): build the instanced note field for the loaded song.
	//Capture the file name (basename) for the dashboard; it must survive a later
	//renderer re-init (AA / window-size change clears m_LoadFilePathW).
	{
		const wchar_t* pBaseW = wcsrchr(m_LoadFilePathW, L'\\');
		pBaseW = (pBaseW != NULL) ? (pBaseW + 1) : m_LoadFilePathW;
		WideCharToMultiByte(CP_ACP, 0, pBaseW, -1, m_DashFileNameA, MAX_PATH, NULL, NULL);
	}
	//演奏状態変更
	result = _ChangePlayStatus(Stop);
	if (result != 0) goto EXIT;

	//rewind the sequencer to the song head so the freshly loaded scene starts at
	//tick 0 (otherwise GetCurrentTickTime still reports the previous song's stop
	//position and the new scene would render at a wrong scroll until Play).
	m_Sequencer.Rewind();

	//M3 (DX11): build the scene components now that the play status is set
	//(so _SetupDX11Scene sees a loaded song, not NoData).
	_SetupDX11Scene();
	m_Renderer11.DrawLoadingScreen("Ready", 1.0f);

	m_isRewind = false;

EXIT:;
	m_LoadFilePathW[0] = L'\0';
	SMFileReader::SetLoadProgressCallback(NULL, NULL);
	m_isLoading = false;
	//ced 20260629: ロード後にウィンドウを必ずアクティブ(フォアグラウンド)化する。
	//DirectInput は DISCL_FOREGROUND のため、ドラッグ&ドロップ等でウィンドウが
	//非アクティブのままロードするとキーボード/マウス(カメラ)が取得できず操作不能に
	//なる（メニュー等をクリックしてアクティブ化すると直る、の根本原因）。
	if (m_hWnd != NULL) {
		SetForegroundWindow(m_hWnd);
		SetFocus(m_hWnd);
	}
	if (_tcslen(smfTempPath) != 0) {
		DeleteFile(smfTempPath);
	}
	return result;
}

//******************************************************************************
// (Re)create the DX11 scene components for the currently loaded song.
//
// Called on load and again after the renderer re-initializes (AA / window-size
// change recreates the D3D11 device, so every device resource must be rebuilt -
// otherwise the stale components reference a destroyed device => "Device lost").
// Safe to call with no song loaded (NoData): it just detaches all components.
//******************************************************************************
//******************************************************************************
// M6: feed the live playback tick to the active DX11 scene components
//******************************************************************************
void MIDITrailApp::_FeedDX11Tick(unsigned long tick, unsigned long playMs)
{
	// `tick` is polled straight from the sequencer. A seek scrolls smoothly because the
	// sequencer's _SlidePlaybackTime slides this tick from the old to the new position
	// over MovingTimeSpanInMsec (DX9's seek slide); no extra easing is needed here.
	m_FpCam11.SetCurTickTime(tick);
	m_Dashboard11.SetPlayTimeMSec(playMs);
	// feed only the components created for the active scene family; the others
	// may never have been created (crash if touched).
	if (m_DX11Family == DX11_FAMILY_RAIN) {
		m_NoteRain11.SetCurTickTime(tick);
		m_KbdRain11.SetCurTickTime(tick);
		//live: the note count is the played-note counter (note-on driven), not the
		//static field's GetPlayedNoteCount (0 with no song) - else it overwrites to 0.
		if (m_PlayStatus != MonitorON) {
			m_Dashboard11.SetCurNotes(m_NoteRain11.GetPlayedNoteCount(tick));
		}
	}
	else if (m_DX11Family == DX11_FAMILY_RING) {
		m_NoteBoxRing11.SetCurTickTime(tick);
		m_TimeIndicatorRing11.SetCurTickTime(tick);
		m_PictBoardRing11.SetCurTickTime(tick);
		m_NoteRipple11.SetCurTickTime(tick);
		m_NoteRipple11.SetPlayTimeMSec(playMs);
		m_NoteLyrics11.SetCurTickTime(tick);   //ring lyrics (1.4.1 ported)
		m_NoteLyrics11.SetPlayTimeMSec(playMs);
		if (m_PlayStatus != MonitorON) {
			m_Dashboard11.SetCurNotes(m_NoteBoxRing11.GetPlayedNoteCount(tick));
		}
	}
	else if (m_DX11Family == DX11_FAMILY_BOX) {
		m_NoteBox11.SetCurTickTime(tick);
		// song time scale (ticks/ms) for the keyboard press envelope: TimeDivision*BPM/60000.
		// Uses the SONG tempo (not play speed) - the polled tick already carries speed.
		// The note box uses the same scale to decay the active-note flash over a fixed time.
		{
			unsigned long tpqn = m_SeqData.GetTimeDivision();
			if (tpqn > 0) {
				double tickPerMs = (double)tpqn * (double)m_CurSongBPM / 60000.0;
				m_Kbd11.SetSongTickPerMs(tickPerMs);
				m_NoteBox11.SetSongTickPerMs(tickPerMs);
			}
		}
		m_Kbd11.SetCurTickTime(tick);
		m_NoteRipple11.SetCurTickTime(tick);
		m_NoteRipple11.SetPlayTimeMSec(playMs);
		m_NoteLyrics11.SetCurTickTime(tick);
		m_NoteLyrics11.SetPlayTimeMSec(playMs);
		m_TimeIndicator11.SetCurTickTime(tick);
		//live: the note count is the played-note counter (maintained on note-on),
		//not the static field's GetPlayedNoteCount (which is 0 with no song).
		if (m_PlayStatus != MonitorON) {
			m_Dashboard11.SetCurNotes(m_NoteBox11.GetPlayedNoteCount(tick));
		}
	}
}

//******************************************************************************
// M6: offline video export. Drives the tick from a fixed-FPS timeline (via the
// tempo map), renders each frame off-screen at the chosen resolution with the
// current camera viewpoint, and streams raw BGRA frames to ffmpeg.
//******************************************************************************
int MIDITrailApp::_ExportVideo(const MTVideoExportParams& params)
{
	int result = 0;

	if (m_Renderer11.GetDevice() == NULL) return YN_SET_ERR("Program error.", 0, 0);
	if (m_DX11Family == DX11_FAMILY_NONE) return YN_SET_ERR("No scene to export.", 0, 0);
	if ((params.width <= 0) || (params.height <= 0) || (params.fps <= 0)) {
		return YN_SET_ERR("Program error.", 0, 0);
	}

	// stop live playback (export drives the tick manually; no audio meanwhile)
	if ((m_PlayStatus != Stop) && (m_PlayStatus != NoData)) {
		_ChangePlayStatus(Stop);
	}

	MTTempoMap tempo;
	if (tempo.Build(&m_SeqData) != 0) return YN_SET_ERR("Tempo map build failed.", 0, 0);

	// pitch bend is normally fed by the live message queue; replay it offline
	MTPitchBendTimeline pbTimeline;
	pbTimeline.Build(&m_SeqData);
	m_NotePitchBend11.Reset();

	// End the video at the last NOTE-OFF (end of musical content) rather than at
	// the End-Of-Track event, which some files pad well past the last note. Fall
	// back to the total play time if no note range is available.
	unsigned long endTick = 0;
	if (m_DX11Family == DX11_FAMILY_BOX)       endTick = m_NoteBox11.GetMaxEndTick();
	else if (m_DX11Family == DX11_FAMILY_RAIN) endTick = m_NoteRain11.GetMaxEndTick();
	else if (m_DX11Family == DX11_FAMILY_RING) endTick = m_NoteBoxRing11.GetMaxEndTick();

	double endMs = (endTick > 0) ? tempo.TickToMs(endTick) : (double)m_SeqData.GetTotalPlayTime();
	const double tailMs = 700.0;   // short tail so the last ripples / release finish
	int fps = params.fps;
	int totalFrames = (int)((endMs + tailMs) * (double)fps / 1000.0) + 1;
	if (totalFrames < 1) totalFrames = 1;

	// 360: force a 2:1 equirectangular frame; the cubemap face resolution provides
	// the detail, so the ffmpeg-downscale supersample is not used.
	int ss, ow, oh, beginRes;
	if (params.equirect360) {
		ss = 1;
		ow = params.width;
		oh = ow / 2;                  // equirectangular is always 2:1 (dialog forced this too)
		beginRes = m_Renderer11.BeginOffscreen(ow, oh, true);
		if (beginRes != 0) return YN_SET_ERR("Offscreen target failed.", ow, oh);
	}
	else {
		// supersample for cleaner edges: render at 2x then let ffmpeg downscale (lanczos).
		// Skip it for already-large outputs (cost/memory), and fall back to native res if
		// the 2x offscreen target can't be allocated.
		ss = ((params.width <= 1920) && (params.height <= 1080)) ? 2 : 1;
		ow = params.width * ss; oh = params.height * ss;
		beginRes = m_Renderer11.BeginOffscreen(ow, oh);
		if ((beginRes != 0) && (ss > 1)) {
			ss = 1; ow = params.width; oh = params.height;
			beginRes = m_Renderer11.BeginOffscreen(ow, oh);
		}
		if (beginRes != 0) {
			return YN_SET_ERR("Offscreen target failed.", ow, oh);
		}
	}

	MTVideoExportParams ep = params;   // tell ffmpeg the effective supersample factor
	ep.superSample = ss;
	if (params.equirect360) { ep.width = ow; ep.height = oh; }   // 2:1 frame, no supersample

	MTFFmpegPipe pipe;
	if (pipe.Open(ep) != 0) {
		m_Renderer11.EndOffscreen();
		return YN_SET_ERR("Could not start ffmpeg.", 0, 0);
	}

	int rowBytes = ow * 4;

	m_isExporting = true;
	for (int f = 0; f < totalFrames; f++) {
		double ms = (double)f * 1000.0 / (double)fps;
		unsigned long tick = tempo.MsToTick(ms);
		_FeedDX11Tick(tick, (unsigned long)(ms + 0.5));

		// pitch bend at this tick (per channel), normally fed by the message queue
		pbTimeline.Apply(tick, &m_NotePitchBend11);

		// dashboard counters normally fed by the live sequencer message queue
		// (which does not run offline): tempo / bar / beat at the current tick
		{
			MTPlaybackState st;
			tempo.GetStateAtTick(tick, &st);
			m_Dashboard11.SetTempo(st.tempoBPM);
			m_Dashboard11.SetCurBar(st.barNo);
			m_Dashboard11.SetBeat(st.beatNum, st.beatDenom);
		}

		// 3-stage pipeline: render frame f (GPU) + queue its copy to staging[f&1],
		// then read back frame f-1 (staging[(f-1)&1], already done) and hand it to the
		// async ffmpeg writer. So GPU render(f) || readback(f-1) || encode(f-2) overlap.
		if (m_Renderer11.RenderOffscreenFrame(params.transparent, f & 1) != 0) { result = -1; break; }
		if (f >= 1) {
			int slot = pipe.AcquireBuffer();
			if (slot < 0) { result = -1; break; }   // ffmpeg pipe died
			if (m_Renderer11.ReadOffscreenBGRA((f - 1) & 1, pipe.BufferPtr(slot), rowBytes) != 0) { result = -1; break; }
			pipe.SubmitBuffer(slot);
		}

		// progress overlay on the real window (DrawLoadingScreen pumps messages,
		// so the window stays responsive and re-entry is guarded by m_isExporting)
		if ((f % 8) == 0) {
			char msg[128];
			_snprintf_s(msg, sizeof(msg), _TRUNCATE, "Exporting video...  %d / %d", f + 1, totalFrames);
			m_Renderer11.DrawLoadingScreen(msg, (float)f / (float)totalFrames);
		}
	}

	// drain the final rendered frame (the readback runs one frame behind the render)
	if ((result == 0) && (totalFrames >= 1)) {
		int slot = pipe.AcquireBuffer();
		if (slot >= 0) {
			if (m_Renderer11.ReadOffscreenBGRA((totalFrames - 1) & 1, pipe.BufferPtr(slot), rowBytes) == 0) {
				pipe.SubmitBuffer(slot);
			}
		}
	}
	m_isExporting = false;

	int code = pipe.Close();
	m_Renderer11.EndOffscreen();
	if ((result == 0) && (code != 0)) result = YN_SET_ERR("ffmpeg returned an error.", code, 0);

	// on failure, grab ffmpeg's stderr tail (the real reason) before pipe destructs
	m_ExportErrorMsg[0] = _T('\0');
	if (result != 0) pipe.GetErrorTail(m_ExportErrorMsg, _countof(m_ExportErrorMsg));
	return result;
}

//******************************************************************************
// M6: "Export Video..." menu - pick an output file then export (v1 defaults:
// current window size, 60fps, H.264/libx264, opaque). Encoder/resolution dialog
// comes later; ffmpeg must be on PATH.
//******************************************************************************
int MIDITrailApp::_OnMenuExportVideo()
{
	if (m_isExporting) return 0;

	// 1) pick the MIDI file to render (export always loads a freshly chosen file)
	TCHAR midiPath[MAX_PATH] = { _T('\0') };
	bool isSelected = false;
	int sel = _SelectMIDIFile(midiPath, MAX_PATH, &isSelected);
	if (sel != 0) return sel;
	if (!isSelected) return 0;   // cancelled

	if (m_isFullScreen) _HideMenu();

	// 2) load it (stops playback + builds the DX11 scene for the song)
	sel = _StopPlaybackAndOpenFile(midiPath);
	if (sel != 0) return sel;

	if (m_DX11Family == DX11_FAMILY_NONE) {
		MessageBox(m_hWnd, _T("Could not build a scene for this MIDI."), _T("Export Video"), MB_OK | MB_ICONWARNING);
		return 0;
	}

	// 3) settings dialog (codec / resolution / fps / quality). Defaults: current
	//    window client size (even), 60fps, H.264 CPU.
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	int w = (int)(rc.right - rc.left) & ~1;
	int h = (int)(rc.bottom - rc.top) & ~1;
	if (w < 16) w = 1280;
	if (h < 16) h = 720;

	MTVideoExportParams params;
	ZeroMemory(&params, sizeof(params));
	params.codec = MTVC_H264_CPU;
	params.width = w;
	params.height = h;
	params.fps = 60;
	params.quality = 0;
	params.transparent = false;
	params.equirect360 = false;

	// restore the last-used export settings (conf/Video.ini), if any
	{
		MTConfFile vconf;
		if ((vconf.Initialize(_T("Video")) == 0) && (vconf.SetCurSection(_T("Video")) == 0)) {
			int v = (int)params.codec;
			vconf.GetInt(_T("Codec"), &v, (int)params.codec);
			if ((v >= 0) && (v <= (int)MTVC_HEVC_AMF)) params.codec = (MTVideoCodec)v;
			vconf.GetInt(_T("Width"),   &params.width,   params.width);
			vconf.GetInt(_T("Height"),  &params.height,  params.height);
			vconf.GetInt(_T("Fps"),     &params.fps,     params.fps);
			vconf.GetInt(_T("Quality"), &params.quality, params.quality);
			int tr = params.transparent ? 1 : 0;
			vconf.GetInt(_T("Transparent"), &tr, tr);
			params.transparent = (tr != 0);
			int eq = params.equirect360 ? 1 : 0;
			vconf.GetInt(_T("Equirect360"), &eq, eq);
			params.equirect360 = (eq != 0);
		}
	}

	MTVideoExportDlg dlg;
	if (!dlg.Show(m_hWnd, &params)) {
		// cancelled: return to the title screen (a song was opened for the export)
		m_Sequencer.Stop();
		m_SceneType = Title;
		_ChangePlayStatus(NoData);
		_SetupDX11Scene();
		return 0;
	}

	// remember the confirmed settings for next time (conf/Video.ini)
	{
		MTConfFile vconf;
		if (vconf.Initialize(_T("Video")) == 0) {
			vconf.SetCurSection(_T("Video"));
			vconf.SetInt(_T("Codec"),       (int)params.codec);
			vconf.SetInt(_T("Width"),       params.width);
			vconf.SetInt(_T("Height"),      params.height);
			vconf.SetInt(_T("Fps"),         params.fps);
			vconf.SetInt(_T("Quality"),     params.quality);
			vconf.SetInt(_T("Transparent"), params.transparent ? 1 : 0);
			vconf.SetInt(_T("Equirect360"), params.equirect360 ? 1 : 0);
		}
	}

	// 4) output path via Save dialog (default = MIDI base name + codec extension)
	const TCHAR* ext = MTVideoCodecExt(params.codec);
	TCHAR path[MAX_PATH] = _T("miditrail");
	{
		const TCHAR* base = _tcsrchr(midiPath, _T('\\'));
		base = (base != NULL) ? (base + 1) : midiPath;
		_tcsncpy_s(path, MAX_PATH, base, _TRUNCATE);
		TCHAR* dot = _tcsrchr(path, _T('.'));
		if (dot != NULL) *dot = _T('\0');
		_tcscat_s(path, MAX_PATH, ext);
	}
	TCHAR filter[64];
	_sntprintf_s(filter, _countof(filter), _TRUNCATE, _T("Video (*%s)|*%s|All Files|*.*|"), ext, ext);
	for (int k = 0; filter[k] != _T('\0'); k++) { if (filter[k] == _T('|')) filter[k] = _T('\0'); }

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = ext + 1;   // skip the leading dot
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	if (!GetSaveFileName(&ofn)) {
		m_Sequencer.Stop();
		m_SceneType = Title;
		_ChangePlayStatus(NoData);
		_SetupDX11Scene();
		return 0;
	}
	_tcsncpy_s(params.outPath, MAX_PATH, path, _TRUNCATE);

	int result = _ExportVideo(params);

	// return to the title screen (close the exported song so the session is not
	// left sitting on a file the user only opened to render)
	m_Sequencer.Stop();
	m_SceneType = Title;
	_ChangePlayStatus(NoData);
	_SetupDX11Scene();   // NoData -> detaches all scene components; the logo shows

	if (result == 0) {
		MessageBox(m_hWnd, _T("Video export finished."), _T("Export Video"), MB_OK | MB_ICONINFORMATION);
	} else {
		TCHAR msg[1280];
		if (m_ExportErrorMsg[0] != _T('\0')) {
			// show ffmpeg's own error (e.g. "Unknown encoder 'libx264'", odd size)
			_sntprintf_s(msg, _countof(msg), _TRUNCATE,
				_T("Video export failed.\n\nffmpeg:\n%s"), m_ExportErrorMsg);
		} else {
			_tcscpy_s(msg, _countof(msg),
				_T("Video export failed. Make sure ffmpeg is installed and on PATH."));
		}
		MessageBox(m_hWnd, msg, _T("Export Video"), MB_OK | MB_ICONWARNING);
	}
	return 0;
}

int MIDITrailApp::_SetupDX11Scene()
{
	if (m_Renderer11.GetDevice() == NULL) return 0;
	ID3D11Device* pDevice = m_Renderer11.GetDevice();
	ID3D11DeviceContext* pContext = m_Renderer11.GetContext();

	//(re)create the dashboard on the current device
	m_Dashboard11.Create(pDevice, pContext);

	//M4.15: background image (Graphic.ini [Background-image] ImageFilePath; empty = off)
	{
		TCHAR bgPath[_MAX_PATH] = { _T('\0') };
		if (m_GraphicConf.SetCurSection(_T("Background-image")) == 0) {
			m_GraphicConf.GetStr(_T("ImageFilePath"), bgPath, _MAX_PATH, _T(""));
		}
		m_BackgroundImage11.Create(pDevice, pContext, bgPath);
	}

	//Scene family: BOX = PianoRoll3D/2D (note boxes); RAIN = PianoRollRain
	//(falling notes); RING = PianoRollRing (circular notes).
	const TCHAR* pSceneName = NULL;
	enum { FAMILY_NONE, FAMILY_BOX, FAMILY_RAIN, FAMILY_RING } family = FAMILY_NONE;
	switch (m_SceneType) {
		case PianoRoll3D:   pSceneName = _T("PianoRoll3D");   family = FAMILY_BOX;  break;
		case PianoRoll2D:   pSceneName = _T("PianoRoll2D");   family = FAMILY_BOX;  break;
		case PianoRollRain:   pSceneName = _T("PianoRollRain");   family = FAMILY_RAIN; break;
		case PianoRollRain2D: pSceneName = _T("PianoRollRain2D"); family = FAMILY_RAIN; break;
		case PianoRollRing:   pSceneName = _T("PianoRollRing");   family = FAMILY_RING; break;
		default: break;
	}

	//no song / unsupported scene: detach everything
	if ((family == FAMILY_NONE) || (m_PlayStatus == NoData)) {
		m_DX11Family = DX11_FAMILY_NONE;
		m_Renderer11.SetNoteBox11(NULL);
		m_Renderer11.SetNoteRain11(NULL);
		m_Renderer11.SetNoteBoxRing11(NULL);
		m_Renderer11.SetKeyboard11(NULL);
		m_Renderer11.SetKeyboardRain11(NULL);
		m_Renderer11.SetNoteRipple11(NULL);
		m_Renderer11.SetNoteLyrics11(NULL);
		m_Renderer11.SetNoteBoxLive11(NULL);
		m_Renderer11.SetGridBox11(NULL);
		m_Renderer11.SetCamera11(NULL);
		m_Renderer11.SetDashboard11(NULL);
		m_Renderer11.SetTimeIndicator11(NULL);
		m_Renderer11.SetPictBoard11(NULL);
		m_Renderer11.SetGridRing11(NULL);
		m_Renderer11.SetTimeIndicatorRing11(NULL);
		m_Renderer11.SetPictBoardRing11(NULL);
		m_Renderer11.SetBackgroundImage11(NULL);
		m_Renderer11.SetStars11(NULL);
		return 0;
	}

	unsigned long totalNotes = 0;
	m_DX11Family = (family == FAMILY_RAIN) ? DX11_FAMILY_RAIN
	             : (family == FAMILY_RING) ? DX11_FAMILY_RING : DX11_FAMILY_BOX;

	bool isLiveBox = (m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF);
	//Live has its own conf (e.g. PianoRoll2DLive.ini) and its own saved viewpoint,
	//kept separate from playback. EVERY conf read below (note box / keyboard /
	//camera / background colour / starfield) uses this name in live mode.
	TCHAR liveSceneName[256] = { _T('\0') };
	_tcscpy_s(liveSceneName, 256, pSceneName);
	_tcscat_s(liveSceneName, 256, _T("Live"));
	//in live mode every scene reads its own *Live conf (matches the separate live
	//viewpoint). Live note drawing itself is only implemented for the box scenes.
	const TCHAR* sceneConfName = isLiveBox ? liveSceneName : pSceneName;

	//M5: background clear color from the scene conf [Color] BackGroundRGB. The DX9
	//scene's SetBGColor path is dead (m_pScene is NULL), so read it here directly.
	{
		MTConfFile bgConf;
		TCHAR hexColor[16] = { _T('\0') };
		if ((bgConf.Initialize(sceneConfName) == 0) && (bgConf.SetCurSection(_T("Color")) == 0)) {
			bgConf.GetStr(_T("BackGroundRGB"), hexColor, 16, _T("000000"));
			m_Renderer11.SetBackgroundColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
			//dashboard text color: DX9 read [Color] CaptionRGBA (shipped confs use the
			//translucent grey AAAAAAAA); the DX11 port hardcoded solid white, making the
			//counter/filename text look brighter than DX9. Restore the conf color.
			TCHAR hexCaption[16] = { _T('\0') };
			bgConf.GetStr(_T("CaptionRGBA"), hexCaption, 16, _T("FFFFFFFF"));
			D3DXCOLOR cap = DXColorUtil::MakeColorFromHexRGBA(hexCaption);
			m_Dashboard11.SetTextColor(cap.r, cap.g, cap.b, cap.a);
		}
	}

	//M4.8: per-channel pitch bend state (fed from MsgPitchBend); multi-keyboard
	//mode enables it (single keyboard disables it, matching the DX9 scenes).
	m_NotePitchBend11.Initialize();
	m_NotePitchBend11.SetEnable(true);

	if ((family == FAMILY_BOX) && isLiveBox) {
		//live monitor: real-time dynamic note boxes (no pre-built field / tick).
		//The note field, keyboard and grid mirror the playback 2D look; ripple /
		//lyrics / time-indicator need a pre-built note list, so they stay off.
		m_NoteBoxLive11.SetPitchBend(&m_NotePitchBend11);
		//live has no song: pass NULL seq data (matches DX9 MTNoteBoxLive)
		m_NoteBoxLive11.Create(pDevice, pContext, liveSceneName, NULL);
		m_Renderer11.SetNoteBoxLive11(&m_NoteBoxLive11);
		m_Renderer11.SetNoteRainLive11(NULL);
		m_Renderer11.SetNoteBox11(NULL);
		m_Renderer11.SetNoteRain11(NULL);
		m_Renderer11.SetNoteBoxRing11(NULL);
		m_Renderer11.SetGridRing11(NULL);
		m_Renderer11.SetTimeIndicatorRing11(NULL);
		m_Renderer11.SetPictBoardRing11(NULL);

		//keyboard with live key-press reaction (single keyboard, NULL seq = no song;
		//keys are lit directly from real-time MIDI via SetNoteOnLive/OffLive).
		m_Kbd11.Create(pDevice, pContext, liveSceneName, NULL, true);
		m_Kbd11.SetPitchBend(&m_NotePitchBend11);
		m_Renderer11.SetKeyboard11(&m_Kbd11);
		m_Renderer11.SetKeyboardRain11(NULL);

		//grid box uses the live conf (no song); off if it can't build cleanly
		m_Renderer11.SetGridBox11(NULL);

		//live ripple: real-time note-on driven (no song), clocked by timeGetTime
		m_NoteRipple11.SetPitchBend(&m_NotePitchBend11);
		m_NoteRipple11.Create(pDevice, pContext, liveSceneName, NULL);
		m_Renderer11.SetNoteRipple11(&m_NoteRipple11);
		m_Renderer11.SetNoteLyrics11(NULL);
		m_Renderer11.SetTimeIndicator11(NULL);
		m_Renderer11.SetPictBoard11(NULL);
		totalNotes = 0;
	}
	else if (family == FAMILY_BOX) {
		m_Renderer11.SetNoteBoxLive11(NULL);
		m_Renderer11.SetNoteRainLive11(NULL);
		//single keyboard -> collapse all ports onto one note row to match it
		m_Renderer11.DrawLoadingScreen("Building note field...", 0.35f);
		//drive the loading bar from inside the (huge) instance-buffer build
		DXNoteBox11::SetBuildProgressCallback(&MIDITrailApp::_LoadProgressCallback, this);
		m_NoteBox11.Create(pDevice, pContext, pSceneName, &m_SeqData, m_IsSingleKeyboard11);
		DXNoteBox11::SetBuildProgressCallback(NULL, NULL);
		m_NoteBox11.SetPitchBend(&m_NotePitchBend11);
		m_Renderer11.SetNoteBox11(&m_NoteBox11);
		m_Renderer11.SetNoteRain11(NULL);
		m_Renderer11.SetNoteBoxRing11(NULL);
		m_Renderer11.SetGridRing11(NULL);
		m_Renderer11.SetTimeIndicatorRing11(NULL);
		m_Renderer11.SetPictBoardRing11(NULL);
		totalNotes = m_NoteBox11.GetNoteCount();

		//M3: faithful keyboard (real key geometry + HDKeyboard.png + Mod transform)
		m_Renderer11.DrawLoadingScreen("Building keyboard...", 0.88f);
		m_Kbd11.Create(pDevice, pContext, pSceneName, &m_SeqData, m_IsSingleKeyboard11);
		m_Kbd11.SetPitchBend(&m_NotePitchBend11);
		m_Renderer11.SetKeyboard11(&m_Kbd11);
		m_Renderer11.SetKeyboardRain11(NULL);

		//M3: note ripple effect (note-on driven)
		m_NoteRipple11.SetPitchBend(&m_NotePitchBend11);   //M4.23: ripple follows the bend
		m_Renderer11.DrawLoadingScreen("Building ripple...", 0.93f);
		m_NoteRipple11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetNoteRipple11(&m_NoteRipple11);

		//note lyrics (0x05 text drawn over the played notes); View > Lyrics toggles it
		m_NoteLyrics11.SetPitchBend(&m_NotePitchBend11);
		m_NoteLyrics11.SetRingMode(false);   //planar layout for 3D/2D
		m_NoteLyrics11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetNoteLyrics11(m_isEnableLyrics ? &m_NoteLyrics11 : NULL);

		//M3: grid box (piano-roll grid lines, spatial reference)
		m_Renderer11.DrawLoadingScreen("Building grid...", 0.97f);
		m_GridBox11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetGridBox11(&m_GridBox11);

		//M4.4: time indicator (translucent "now playing" playback section)
		m_TimeIndicator11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetTimeIndicator11(&m_TimeIndicator11);

		//M4.5: picture board disabled in the Mod scenes (see M4.5a)
		m_Renderer11.SetPictBoard11(NULL);
	}
	else if ((family == FAMILY_RAIN) && isLiveBox) {
		//live monitor: real-time falling notes (Rain). No song / pre-built field.
		m_NoteRainLive11.SetPitchBend(&m_NotePitchBend11);
		m_NoteRainLive11.Create(pDevice, pContext, liveSceneName, NULL);
		m_Renderer11.SetNoteRainLive11(&m_NoteRainLive11);
		m_Renderer11.SetNoteBoxLive11(NULL);
		m_Renderer11.SetNoteBox11(NULL);
		m_Renderer11.SetNoteRain11(NULL);
		m_Renderer11.SetNoteBoxRing11(NULL);
		m_Renderer11.SetGridRing11(NULL);
		m_Renderer11.SetTimeIndicatorRing11(NULL);
		m_Renderer11.SetPictBoardRing11(NULL);

		//live Rain keyboard with key-press reaction (the rain-specific keyboard so it
		//sits at the right place/orientation; lit directly from real-time MIDI).
		m_KbdRain11.Create(pDevice, pContext, liveSceneName, NULL);
		m_KbdRain11.SetPitchBend(&m_NotePitchBend11);
		m_Renderer11.SetKeyboardRain11(&m_KbdRain11);
		m_Renderer11.SetKeyboard11(NULL);

		m_Renderer11.SetGridBox11(NULL);
		m_Renderer11.SetNoteRipple11(NULL);
		m_Renderer11.SetNoteLyrics11(NULL);
		m_Renderer11.SetTimeIndicator11(NULL);
		m_Renderer11.SetPictBoard11(NULL);
		totalNotes = 0;
	}
	else if ((family == FAMILY_RING) && isLiveBox) {
		//live monitor: real-time circular notes (Ring). Reuse the box-live renderer
		//slot, built in ring mode (MTNoteDesignRing live placement).
		m_NoteBoxLive11.SetPitchBend(&m_NotePitchBend11);
		m_NoteBoxLive11.Create(pDevice, pContext, liveSceneName, NULL, true);   //ringMode
		m_Renderer11.SetNoteBoxLive11(&m_NoteBoxLive11);
		m_Renderer11.SetNoteRainLive11(NULL);
		m_Renderer11.SetNoteBox11(NULL);
		m_Renderer11.SetNoteRain11(NULL);
		m_Renderer11.SetNoteBoxRing11(NULL);
		m_Renderer11.SetGridRing11(NULL);
		m_Renderer11.SetTimeIndicatorRing11(NULL);

		//ring "keyboard" = the textured picture board (DX9 EffectPianoKeyboard). It is
		//static (no song needed), so build it for live too.
		m_PictBoardRing11.Create(pDevice, pContext, liveSceneName, NULL);
		m_Renderer11.SetPictBoardRing11(&m_PictBoardRing11);

		m_Renderer11.SetKeyboard11(NULL);
		m_Renderer11.SetKeyboardRain11(NULL);
		m_Renderer11.SetGridBox11(NULL);
		//ring live ripple (real-time note-on driven, ring positions)
		m_NoteRipple11.SetPitchBend(&m_NotePitchBend11);
		m_NoteRipple11.Create(pDevice, pContext, liveSceneName, NULL, true);
		m_Renderer11.SetNoteRipple11(&m_NoteRipple11);
		m_Renderer11.SetNoteLyrics11(NULL);
		m_Renderer11.SetTimeIndicator11(NULL);
		m_Renderer11.SetPictBoard11(NULL);
		totalNotes = 0;
	}
	else if (family == FAMILY_RAIN) {
		m_Renderer11.SetNoteRainLive11(NULL);
		m_NoteRain11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_NoteRain11.SetPitchBend(&m_NotePitchBend11);
		m_Renderer11.SetNoteRain11(&m_NoteRain11);
		m_Renderer11.SetNoteBox11(NULL);
		m_Renderer11.SetNoteBoxRing11(NULL);
		m_Renderer11.SetGridRing11(NULL);
		m_Renderer11.SetTimeIndicatorRing11(NULL);
		m_Renderer11.SetPictBoardRing11(NULL);
		totalNotes = m_NoteRain11.GetNoteCount();

		//M4.7b: Rain keyboard (non-Mod, per channel, rises with playback in Y)
		m_KbdRain11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_KbdRain11.SetPitchBend(&m_NotePitchBend11);
		m_Renderer11.SetKeyboardRain11(&m_KbdRain11);

		//Rain has no box keyboard / grid / ripple / lyrics / time indicator / picture board.
		m_Renderer11.SetKeyboard11(NULL);
		m_Renderer11.SetNoteRipple11(NULL);
		m_Renderer11.SetNoteLyrics11(NULL);
		m_Renderer11.SetNoteBoxLive11(NULL);
		m_Renderer11.SetGridBox11(NULL);
		m_Renderer11.SetTimeIndicator11(NULL);
		m_Renderer11.SetPictBoard11(NULL);
	}
	else {  // FAMILY_RING
		//M4.9 circular notes + M4.13 ring decorations (grid / board / time
		//indicator). Ring note ripple not ported yet.
		m_NoteBoxRing11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_NoteBoxRing11.SetPitchBend(&m_NotePitchBend11);
		m_Renderer11.SetNoteBoxRing11(&m_NoteBoxRing11);
		totalNotes = m_NoteBoxRing11.GetNoteCount();

		m_GridRing11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetGridRing11(&m_GridRing11);
		m_TimeIndicatorRing11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetTimeIndicatorRing11(&m_TimeIndicatorRing11);
		m_PictBoardRing11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetPictBoardRing11(&m_PictBoardRing11);

		//M4.13: ring ripple (reuse MTNoteRipple11 in ring mode)
		m_NoteRipple11.SetPitchBend(&m_NotePitchBend11);   //M4.23: ripple follows the bend
		m_NoteRipple11.Create(pDevice, pContext, pSceneName, &m_SeqData, true);
		m_Renderer11.SetNoteRipple11(&m_NoteRipple11);

		//ring lyrics (1.4.1 PianoRollRing lyrics, ported to DX11): reuse MTNoteLyrics11
		//in ring mode so 0x05 text is laid on the ring like the notes.
		m_NoteLyrics11.SetPitchBend(&m_NotePitchBend11);
		m_NoteLyrics11.SetRingMode(true);
		m_NoteLyrics11.Create(pDevice, pContext, pSceneName, &m_SeqData);
		m_Renderer11.SetNoteLyrics11(m_isEnableLyrics ? &m_NoteLyrics11 : NULL);

		m_Renderer11.SetNoteBox11(NULL);
		m_Renderer11.SetNoteRain11(NULL);
		m_Renderer11.SetKeyboard11(NULL);
		m_Renderer11.SetKeyboardRain11(NULL);
		m_Renderer11.SetGridBox11(NULL);
		m_Renderer11.SetTimeIndicator11(NULL);
		m_Renderer11.SetNoteLyrics11(NULL);   //ring scene has no lyrics
		m_Renderer11.SetNoteBoxLive11(NULL);  //ring scene has no live notes
		m_Renderer11.SetNoteRainLive11(NULL);
		m_Renderer11.SetPictBoard11(NULL);
	}

	//M4: dashboard - file name (basename), totals, reset counters
	{
		SMBarList barList;
		unsigned long totalBars = 0;
		if (m_SeqData.GetBarList(&barList) == 0) totalBars = barList.GetSize();

		m_NpsNoteCount = 0;
		m_Dashboard11.Reset();
		//playback: ensure the dashboard is in normal (non-monitor) mode; live mode
		//is (re)enabled by _OnMenuStartMonitoring after this runs.
		if ((m_PlayStatus != MonitorON) && (m_PlayStatus != MonitorOFF)) {
			m_Dashboard11.SetMonitorMode(false, "");
		}
		m_Dashboard11.SetFileName(m_DashFileNameA);
		m_Dashboard11.SetTotals(m_SeqData.GetTotalPlayTime(), totalBars, totalNotes);
		//loading resets the play speed to 100% (m_PlaySpeedRatio above); mirror that
		//on the dashboard so a leftover "SPEED:NNN%" from the previous song clears.
		m_Dashboard11.SetPlaySpeedRatio(m_PlaySpeedRatio);
		m_Renderer11.SetDashboard11(&m_Dashboard11);   //show only once a song is loaded
	}

	//M4.16: starfield (scene conf [Stars] NumberOfStars; 0 = off). The sky
	//follows the camera and is drawn behind everything in every scene family.
	{
		int numStars = 0;
		MTConfFile starsConf;
		if (starsConf.Initialize(sceneConfName) == 0) {
			if (starsConf.SetCurSection(_T("Stars")) == 0) {
				starsConf.GetInt(_T("NumberOfStars"), &numStars, 2000);
			}
		}
		m_Stars11.Create(pDevice, pContext, numStars);
	}

	//M2.5: real first-person camera (free look + playback follow scroll).
	//Rain progresses along Y (falling notes); box/ring scenes along X (time).
	//ced 20260628: if this is just a re-setup of the SAME scene (switching songs in
	//the same view mode, or a renderer re-init on resize/AA), keep the user's current
	//viewpoint instead of snapping back to default/last-saved. Capture it NOW - while
	//the OLD note-design + current tick are still active - so the now-line-relative
	//position transfers correctly; reapplied after Initialize/Reset + the restore below.
	bool keepView = m_HasPrevView && (m_SceneType == m_PrevViewSceneType)
			&& (isLiveBox == m_PrevViewIsLive);
	float kvX = 0, kvY = 0, kvZ = 0, kvPhi = 0, kvTheta = 0, kvRoll = 0, kvAutoRoll = 0;
	if (keepView) {
		m_FpCam11.GetViewpointParam(&kvX, &kvY, &kvZ, &kvPhi, &kvTheta, &kvRoll);
		kvAutoRoll = m_IsAutoRollMode11 ? m_FpCam11.GetAutoRollVelocity() : 0.0f;
	}

	//Live: no song -> init with the live conf + NULL seq data (matches DX9;
	//passing the empty &m_SeqData makes the camera divide by a 0 time-division
	//and crash in TransformDX11).
	if (isLiveBox) {
		m_FpCam11.Initialize(m_hWnd, liveSceneName, NULL);   // live: own conf, no song
	} else {
		m_FpCam11.Initialize(m_hWnd, pSceneName, &m_SeqData);
	}
	// reset the playback tick baseline so the scroll offset is applied from tick 0
	// (Initialize does not clear it; a stale value - e.g. left by a video export -
	// would otherwise bake a wrong scroll offset into the viewpoint => blank scene).
	m_FpCam11.Reset();
	// seed the keyboard-envelope tempo with the song's initial tempo (tempo meta
	// events update it during playback); guard against a 0 / missing tempo.
	{ unsigned long bpm = m_SeqData.GetTempoBPM(); m_CurSongBPM = (bpm > 0) ? bpm : 120; }
	m_FpCam11.SetProgressDirection((family == FAMILY_RAIN)
			? MTFirstPersonCam::DirY : MTFirstPersonCam::DirX);
	//Live uses the SAME default + saved viewpoint as playback 2D so the orientation
	//(roll/angle the user tuned) matches; the live now-line sits at tick 0 (no scroll).
	if (family == FAMILY_RING) m_FpCam11.SetDefaultViewpointRing();
	else m_FpCam11.SetDefaultViewpoint();

	//M3 (DX11): restore the saved viewpoint (m_ViewConf "Viewpoint-<sceneName>")
	//so it persists across restarts; if none saved, keep the default above.
	{
		const float SENT = -1.0e30f;
		TCHAR section[256] = { _T('\0') };
		_tcscat_s(section, 256, _T("Viewpoint-"));
		_tcscat_s(section, 256, pSceneName);
		//live monitor keeps its own viewpoint, separate from playback
		if (isLiveBox) _tcscat_s(section, 256, _T("Live"));
		if (m_ViewConf.SetCurSection(section) == 0) {
			float x = SENT, y = 0, z = 0, phi = 0, theta = 0, roll = 0, autoRoll = 0;
			m_ViewConf.GetFloat(_T("X"), &x, SENT);
			if (x != SENT) {   // a viewpoint was saved for this scene
				m_ViewConf.GetFloat(_T("Y"), &y, 0.0f);
				m_ViewConf.GetFloat(_T("Z"), &z, 0.0f);
				m_ViewConf.GetFloat(_T("Phi"), &phi, 0.0f);
				m_ViewConf.GetFloat(_T("Theta"), &theta, 0.0f);
				m_ViewConf.GetFloat(_T("ManualRollAngle"), &roll, 0.0f);
				m_ViewConf.GetFloat(_T("AutoRollVelocity"), &autoRoll, 0.0f);
				m_FpCam11.SetViewpointParam(x, y, z, phi, theta, roll, autoRoll);
				m_IsAutoRollMode11 = (autoRoll != 0.0f);
			}
		}
	}
	//ced 20260628: restore the captured live viewpoint (overrides default/saved above)
	//so the same-scene re-setup keeps exactly where the user was looking.
	if (keepView) {
		m_FpCam11.SetViewpointParam(kvX, kvY, kvZ, kvPhi, kvTheta, kvRoll, kvAutoRoll);
		m_IsAutoRollMode11 = (kvAutoRoll != 0.0f);
	}
	m_PrevViewSceneType = m_SceneType;
	m_PrevViewIsLive = isLiveBox;
	m_HasPrevView = true;
	m_Renderer11.SetCamera11(&m_FpCam11);

	//M4.10: honor the View-menu effect toggles (keyboard/ripple/grid/etc.)
	_ApplyDX11Visibility();

	// free the shared note-list cache now that every component has built its
	// buffers (large for Black MIDI; rebuilt on the next scene setup if needed).
	m_SeqData.ReleaseMergedNoteList();

	return 0;
}

// >>> add 20120728 yossiepon begin

//******************************************************************************
// MIDIファイル追加読み込み
// ファイル名に「portX」が含まれる場合、Xをポート番号とみなす（a-Z:大小同一視）
// ファイル名に「chXX」が含まれる場合、XXをチャンネル番号と見なす（00-99)
//******************************************************************************
int MIDITrailApp::_AddMIDIFile(
		const TCHAR* pFilePath
	)
{
	int result = 0;
	TCHAR* pPath = NULL;
	TCHAR smfTempPath[_MAX_PATH] = {_T('\0')};
	TCHAR smfDumpPath[_MAX_PATH] = {_T('\0')};
	SMSeqData tmpSeqData;
	SMFileReader smfReader;
	short portNo = -1;
	short chNo = -1;

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
		WCHAR smfDumpPathW[_MAX_PATH] = { L'\0' };
		MultiByteToWideChar(CP_ACP, 0, smfDumpPath, -1, smfDumpPathW, _MAX_PATH);
		smfReader.SetLogPath(smfDumpPathW);
	}

	//ファイルを一時シーケンスに読み込み
	result = smfReader.Load(pPath, &tmpSeqData);
	if (result != 0) goto EXIT;

	//ファイル名にポート番号が含まれていれば抽出
	char *pPortNo = strstr(pPath, "port");
	if(pPortNo != NULL) {
		portNo = tolower(*(pPortNo + 4)) - 'a';
	}

	//ファイル名にチャンネル番号が含まれていれば抽出
	char *pChNo = strstr(pPath, "ch");
	if(pChNo != NULL) {
		char bufChNo[3];
		strncpy_s(bufChNo, 3, pChNo + 2, 2);
		bufChNo[2] = '\0';
		chNo = atoi(bufChNo) - 1;
	}

	//一時シーケンスをマージ
	m_SeqData.AddSequence(tmpSeqData, portNo, chNo);

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

// <<< add 20120728 yossiepon end

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

	//�V�[���� MIDI IN �f�o�C�X����o�^�iM5/DX11�F m_pScene �� NULL �Ȃ̂ŃX�L�b�v�j
	if (pScene != NULL) {
		result = pScene->SetParam("MIDI_IN_DEVICE_NAME", devName);
		if (result != 0) goto EXIT;
	}

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
		//MIDITHRU is a convenience pass-through to a synth; if the OUT device can't
		//be opened (e.g. no/busy device) the monitor + visualization should still
		//run, so swallow the error here instead of aborting the whole monitor start.
		(void)pLiveMonitor->SetOutPortDev(devName);
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
	MTScene::MTViewParamMap viewParamMap;

	//モニタ状態の確認
	if ((m_PlayStatus == MonitorOFF) || (m_PlayStatus == MonitorON)) {
		isMonitor = true;
	}

	//現在の視点を退避
	if (m_pScene != NULL) {
		m_pScene->GetViewParam(&viewParamMap);
	}

	//シーン破棄
	if (m_pScene != NULL) {
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}

	//レンダラ終了
	m_Renderer11.Terminate();

	//ユーザー設定ウィンドウサイズ変更
	result = _SetWindowSize();
	if (result != 0) goto EXIT;

	//レンダラ初期化
	result = m_Renderer11.Initialize(m_hWnd, m_MultiSampleType, false, m_SuperSample);
	if (result != 0) goto EXIT;

	//M3 (DX11): the device was recreated - rebuild every DX11 component for the
	//loaded song, else they reference the destroyed device ("Device lost").
	_SetupDX11Scene();

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

	//視点を復帰
	if (m_pScene != NULL) {
		m_pScene->SetViewParam(&viewParamMap);
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

	//ced 20260628: 曲アンロード(NoData)時のみマウスカメラ掴みを解除する。
	//以前は Stop でも解除していたが、それだと停止中にマウスで見回せなくなるため、
	//停止中はマウスカメラを維持する（カーソルを戻したい時は左クリックでトグル off）。
	if ((status == NoData) && m_IsMouseCamMode11) {
		m_IsMouseCamMode11 = false;
		m_FpCam11.SetMouseCamMode(false);
	}

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
// >>> add 20120728 yossiepon begin
		IDM_ADD_FILE,
// <<< add 20120728 yossiepon end
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
		IDM_VIEW_SINGLEKEYBOARD,
		IDM_ENABLE_PIANOKEYBOARD,
		IDM_ENABLE_RIPPLE,
		IDM_ENABLE_LYRICS,
		IDM_ENABLE_PITCHBEND,
		IDM_ENABLE_PITCHBEND_ALLNOTES,
		IDM_ENABLE_STARS,
		IDM_ENABLE_COUNTER,
		IDM_ENABLE_BACKGROUNDIMAGE,
// >>> add 20180404 yossiepon begin
		IDM_ENABLE_TIMEINDICATOR,
		IDM_ENABLE_GRIDBOX,
// <<< add 20180404 yossiepon end
		IDM_RESET_VIEWPOINT,
		IDM_VIEWPOINT2,
		IDM_VIEWPOINT3,
		IDM_AUTO_SAVE_VIEWPOINT,
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
// >>> add 20120728 yossiepon begin
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_ADD_FILE
// <<< add 20120728 yossiepon end
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
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_SINGLEKEYBOARD
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_PIANOKEYBOARD
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_RIPPLE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_LYRICS
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_PITCHBEND
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_PITCHBEND_ALLNOTES
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_STARS
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_COUNTER
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_BACKGROUNDIMAGE
// >>> add 20180404 yossiepon begin
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_TIMEINDICATOR
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_GRIDBOX
// <<< add 20180404 yossiepon end
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_RESET_VIEWPOINT
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_VIEWPOINT2
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_VIEWPOINT3
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_AUTO_SAVE_VIEWPOINT
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

	//M5: DX9 scene rendering removed. The DX11 path renders via m_Renderer11 and
	//the *11 components (set up in _SetupDX11Scene), so this runs with no scene.
	m_pScene = NULL;
	return 0;
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
	int val = 0;

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	//自動視点保存：メニューの On/Off を設定ファイルから復元（既定は有効）
	result = m_ViewConf.GetInt(_T("AutoSaveViewpoint"), &autoSaveViewpoint, 1);
	if (result != 0) goto EXIT;

	m_isAutoSaveViewpoint = (autoSaveViewpoint == 1);

	//ced 20260628: View 表示設定の自動保存フラグ（既定は無効）
	result = m_ViewConf.GetInt(_T("AutoSaveViewSettings"), &val, 0);
	if (result != 0) goto EXIT;
	m_isAutoSaveViewSettings = (val == 1);

	//有効時のみ、保存済みの View 表示トグルを復元する。
	//（未設定キーはコンストラクタ既定値をそのまま採用。実際の描画反映は次の
	//  シーン構築時 _ApplyDX11Visibility / メニューチェックは _UpdateMenuCheckmark）
	if (m_isAutoSaveViewSettings) {
		m_ViewConf.GetInt(_T("EnablePianoKeyboard"),     &val, m_isEnablePianoKeyboard    ? 1 : 0); m_isEnablePianoKeyboard     = (val == 1);
		m_ViewConf.GetInt(_T("EnableRipple"),            &val, m_isEnableRipple           ? 1 : 0); m_isEnableRipple            = (val == 1);
		m_ViewConf.GetInt(_T("EnableLyrics"),            &val, m_isEnableLyrics           ? 1 : 0); m_isEnableLyrics            = (val == 1);
		m_ViewConf.GetInt(_T("EnablePitchBend"),         &val, m_isEnablePitchBend        ? 1 : 0); m_isEnablePitchBend         = (val == 1);
		m_ViewConf.GetInt(_T("EnablePitchBendAllNotes"), &val, m_isEnablePitchBendAllNotes? 1 : 0); m_isEnablePitchBendAllNotes = (val == 1);
		m_ViewConf.GetInt(_T("EnableStars"),             &val, m_isEnableStars            ? 1 : 0); m_isEnableStars             = (val == 1);
		m_ViewConf.GetInt(_T("EnableCounter"),           &val, m_isEnableCounter          ? 1 : 0); m_isEnableCounter           = (val == 1);
		m_ViewConf.GetInt(_T("EnableBackgroundImage"),   &val, m_isEnableBackgroundImage  ? 1 : 0); m_isEnableBackgroundImage   = (val == 1);
		m_ViewConf.GetInt(_T("EnableTimeIndicator"),     &val, m_isEnableTimeIndicator    ? 1 : 0); m_isEnableTimeIndicator     = (val == 1);
		m_ViewConf.GetInt(_T("EnableGridBox"),           &val, m_isEnableGridBox          ? 1 : 0); m_isEnableGridBox           = (val == 1);
		m_ViewConf.GetInt(_T("SingleKeyboard"),          &val, m_IsSingleKeyboard11       ? 1 : 0); m_IsSingleKeyboard11        = (val == 1);
	}

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

	//ced 20260628: View 表示設定の自動保存フラグ
	result = m_ViewConf.SetInt(_T("AutoSaveViewSettings"), m_isAutoSaveViewSettings ? 1 : 0);
	if (result != 0) goto EXIT;

	//有効時のみ、現在の View 表示トグルを書き出す（無効時は触らない＝旧値を残すが
	//復元側もフラグで gate しているので影響しない）
	if (m_isAutoSaveViewSettings) {
		m_ViewConf.SetInt(_T("EnablePianoKeyboard"),     m_isEnablePianoKeyboard     ? 1 : 0);
		m_ViewConf.SetInt(_T("EnableRipple"),            m_isEnableRipple            ? 1 : 0);
		m_ViewConf.SetInt(_T("EnableLyrics"),            m_isEnableLyrics            ? 1 : 0);
		m_ViewConf.SetInt(_T("EnablePitchBend"),         m_isEnablePitchBend         ? 1 : 0);
		m_ViewConf.SetInt(_T("EnablePitchBendAllNotes"), m_isEnablePitchBendAllNotes ? 1 : 0);
		m_ViewConf.SetInt(_T("EnableStars"),             m_isEnableStars             ? 1 : 0);
		m_ViewConf.SetInt(_T("EnableCounter"),           m_isEnableCounter           ? 1 : 0);
		m_ViewConf.SetInt(_T("EnableBackgroundImage"),   m_isEnableBackgroundImage   ? 1 : 0);
		m_ViewConf.SetInt(_T("EnableTimeIndicator"),     m_isEnableTimeIndicator     ? 1 : 0);
		m_ViewConf.SetInt(_T("EnableGridBox"),           m_isEnableGridBox           ? 1 : 0);
		m_ViewConf.SetInt(_T("SingleKeyboard"),          m_IsSingleKeyboard11        ? 1 : 0);
	}

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

	//M3 (DX11): no MTScene -> save the live camera viewpoint to m_ViewConf so it
	//persists across restarts (section "Viewpoint-<sceneName>", now-line relative).
	if (m_pScene == NULL) {
		const TCHAR* pName = _DX11SceneName();
		if ((m_DX11Family != DX11_FAMILY_NONE) && (pName != NULL)) {
			float x = 0, y = 0, z = 0, phi = 0, theta = 0, roll = 0;
			m_FpCam11.GetViewpointParam(&x, &y, &z, &phi, &theta, &roll);
			float autoRoll = m_IsAutoRollMode11 ? m_FpCam11.GetAutoRollVelocity() : 0.0f;
			_tcscat_s(section, 256, _T("Viewpoint-"));
			_tcscat_s(section, 256, pName);
			//live monitor keeps its own viewpoint, separate from playback
			if ((m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF)) {
				_tcscat_s(section, 256, _T("Live"));
			}
			if (m_ViewConf.SetCurSection(section) == 0) {
				m_ViewConf.SetFloat(_T("X"), x);
				m_ViewConf.SetFloat(_T("Y"), y);
				m_ViewConf.SetFloat(_T("Z"), z);
				m_ViewConf.SetFloat(_T("Phi"), phi);
				m_ViewConf.SetFloat(_T("Theta"), theta);
				m_ViewConf.SetFloat(_T("ManualRollAngle"), roll);
				m_ViewConf.SetFloat(_T("AutoRollVelocity"), autoRoll);
			}
		}
		goto EXIT;
	}
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
// ced 20260703: 視点を「保存済み(なければ既定)」へ戻す（DX11 ライブカメラ）
//   手動停止時のリセット先を、ハードコード既定ではなく _LoadViewpoint と同じ
//   保存視点にする。ロード時 (Initialize 直後) の視点復元ロジックと同一手順。
//******************************************************************************
void MIDITrailApp::_ResetViewpointToSaved()
{
	//まず既定視点（保存が無ければこれが最終結果）
	if (m_SceneType == PianoRollRing) m_FpCam11.SetDefaultViewpointRing();
	else m_FpCam11.SetDefaultViewpoint();

	const TCHAR* pName = _DX11SceneName();
	if ((m_DX11Family == DX11_FAMILY_NONE) || (pName == NULL)) return;

	//保存済み視点があれば上書き（conf "Viewpoint-<scene>"、ライブは "...Live"）
	const float SENT = -1.0e30f;
	TCHAR section[256] = { _T('\0') };
	_tcscat_s(section, 256, _T("Viewpoint-"));
	_tcscat_s(section, 256, pName);
	if ((m_PlayStatus == MonitorON) || (m_PlayStatus == MonitorOFF)) _tcscat_s(section, 256, _T("Live"));
	if (m_ViewConf.SetCurSection(section) == 0) {
		float x = SENT, y = 0, z = 0, phi = 0, theta = 0, roll = 0, autoRoll = 0;
		m_ViewConf.GetFloat(_T("X"), &x, SENT);
		if (x != SENT) {   // a viewpoint was saved for this scene
			m_ViewConf.GetFloat(_T("Y"), &y, 0.0f);
			m_ViewConf.GetFloat(_T("Z"), &z, 0.0f);
			m_ViewConf.GetFloat(_T("Phi"), &phi, 0.0f);
			m_ViewConf.GetFloat(_T("Theta"), &theta, 0.0f);
			m_ViewConf.GetFloat(_T("ManualRollAngle"), &roll, 0.0f);
			m_ViewConf.GetFloat(_T("AutoRollVelocity"), &autoRoll, 0.0f);
			m_FpCam11.SetViewpointParam(x, y, z, phi, theta, roll, autoRoll);
			m_IsAutoRollMode11 = (autoRoll != 0.0f);
		}
	}
}

//******************************************************************************
// グラフィック設定読み込み
//******************************************************************************
int MIDITrailApp::_LoadGraphicConf()
{
	int result = 0;
	int multiSampleType = 0;
	int superSample = 0;   //ced 20260628

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

	//ced 20260628: スーパーサンプリング(SSAA)倍率（同 Anti-aliasing セクション）
	result = m_GraphicConf.GetInt(_T("SuperSample"), &superSample, MT_GRAPHIC_SUPER_SAMPLE_DEF);
	if (result != 0) goto EXIT;
	if ((DX_SUPER_SAMPLE_MIN <= superSample) && (superSample <= DX_SUPER_SAMPLE_MAX)) {
		m_SuperSample = superSample;
	}
	else {
		m_SuperSample = 1;   //OFF
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

	//ced 20260628: View 表示設定の保存（有効時は現在のトグルを書き出す）。
	//エラーが発生しても処理を続行する。
	_SaveSceneConf();

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
			if (m_pScene != NULL) result = m_pScene->OnPlayStart(NULL /*M5: DX9 device removed; m_pScene is always NULL here*/);
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

	//フォルダ演奏（1.4.1 移植）
	_CheckMenuItem(IDM_FOLDER_PLAYBACK, m_isFolderPlayback);

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

	//M4.6c: single keyboard (DX11)
	_CheckMenuItem(IDM_VIEW_SINGLEKEYBOARD, m_IsSingleKeyboard11);

	//ピアノキーボード表示
	_CheckMenuItem(IDM_ENABLE_PIANOKEYBOARD, m_isEnablePianoKeyboard);

	//波紋効果
	_CheckMenuItem(IDM_ENABLE_RIPPLE, m_isEnableRipple);

	//歌詞
	_CheckMenuItem(IDM_ENABLE_LYRICS, m_isEnableLyrics);

	//ピッチベンド効果
	_CheckMenuItem(IDM_ENABLE_PITCHBEND, m_isEnablePitchBend);

	//�s�b�`�x���h�i�`�����l���S�́j
	_CheckMenuItem(IDM_ENABLE_PITCHBEND_ALLNOTES, m_isEnablePitchBendAllNotes);

	//星表示
	_CheckMenuItem(IDM_ENABLE_STARS, m_isEnableStars);

	//カウンタ表示
	_CheckMenuItem(IDM_ENABLE_COUNTER, m_isEnableCounter);

	//背景画像表示
	_CheckMenuItem(IDM_ENABLE_BACKGROUNDIMAGE, m_isEnableBackgroundImage);

// >>> add 20180404 yossiepon begin
	//タイムインジケータ表示
	_CheckMenuItem(IDM_ENABLE_TIMEINDICATOR, m_isEnableTimeIndicator);

	//グリッドボックス表示
	_CheckMenuItem(IDM_ENABLE_GRIDBOX, m_isEnableGridBox);
// <<< add 20180404 yossiepon end

	//自動視点保存
	_CheckMenuItem(IDM_AUTO_SAVE_VIEWPOINT, m_isAutoSaveViewpoint);
	_CheckMenuItem(IDM_AUTO_SAVE_VIEWSETTINGS, m_isAutoSaveViewSettings);  //ced 20260628

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
// >>> add 20180404 yossiepon begin
		m_pScene->SetEffect(MTScene::EffectTimeIndicator, m_isEnableTimeIndicator);
		m_pScene->SetEffect(MTScene::EffectGridBox, m_isEnableGridBox);
// <<< add 20180404 yossiepon end
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

	isSupport = true;  // M1b: DX11 supports 32-bit index buffers
	maxVertexIndex = 0xFFFFFFFF;

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
