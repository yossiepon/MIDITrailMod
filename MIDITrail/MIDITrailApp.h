//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail アプリケーションクラス
//
// Copyright (C) 2010-2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "resource.h"
#include "Commdlg.h"
#include "YNBaseLib.h"
#include "SMIDILib.h"
#include "DXRenderer.h"
#include "MTScene.h"
#include "MTOperationPanelCtrl.h"
#include "MTViewModeDlg.h"
#include "MTWindowSizeCfgDlg.h"
#include "MTMIDIOUTCfgDlg.h"
#include "MTMIDIINCfgDlg.h"
#include "MTGraphicCfgDlg.h"
#include "MTWavetableSynthCfgDlg.h"
#include "MTColorCfgDlg.h"
#include "MTHowToViewDlg.h"
#include "MTAboutDlg.h"
#include "MTCmdLineParser.h"
#include "MTGamePadCtrl.h"
#include "MTFileList.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
#define MAX_LOADSTRING  (256)

//ウィンドウスタイル
//  WS_OVERLAPPEDWINDOW から次のスタイルを削ったもの
//    WS_THICKFRAME   サイズ変更可
#define MIDITRAIL_WINDOW_STYLE  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

//後続起動プロセスのファイルパスポスト通知
#define WM_FILEPATH_POSTED  (WM_USER + 100)

//メニュースタイル制御
//TAG:シーン追加
#define MT_MENU_NUM        (49)

//デバイスロスト警告メッセージ
#define MIDITRAIL_MSG_DEVICELOST  _T("Direct3D device is lost.")

//ファイルなし警告メッセージ
#define MIDITRAIL_MSG_FILE_NOT_FOUND  _T("MIDI file (*.mid) not found.")

//タイマーID
#define MIDITRAIL_TIMER_CHECK_KEY           (1)
#define MIDITRAIL_TIMER_PLAY                (2)
#define MIDITRAIL_TIMER_OPEN_FILE_AND_PLAY  (3)

//二重起動抑止用ミューテクス名称
#define MIDITRAIL_MUTEX     _T("yknk.MIDITrail")

//メールスロット名称
#define MIDITRAIL_MAILSLOT  _T("\\\\.\\mailslot\\yknk\\MIDITrail")

//ウィンドウタイトル  ex.: "MIDITrail - file_name.mid - FPS:60.0"
#define MIDITRAIL_WINDOW_TITLE			L"MIDITrail"
#define MIDITRAIL_WINDOW_TITLE_FILE		L"MIDITrail - %s"
#define MIDITRAIL_WINDOW_TITLE_FILES	L"MIDITrail - [%d/%d] %s"
#define MIDITRAIL_WINDOW_TITLE_FPS		L"%s - FPS:%.1f"


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

	//GDI+トークン
	ULONG_PTR m_GDIPlusToken;

	//コマンドラインパーサ
	MTCmdLineParser m_CmdLineParser;

	//ウィンドウ系
	HWND m_hWnd;
	HACCEL m_Accel;
	WCHAR m_Title[MAX_LOADSTRING];
	WCHAR m_WndClassName[MAX_LOADSTRING];
	bool m_isFullScreen;
	bool m_isEnableMenuBar;
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
	TCHAR m_MIDIINDevName[MAXPNAMELEN];

	//演奏状態
	PlayStatus m_PlayStatus;
	bool m_isRepeat;
	bool m_isFolderPlayback;
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
	bool m_isEnableGridLine;
	bool m_isEnableTimeIndicator;

	//シーン種別
	SceneType m_SceneType;
	SceneType m_SelectedSceneType;

	//操作パネル
	MTOperationPanelCtrl m_OperationPanelCtrl;
	
	//ビューモード選択ダイアログ
	MTViewModeDlg m_ViewModeDlg;

	//ウィンドウサイズ設定ダイアログ
	MTWindowSizeCfgDlg m_WindowSizeCfgDlg;

	//MIDI OUT設定ダイアログ
	MTMIDIOUTCfgDlg m_MIDIOUTCfgDlg;

	//MIDI IN設定ダイアログ
	MTMIDIINCfgDlg m_MIDIINCfgDlg;
	
	//ウェーブテーブルシンセサイザ設定ダイアログ
	MTWavetableSynthCfgDlg m_WavetableSynthCfgDlg;

	//グラフィック設定ダイアログ
	MTGraphicCfgDlg m_GraphicCfgDlg;

	//カラー設定ダイアログ
	MTColorCfgDlg m_ColorCfgDlg;

	//操作方法ダイアログ
	MTHowToViewDlg m_HowToViewDlg;

	//バージョン情報ダイアログ
	MTAboutDlg m_AboutDlg;

	//設定ファイル
	YNConfFile m_PlayerConf;
	YNConfFile m_MIDIConf;
	YNConfFile m_ViewConf;
	YNConfFile m_WindowConf;
	YNConfFile m_SynthConf;
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

	//演奏制御
	int m_DelayBetweenSongsInMsec;

	//自動視点保存
	bool m_isAutoSaveViewpoint;

	//次回オープン対象ファイルパス
	WCHAR m_NextFilePath[_MAX_PATH];

	//ゲームパッド制御
	MTGamePadCtrl m_GamePadCtrl;

	//ゲームパッド用視点番号
	int m_GamePadViewPointNo;

	//ウェーブテーブルファイルパス
	WCHAR m_WavetableFilePath[_MAX_PATH];

	//MIDIデータファイルリスト
	MTFileList m_MIDIFileList;

	//----------------------------------------------------------------
	//メソッド定義
	//----------------------------------------------------------------
	//GDI+
	int _InitGDIPlus();
	void _ReleaseGDIPlus();

	//ウィンドウ制御
	int _RegisterClass(HINSTANCE hInstance);
	int _CreateWindow(HINSTANCE hInstance, int nCmdShow);
	int _CreateOperationPanel();
	int _SetWindowSize();
	int _SetWindowSizeFullScreen();

	//設定ファイル初期化
	int _InitConfFile();

	//ウィンドウプロシージャ
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//メニューイベント処理
	int _OnMenuOpenFile();
	int _OnMenuOpenFolder();
	int _OnMenuPreviousFile();
	int _OnMenuNextFile();
	int _OnMenuPlay();
	int _OnMenuStop();
	int _OnMenuRepeat();
	int _OnMenuFolderPlayback();
	int _OnMenuSkipBack();
	int _OnMenuSkipForward();
	int _OnMenuPlaySpeedDown();
	int _OnMenuPlaySpeedUp();
	int _OnMenuStartMonitoring();
	int _OnMenuStopMonitoring();
	int _OnMenuAutoSaveViewpoint();
	int _OnMenuResetViewpoint();
	int _OnMenuViewpoint(unsigned long viewpointNo);
	int _OnMenuMyViewpoint(unsigned long viewpointNo);
	int _OnMenuSaveMyViewpoint(unsigned long viewpointNo);
	int _OnMenuSaveViewpoint();
	int _OnMenuEnableEffect(MTScene::EffectType type);
	int _OnMenuViewMode();
	int _OnMenuOperationPanel();
	int _OnMenuWindowSize();
	int _OnMenuFullScreen();
	int _OnMenuMenuBar();
	int _OnMenuOptionMIDIOUT();
	int _OnMenuOptionMIDIIN();
	int _OnMenuOptionSynthesizer();
	int _OnMenuOptionGraphic();
	int _OnMenuOptionColor();
	int _OnMenuHowToView();
	int _OnMenuManual();
	int _OnMenuAbout();
	int _OnMenuSelectSceneType(SceneType type);
	int _OnFilePathPosted();

	//その他イベント処理
	int _SequencerMsgProc();
	int _OnRecvSequencerMsg(unsigned long wParam, unsigned long lParam);
	int _OnMouseButtonDown(UINT button, WPARAM wParam, LPARAM lParam);
	int _OnMouseMove(UINT button, WPARAM wParam, LPARAM lParam);
	int _OnKeyDown(WPARAM wParam, LPARAM lParam);
	int _OnDropFiles(WPARAM wParam, LPARAM lParam);

	int _SelectMIDIFile(WCHAR* pFilePath,  unsigned long bufSize, bool* pIsSelected);
	int _SelectFolder(WCHAR* pFolderPath, unsigned long bufSize, bool* pIsSelected);
	int _LoadMIDIFile(const WCHAR* pFilePath);
	void _UpdateWindowTitle(const WCHAR* pFileName);
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
	int _LoadEffectStatus();
	int _SaveEffectStatus();
	int _LoadViewpoint();
	int _SaveViewpoint();
	int _MoveToMyViewpoint(unsigned long viewpointNo);
	int _SaveMyViewpoint(unsigned long viewpointNo);
	int _LoadGraphicConf();
	int _LoadPlayerConf();
	int _OnDestroy();
	int _RebuildScene();
	int _DispHowToView();
	int _UpdateMenuCheckmark();
	void _CheckMenuItem(UINT uIDCheckItem, bool isEnable);
	void _UpdateEffect();
	void _UpdateOperationPanelCheckmark();
	int _ParseCmdLine();
	int _StartTimer();
	int _StopTimer();
	int _StartTimer_Play(int delayBetweenSongsInMsec);
	int _StartTimer_OpenFileAndPlay(int delayBetweenSongsInMsec);
	int _OnTimer(WPARAM timerId);
	int _CheckRenderer();
	int _AutoConfigMIDIOUT();
	int _SearchMicrosoftWavetableSynth(std::string& productName);
	int _CheckMultipleInstances(bool* pIsExitApp);
	int _CreateMailSlot();
	int _PostFilePathToFirstMIDITrail();
	int _StopPlaybackAndOpenFile(const WCHAR* pFilePath);
	int _StopPlaybackAndOpenFolder(const WCHAR* pFolderPath);
	int _FileOpenProc(const WCHAR* pFilePath);
	int _ToggleFullScreen();
	int _ToggleMenuBar();
	int _ShowMenu();
	int _HideMenu();
	int _GamePadProc();
	int _ChangeViewPoint(int step);
	int _MakeFileListWithFolder(const WCHAR* pFolderPath, MTFileList* pFileList);
	WCHAR* _GetWavetableFilePath();
	SM_WAVETABLE_SYNTH_PARAM _GetWavetableSynthParam();
	void _BeforeDisplayDialog();
	void _AfterDisplayDialog();

};

