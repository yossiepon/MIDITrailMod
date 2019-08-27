//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail アプリケーションクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "resource.h"
#include "Commdlg.h"
#include "YNBaseLib.h"
#include "SMIDILib.h"
#include "DXRenderer.h"
#include "MTScene.h"
#include "MTWindowSizeCfgDlg.h"
#include "MTMIDIOUTCfgDlg.h"
#include "MTMIDIINCfgDlg.h"
#include "MTGraphicCfgDlg.h"
#include "MTHowToViewDlg.h"
#include "MTAboutDlg.h"
#include "MTCmdLineParser.h"
#include "MTGamePadCtrl.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
#define MAX_LOADSTRING  (100)

//ウィンドウスタイル
//  WS_OVERLAPPEDWINDOW から次のスタイルを削ったもの
//    WS_THICKFRAME   サイズ変更可
#define MIDITRAIL_WINDOW_STYLE  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

//後続起動プロセスのファイルパスポスト通知
#define WM_FILEPATH_POSTED  (WM_USER + 100)

//メニュースタイル制御
#define MT_MENU_NUM        (32)
#define MT_PLAYSTATUS_NUM  (6)

//デバイスロスト警告メッセージ
#define MIDITRAIL_MSG_DEVICELOST  _T("Direct3D device is lost.")

//タイマーID
#define MIDITRAIL_TIMER_CHECK_KEY  (1)

//二重起動抑止用ミューテクス名称
#define MIDITRAIL_MUTEX     _T("yknk.MIDITrail")

//メールスロット名称
#define MIDITRAIL_MAILSLOT  _T("\\\\.\\mailslot\\yknk\\MIDITrail")


//******************************************************************************
// MIDITrail アプリケーションクラス
//******************************************************************************
class MIDITrailApp
{
public:

	//コンストラクタ／デストラクタ
	MIDITrailApp(void);
	virtual ~MIDITrailApp(void);

	//初期化
	int Initialize(HINSTANCE hInstance, LPTSTR pCmdLine, int nCmdShow);

	//実行
	int Run();

	//停止
	int Terminate();

private:

	//----------------------------------------------------------------
	//パラメータ定義
	//----------------------------------------------------------------
	//演奏状態
	enum PlayStatus {
		NoData,			//データなし
		Stop,			//停止状態
		Play,			//再生中
		Pause,			//一時停止
		MonitorOFF,		//モニタ停止
		MonitorON		//モニタ中
	};

	//シーン種別
	//TAG:シーン追加
	enum SceneType {
		Title,			//タイトル
		PianoRoll3D,	//ピアノロール3D
		PianoRoll2D,	//ピアノロール2D
		PianoRollRain,	//ピアノロールレイン
		PianoRollRain2D	//ピアノロールレイン2D
	};

	//シーケンサメッセージ
	typedef struct {
		unsigned long param1;
		unsigned long param2;
	} MTSequencerMsg;

	//最新シーケンサメッセージ
	typedef struct {
		bool isRecvPlayTime;
		bool isRecvTempo;
		bool isRecvBar;
		bool isRecvBeat;
		MTSequencerMsg playTime;
		MTSequencerMsg tempo;
		MTSequencerMsg bar;
		MTSequencerMsg beat;
	} MTSequencerLastMsg;

private:

	//----------------------------------------------------------------
	//メンバ定義
	//----------------------------------------------------------------
	//ウィンドウプロシージャ制御用ポインタ
	static MIDITrailApp* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//アプリケーション二重起動抑止制御
	HANDLE m_hAppMutex;
	HANDLE m_hMailSlot;
	bool m_isExitApp;

	//コマンドラインパーサ
	MTCmdLineParser m_CmdLineParser;

	//ウィンドウ系
	HWND m_hWnd;
	HACCEL m_Accel;
	TCHAR m_Title[MAX_LOADSTRING];
	TCHAR m_WndClassName[MAX_LOADSTRING];
	bool m_isFullScreen;
	HMENU m_hMenu;

	//レンダリング系
	DXRenderer m_Renderer;
	MTScene* m_pScene;
	unsigned long m_MultiSampleType;

	//FPS表示系
	DWORD m_PrevTime;
	DWORD m_FPSCount;

	//MIDI制御系
	SMSeqData m_SeqData;
	SMSequencer m_Sequencer;
	SMRcpConv m_RcpConv;
	SMMsgQueue m_MsgQueue;
	SMLiveMonitor m_LiveMonitor;

	//演奏状態
	PlayStatus m_PlayStatus;
	bool m_isRepeat;
	bool m_isRewind;
	bool m_isOpenFileAfterStop;
	MTSequencerLastMsg m_SequencerLastMsg;
	unsigned long m_PlaySpeedRatio;

	//表示効果
	bool m_isEnablePianoKeyboard;
	bool m_isEnableRipple;
	bool m_isEnablePitchBend;
	bool m_isEnableStars;
	bool m_isEnableCounter;
	bool m_isEnableFileName;
	bool m_isEnableBackgroundImage;

	//シーン種別
	SceneType m_SceneType;
	SceneType m_SelectedSceneType;

	//ウィンドウサイズ設定ダイアログ
	MTWindowSizeCfgDlg m_WindowSizeCfgDlg;

	//MIDI OUT設定ダイアログ
	MTMIDIOUTCfgDlg m_MIDIOUTCfgDlg;

	//MIDI IN設定ダイアログ
	MTMIDIINCfgDlg m_MIDIINCfgDlg;

	//グラフィック設定ダイアログ
	MTGraphicCfgDlg m_GraphicCfgDlg;

	//操作方法ダイアログ
	MTHowToViewDlg m_HowToViewDlg;

	//バージョン情報ダイアログ
	MTAboutDlg m_AboutDlg;

	//設定ファイル
	YNConfFile m_MIDIConf;
	YNConfFile m_ViewConf;
	YNConfFile m_GraphicConf;

	//プレーヤー制御
	int m_AllowMultipleInstances;
	int m_AutoPlaybackAfterOpenFile;

	//スキップ制御
	int m_SkipBackTimeSpanInMsec;
	int m_SkipForwardTimeSpanInMsec;

	//演奏スピード制御
	unsigned long m_SpeedStepInPercent;
	unsigned long m_MaxSpeedInPercent;

	//自動視点保存
	bool m_isAutoSaveViewpoint;

	//次回オープン対象ファイルパス
	TCHAR m_NextFilePath[_MAX_PATH];

	//ゲームパッド制御
	MTGamePadCtrl m_GamePadCtrl;

	//ゲームパッド用視点番号
	int m_GamePadViewPointNo;

	//----------------------------------------------------------------
	//メソッド定義
	//----------------------------------------------------------------
	//ウィンドウ制御
	int _RegisterClass(HINSTANCE hInstance);
	int _CreateWindow(HINSTANCE hInstance, int nCmdShow);
	int _SetWindowSize();
	int _SetWindowSizeFullScreen();

	//設定ファイル初期化
	int _InitConfFile();

	//ウィンドウプロシージャ
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//メニューイベント処理
	int _OnMenuFileOpen();
	int _OnMenuPlay();
	int _OnMenuStop();
	int _OnMenuRepeat();
	int _OnMenuSkipBack();
	int _OnMenuSkipForward();
	int _OnMenuPlaySpeedDown();
	int _OnMenuPlaySpeedUp();
	int _OnMenuStartMonitoring();
	int _OnMenuStopMonitoring();
	int _OnMenuAutoSaveViewpoint();
	int _OnMenuResetViewpoint();
	int _OnMenuViewpoint(unsigned long viewpointNo);
	int _OnMenuSaveViewpoint();
	int _OnMenuEnableEffect(MTScene::EffectType type);
	int _OnMenuWindowSize();
	int _OnMenuFullScreen();
	int _OnMenuOptionMIDIOUT();
	int _OnMenuOptionMIDIIN();
	int _OnMenuOptionGraphic();
	int _OnMenuManual();
	int _OnMenuSelectSceneType(SceneType type);
	int _OnFilePathPosted();

	//その他イベント処理
	int _SequencerMsgProc();
	int _OnRecvSequencerMsg(unsigned long wParam, unsigned long lParam);
	int _OnMouseButtonDown(UINT button, WPARAM wParam, LPARAM lParam);
	int _OnMouseMove(UINT button, WPARAM wParam, LPARAM lParam);
	int _OnKeyDown(WPARAM wParam, LPARAM lParam);
	int _OnDropFiles(WPARAM wParam, LPARAM lParam);

	int _SelectMIDIFile(TCHAR* pFilePath,  unsigned long bufSize, bool* pIsSelected);
	int _LoadMIDIFile(const TCHAR* pFilePath);
	void _UpdateFPS();
	int _SetPortDev(SMSequencer* pSequencer);
	int _SetMonitorPortDev(SMLiveMonitor* pLiveMonitor, MTScene* pScene);
	int _ChangeWindowSize();
	int _ChangePlayStatus(PlayStatus status);
	int _ChangeMenuStyle();
	int _CreateScene(SceneType type, SMSeqData* pSeqData);
	int _LoadSceneType();
	int _SaveSceneType();
	int _LoadSceneConf();
	int _SaveSceneConf();
	int _LoadViewpoint();
	int _SaveViewpoint();
	int _LoadGraphicConf();
	int _LoadPlayerConf();
	int _OnDestroy();
	int _RebuildScene();
	int _DispHowToView();
	int _UpdateMenuCheckmark();
	void _CheckMenuItem(UINT uIDCheckItem, bool isEnable);
	void _UpdateEffect();
	int _ParseCmdLine(LPTSTR pCmdLine);
	int _StartTimer();
	int _StopTimer();
	int _OnTimer(WPARAM timerId);
	int _CheckRenderer();
	int _AutoConfigMIDIOUT();
	int _SearchMicrosoftWavetableSynth(std::string& productName);
	int _CheckMultipleInstances(bool* pIsExitApp);
	int _CreateMailSlot();
	int _PostFilePathToFirstMIDITrail(LPTSTR pCmdLine);
	int _StopPlaybackAndOpenFile(TCHAR* pFilePath);
	int _FileOpenProc(TCHAR* pFilePath);
	int _ToggleFullScreen();
	int _ShowMenu();
	int _HideMenu();
	int _GamePadProc();
	int _ChangeViewPoint(int step);

};

