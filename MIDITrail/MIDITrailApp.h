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
#include "DXRenderer11.h"
#include "MTKeyboard11.h"
#include "MTFirstPersonCam.h"
#include "DXNoteBox11.h"
#include "DXNoteRain11.h"
#include "DXNoteBoxRing11.h"
#include "MTGridRing11.h"
#include "MTTimeIndicatorRing11.h"
#include "MTPictBoardRing11.h"
#include "MTBackgroundImage11.h"
#include "MTStars11.h"
#include "MTNoteRipple11.h"
#include "MTNoteLyrics11.h"
#include "MTNoteBoxLive11.h"
#include "MTNoteRainLive11.h"
#include "MTGridBox11.h"
#include "MTDashboard11.h"
#include "MTTimeIndicator11.h"
#include "MTPictBoard11.h"
#include "MTKeyboardRain11.h"
#include "MTNotePitchBend.h"
#include "MTScene.h"
#include "MTWindowSizeCfgDlg.h"
#include "MTMIDIOUTCfgDlg.h"
#include "MTMIDIINCfgDlg.h"
#include "MTGraphicCfgDlg.h"
#include "MTColorCfgDlg.h"
#include "MTConfigManager11.h"
#include "MTHowToViewDlg.h"
#include "MTAboutDlg.h"
#include "MTCmdLineParser.h"
#include "MTFileList.h"
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
//TAG:シーン追加
// >>> modify 20191219 yossiepon begin
#define MT_MENU_NUM        (40)
// <<< modify 20191219 yossiepon end

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
		PianoRollRain2D,	//ピアノロールレイン2D
		PianoRollRing		//ピアノロールリング
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
	DXRenderer11 m_Renderer11;
	MTKeyboard11 m_Kbd11;  // M2: DX11 keyboard (temporary)
	MTFirstPersonCam m_FpCam11;  // M2.5: real first-person camera for DX11 path
	bool m_IsMouseCamMode11;  // M3: DX11 mouse-look toggle
	bool m_IsAutoRollMode11;  // M3: DX11 auto-roll toggle
	// ced 20260629: Config Manager(ImGui) を開いている間カメラ入力を止める際の状態退避。
	// 閉じたらマウスカメラを元に戻す（開いて閉じると見回せなくなる問題の対策）。
	bool m_CfgWasVisible;       // 前フレームの Config Manager 表示状態
	bool m_MouseCamBeforeCfg;   // Config を開く直前のマウスカメラ ON/OFF
	// ced 20260628: keep the live viewpoint across a re-setup of the SAME scene
	// (song switch in same view mode / renderer re-init on resize/AA) so the camera
	// doesn't snap back to default/last-saved every time _SetupDX11Scene runs.
	bool m_HasPrevView;
	SceneType m_PrevViewSceneType;
	bool m_PrevViewIsLive;
	DXNoteBox11 m_NoteBox11;  // M3: DX11 instanced note field
	DXNoteRain11 m_NoteRain11;  // M4.7: DX11 instanced falling-note field (Rain scene)
	MTKeyboardRain11 m_KbdRain11;  // M4.7b: DX11 Rain-scene keyboard
	DXNoteBoxRing11 m_NoteBoxRing11;  // M4.9: DX11 Ring-scene circular notes
	MTGridRing11 m_GridRing11;  // M4.13: DX11 Ring grid
	MTTimeIndicatorRing11 m_TimeIndicatorRing11;  // M4.13: DX11 Ring time indicator
	MTPictBoardRing11 m_PictBoardRing11;  // M4.13: DX11 Ring picture board
	MTBackgroundImage11 m_BackgroundImage11;  // M4.15: DX11 background image
	MTStars11 m_Stars11;  // M4.16: DX11 starfield
	MTNotePitchBend m_NotePitchBend11;  // M4.8: per-channel pitch bend state (DX11 path)
	MTNoteRipple11 m_NoteRipple11;  // M3: DX11 note ripple effect
	MTNoteLyrics11 m_NoteLyrics11;  // DX11 note lyrics (0x05 text over notes)
	MTNoteBoxLive11 m_NoteBoxLive11;  // DX11 live-monitor dynamic note boxes (box + ring)
	MTNoteRainLive11 m_NoteRainLive11;  // DX11 live-monitor dynamic falling notes (Rain)
	unsigned long m_LiveNoteCount;    // notes played since monitoring started (dashboard)
	MTGridBox11 m_GridBox11;  // M3: DX11 grid box (piano-roll grid lines)
	MTDashboard11 m_Dashboard11;  // M4: DX11 on-screen info dashboard
	MTTimeIndicator11 m_TimeIndicator11;  // M4.4: DX11 time indicator (playback section)
	MTPictBoard11 m_PictBoard11;  // M4.5: DX11 picture board
	unsigned long m_NpsNoteCount;  // M4: note-on count for NPS
	unsigned long m_NpsLastSec;
	char m_DashFileNameA[MAX_PATH];  // M4: loaded file name for the dashboard (survives re-init)
	// active DX11 scene family: which note/keyboard components are CREATED for the
	// current scene. Only these are fed/reset per frame; the others may never have
	// been created (e.g. loaded straight into Rain) so touching them would crash.
	enum { DX11_FAMILY_NONE = 0, DX11_FAMILY_BOX = 1, DX11_FAMILY_RAIN = 2, DX11_FAMILY_RING = 3 };
	int m_DX11Family;

	// current SONG tempo (BPM, from tempo meta events; not scaled by play speed). Used
	// to map the keyboard's KeyDown/UpDuration (ms) to ticks for the press envelope.
	unsigned long m_CurSongBPM;

	int _SetupDX11Scene();  // (re)create the DX11 scene components for the loaded song
	void _ApplyDX11Visibility();  // attach/detach DX11 components per the View effect toggles
	const TCHAR* _DX11SceneName();  // conf/scene name for the current SceneType (NULL if none)
	void _FeedDX11Tick(unsigned long tick, unsigned long playMs);  // drive the active components' tick
	int _ExportVideo(const struct MTVideoExportParams& params);    // M6: offline video export
	int _OnMenuExportVideo();  // M6: "Export Video..." menu handler
	bool m_isExporting;  // M6: guard against re-entry while exporting
	TCHAR m_ExportErrorMsg[1024];  // M6: ffmpeg stderr tail captured on a failed export
	wchar_t m_LoadFilePathW[MAX_PATH];  // true (Unicode) path of the file being opened
	bool m_isLoading;
	MTScene* m_pScene;
	unsigned long m_MultiSampleType;
	unsigned long m_SuperSample;   //ced 20260628: SSAA 倍率（1=OFF）

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
	bool m_IsSingleKeyboard11;  // M4.6c: DX11 box keyboard - single (default) vs per-port
	bool m_isEnableRipple;
	bool m_isEnableLyrics;   // ced 20260713: lyrics toggle (used to follow m_isEnableRipple)
	bool m_isEnablePitchBend;
	bool m_isEnablePitchBendAllNotes;  // M4.22: bend the whole channel, not just sounding notes
	bool m_isEnableStars;
	bool m_isEnableCounter;
	bool m_isEnableFileName;
	bool m_isEnableBackgroundImage;
// >>> add 20180404 yossiepon begin
	bool m_isEnableTimeIndicator;
	bool m_isEnableGridBox;
// <<< add 20180404 yossiepon end

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

	//カラー設定ダイアログ（1.4.1 移植）
	MTColorCfgDlg m_ColorCfgDlg;

	//設定マネージャ（conf/*.ini を GUI 編集：Mod Mod 独自）
	MTConfigManager11 m_ConfigMgr11;

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

	//ced 20260628: View 表示トグル群を再起動後も保持する（Auto save view settings）
	bool m_isAutoSaveViewSettings;

	//次回オープン対象ファイルパス
	TCHAR m_NextFilePath[_MAX_PATH];

	// >>> add ced 20260627: 1.4.1 folder playback / file navigation
	//フォルダ演奏（フォルダ内 MIDI を連続再生）
	bool m_isFolderPlayback;
	//メニューバー表示
	bool m_isEnableMenuBar;
	//フォルダ演奏時の曲間ディレイ(msec)
	int m_DelayBetweenSongsInMsec;
	//フォルダ内 MIDI ファイルリスト
	MTFileList m_MIDIFileList;
	// <<< add ced 20260627

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
// >>> add 20120728 yossiepon begin
	int _OnMenuFileAdd();
// <<< add 20120728 yossiepon end
// >>> add ced 20260627: 1.4.1 features
	int _OnMenuOpenFolder();
	int _OnMenuPreviousFile();
	int _OnMenuNextFile();
	int _OnMenuFolderPlayback();
	int _OnMenuMenuBar();
	int _OnMenuMyViewpoint(unsigned long viewpointNo);
	int _OnMenuSaveMyViewpoint(unsigned long viewpointNo);
	int _SelectFolder(WCHAR* pFolderPath, unsigned long bufSize, bool* pIsSelected);
	int _MakeFileListWithFolder(const WCHAR* pFolderPath, MTFileList* pFileList);
	int _StopPlaybackAndOpenFolder(const WCHAR* pFolderPath);
	int _OpenFileW(const WCHAR* pFilePathW);
	int _ToggleMenuBar();
	int _MoveToMyViewpoint(unsigned long viewpointNo);
	int _SaveMyViewpoint(unsigned long viewpointNo);
// <<< add ced 20260627
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
	int _OnMenuAutoSaveViewSettings();  //ced 20260628
	int _OnMenuResetViewpoint();
	int _OnMenuViewpoint(unsigned long viewpointNo);
	int _OnMenuSaveViewpoint();
	int _OnMenuEnableEffect(MTScene::EffectType type);
	int _OnMenuWindowSize();
	int _OnMenuFullScreen();
	int _OnMenuOptionMIDIOUT();
	int _OnMenuOptionMIDIIN();
	int _OnMenuOptionGraphic();
	int _OnMenuOptionColor();
	int _OnMenuConfigManager();
	int _OnMenuManual();
	int _OnMenuSelectSceneType(SceneType type);
	int _OnMenuToggleSingleKeyboard();
	int _OnMenuTogglePitchBendAllNotes();
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
	static void _LoadProgressCallback(unsigned long current, unsigned long total, void* user);
	static void _ParseProgressCallback(unsigned long current, unsigned long total, void* user);
// >>> add 20120728 yossiepon begin
	int _AddMIDIFile(const TCHAR* pFilePath);
// <<< add 20120728 yossiepon end
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
	//ced 20260703: reset the live DX11 camera to the saved viewpoint (conf "Viewpoint-<scene>"),
	//falling back to the scene default when none is saved. Used by the manual-stop viewpoint
	//reset so it returns to the user's saved viewpoint instead of the hard-coded default.
	void _ResetViewpointToSaved();
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

