//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail application class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "Resource.h"
#include "Commdlg.h"
#include "YNBaseLib.h"
#include "SMIDILib.h"
#include "DXRenderer11.h"
#include "MTLoadingScreen11.h"
#include "IMTScene11.h"
#include "MTWindowSizeCfgDlg.h"
#include "MTMIDIOUTCfgDlg.h"
#include "MTMIDIINCfgDlg.h"
#include "MTGraphicCfgDlg.h"
#include "MTColorCfgDlg.h"
#include "MTHowToViewDlg.h"
#include "MTAboutDlg.h"
#include "MTCmdLineParser.h"
#include "MTGamePadCtrl.h"
#include "MTFileList.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// Parameter definitions
//******************************************************************************
#define MAX_LOADSTRING  (256)

//Window style
//  WS_OVERLAPPEDWINDOW with the following style removed
//    WS_THICKFRAME   Resizable
#define MIDITRAIL_WINDOW_STYLE  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

//File path post notification from a subsequently launched process
#define WM_FILEPATH_POSTED  (WM_USER + 100)

//Menu style control
//TAG: add scene
#define MT_MENU_NUM        (47+1)
#define MT_PLAYSTATUS_NUM  (6)

//Device lost warning message
#define MIDITRAIL_MSG_DEVICELOST  _T("Direct3D device has been removed.\nThe graphics driver may have crashed or been updated.\nMIDITrail will attempt to recover.")

//No-file warning message
#define MIDITRAIL_MSG_FILE_NOT_FOUND  _T("MIDI file (*.mid) not found.")

//Timer IDs
#define MIDITRAIL_TIMER_CHECK_KEY           (1)
#define MIDITRAIL_TIMER_PLAY                (2)
#define MIDITRAIL_TIMER_OPEN_FILE_AND_PLAY  (3)

//Mutex name for duplicate-launch prevention
#define MIDITRAIL_MUTEX     _T("yknk.MIDITrail")

//Mailslot name
#define MIDITRAIL_MAILSLOT  _T("\\\\.\\mailslot\\yknk\\MIDITrail")

//Window title  ex.: "MIDITrail - file_name.mid - FPS:60.0"
//#define MIDITRAIL_WINDOW_TITLE			L"MIDITrail"
//#define MIDITRAIL_WINDOW_TITLE_FILE		L"MIDITrail - %s"
//#define MIDITRAIL_WINDOW_TITLE_FILES		L"MIDITrail - [%d/%d] %s"

#define MIDITRAIL_WINDOW_TITLE_FILE			L" - %s"
#define MIDITRAIL_WINDOW_TITLE_FILES		L" - [%d/%d] %s"
#define MIDITRAIL_WINDOW_TITLE_FPS			L"%s - FPS:%.1f"


//******************************************************************************
// MIDITrail application class
//******************************************************************************
class MIDITrailApp
{
public:

	//Constructor / Destructor
	MIDITrailApp(void);
	virtual ~MIDITrailApp(void);

	//Initialize
	int Initialize(HINSTANCE hInstance, LPTSTR pCmdLine, int nCmdShow);

	//Run
	int Run();

	//Terminate
	int Terminate();

private:

	//----------------------------------------------------------------
	//Parameter definitions
	//----------------------------------------------------------------
	//Playback status
	enum PlayStatus {
		NoData,			//No data
		Stop,			//Stopped
		Play,			//Playing
		Pause,			//Paused
		MonitorOFF,		//Monitor off
		MonitorON		//Monitor on
	};

	//Scene type
	//TAG: add scene
	enum SceneType {
		Title,				//Title
		PianoRoll3D,		//Piano roll 3D
		PianoRoll2D,		//Piano roll 2D
		PianoRollRain,		//Piano roll rain
		PianoRollRain2D,	//Piano roll rain 2D
		PianoRollRing		//Piano roll ring
	};

	//Sequencer message
	typedef struct {
		unsigned long param1;
		unsigned long param2;
	} MTSequencerMsg;

	//Latest sequencer message
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
	//Member definitions
	//----------------------------------------------------------------
	//Pointer for window procedure control
	static MIDITrailApp* m_pThis;

	//Application instance
	HINSTANCE m_hInstance;

	//Duplicate-launch prevention control
	HANDLE m_hAppMutex;
	HANDLE m_hMailSlot;
	bool m_isExitApp;

	//Command line parser
	MTCmdLineParser m_CmdLineParser;

	//Window-related
	HWND m_hWnd;
	HACCEL m_Accel;
	WCHAR m_Title[MAX_LOADSTRING];
	WCHAR m_TitleBase[MAX_LOADSTRING];
	WCHAR m_WndClassName[MAX_LOADSTRING];
	bool m_isFullScreen;
	bool m_isEnableMenuBar;
	HMENU m_hMenu;

	//Rendering-related
	DXRenderer11 m_Renderer;
	IMTScene11* m_pScene;
	unsigned long m_MultiSampleType;

	//Loading screen
	MTLoadingScreen11 m_LoadingScreen;

	//FPS display-related
	DWORD m_PrevTime;
	DWORD m_FPSCount;

	//MIDI control-related
	SMSeqData m_SeqData;
	SMSequencer m_Sequencer;
	SMRcpConv m_RcpConv;
	SMMsgQueue m_MsgQueue;
	SMLiveMonitor m_LiveMonitor;
	TCHAR m_MIDIINDevName[MAXPNAMELEN];

	//Playback status
	PlayStatus m_PlayStatus;
	bool m_isRepeat;
	bool m_isFolderPlayback;
	bool m_isRewind;
	bool m_isOpenFileAfterStop;
	MTSequencerLastMsg m_SequencerLastMsg;
	unsigned long m_PlaySpeedRatio;

	//Display effects
	bool m_isEnablePianoKeyboard;
	bool m_isEnableRipple;
	bool m_isEnablePitchBend;
	bool m_isEnableStars;
	bool m_isEnableCounter;
	bool m_isEnableFileName;
	bool m_isEnableBackgroundImage;
	bool m_isEnableGridLine;
	bool m_isEnableTimeIndicator;

	//Scene type
	SceneType m_SceneType;
	SceneType m_SelectedSceneType;

	//Window size settings dialog
	MTWindowSizeCfgDlg m_WindowSizeCfgDlg;

	//MIDI OUT settings dialog
	MTMIDIOUTCfgDlg m_MIDIOUTCfgDlg;

	//MIDI IN settings dialog
	MTMIDIINCfgDlg m_MIDIINCfgDlg;

	//Graphics settings dialog
	MTGraphicCfgDlg m_GraphicCfgDlg;

	//Color settings dialog
	MTColorCfgDlg m_ColorCfgDlg;

	//How-to-view dialog
	MTHowToViewDlg m_HowToViewDlg;

	//About dialog
	MTAboutDlg m_AboutDlg;

	//Configuration file
	YNConfFile m_MIDIConf;
	YNConfFile m_ViewConf;
	YNConfFile m_GraphicConf;

	//Player control
	int m_AllowMultipleInstances;
	int m_AutoPlaybackAfterOpenFile;

	//Skip control
	int m_SkipBackTimeSpanInMsec;
	int m_SkipForwardTimeSpanInMsec;

	//Playback speed control
	unsigned long m_SpeedStepInPercent;
	unsigned long m_MaxSpeedInPercent;

	//Playback control
	int m_DelayBetweenSongsInMsec;

	//Auto-save viewpoint
	bool m_isAutoSaveViewpoint;

	//File path to open next
	WCHAR m_NextFilePath[_MAX_PATH];

	//Gamepad control
	MTGamePadCtrl m_GamePadCtrl;

	//Viewpoint number for gamepad
	int m_GamePadViewPointNo;

	//MIDI data file list
	MTFileList m_MIDIFileList;

	//----------------------------------------------------------------
	//Method definitions
	//----------------------------------------------------------------
	//Window control
	int _RegisterClass(HINSTANCE hInstance);
	int _CreateWindow(HINSTANCE hInstance, int nCmdShow);
	int _SetWindowSize();
	int _SetWindowSizeFullScreen();

	//Initialize config file
	int _InitConfFile();

	//Window procedure
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//Menu event processing
	int _OnMenuOpenFile();
	int _OnMenuAddFile();
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
	int _OnMenuEnableEffect(MTEffectType type);
	int _OnMenuWindowSize();
	int _OnMenuFullScreen();
	int _OnMenuMenuBar();
	int _OnMenuOptionMIDIOUT();
	int _OnMenuOptionMIDIIN();
	int _OnMenuOptionGraphic();
	int _OnMenuOptionColor();
	int _OnMenuManual();
	int _OnMenuSelectSceneType(SceneType type);
	int _OnFilePathPosted();

	//Other event processing
	int _SequencerMsgProc();
	int _OnRecvSequencerMsg(unsigned long wParam, unsigned long lParam);
	int _OnMouseButtonDown(UINT button, WPARAM wParam, LPARAM lParam);
	int _OnMouseMove(UINT button, WPARAM wParam, LPARAM lParam);
	int _OnKeyDown(WPARAM wParam, LPARAM lParam);
	int _OnDropFiles(WPARAM wParam, LPARAM lParam);

	int _SelectMIDIFile(WCHAR* pFilePath,  unsigned long bufSize, bool* pIsSelected);
	int _SelectFolder(WCHAR* pFolderPath, unsigned long bufSize, bool* pIsSelected);
	int _LoadMIDIFile(const WCHAR* pFilePath);
	static void _OnLoadingProgress(unsigned long current, unsigned long total, const char* message, void* userData);
	int _AddMIDIFile(const WCHAR* pFilePath);
	void _UpdateWindowTitle(const WCHAR* pFileName);
	void _UpdateFPS();
	int _SetPortDev(SMSequencer* pSequencer);
	int _SetMonitorPortDev(SMLiveMonitor* pLiveMonitor, IMTScene11* pScene);
	int _ChangeWindowSize();
	int _ChangePlayStatus(PlayStatus status);
	int _ChangeMenuStyle();
	int _CreateScene(SceneType type, SMSeqData* pSeqData,
	                 const MTLoadProgressContext* pProgress = NULL);
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

};

