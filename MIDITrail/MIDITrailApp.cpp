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

#include "StdAfx.h"
#include <shlobj.h>
#include "shellapi.h"
#include "shlwapi.h"
#include <ShellScalingApi.h>
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MIDITrailApp.h"
#include "MTLogManager.h"
#include "RDDiagManager.h"
#include "RDFormatProfiles.h"
#include <spdlog/spdlog.h>
#include "MTSceneTitle11.h"
#include "MTScenePianoRoll3D11.h"
#include "MTScenePianoRoll3DLive11.h"
#include "MTScenePianoRollRain11.h"
#include "MTScenePianoRollRainLive11.h"
#include "MTScenePianoRollRing11.h"
#include "MTScenePianoRollRingLive11.h"
#include "DXPrimitive11.h"
#include "MTNoteInstancedBase11.h"
#include "MTNoteAABBInstanced11.h"
#include "MTNoteCylindricalInstanced11.h"
#include "MIDITrailVersion.h"
#include <ShObjIdl.h>
#include <mbctype.h>
#include <dwmapi.h>
#include "MTLoadingDefs.h"

using namespace YNBaseLib;

static const char* _TransportTypeToString(SMIDILib::SMTransportType type)
{
	switch (type) {
	case SMIDILib::SMTransportType::KDMAPIMod: return "Activated, KDMAPI(Mod)";
	case SMIDILib::SMTransportType::KDMAPI:    return "Activated, KDMAPI(Std)";
	case SMIDILib::SMTransportType::WinMM:     return "Activated, WinMM";
	case SMIDILib::SMTransportType::None:
	default:                                   return "Deactivated";
	}
}

//******************************************************************************
// Window procedure control parameter setup
//******************************************************************************
MIDITrailApp* MIDITrailApp::m_pThis = NULL;

//******************************************************************************
// Constructor
//******************************************************************************
MIDITrailApp::MIDITrailApp(void)
{
	m_pThis = this;
	m_hInstance = NULL;
	m_hAppMutex = NULL;
	m_hMailSlot = NULL;
	m_isExitApp = false;

	//Window-related
	m_hWnd = NULL;
	m_Accel = NULL;
	m_Title[0] = L'\0';
	m_WndClassName[0] = L'\0';
	m_isFullScreen = false;
	m_isEnableMenuBar = true;
	m_hMenu = NULL;

	//Rendering-related
	m_pScene = NULL;
	m_MultiSampleType = 0;

	//FPS display-related
	m_PrevTime = 0;
	m_FPSCount = 0;
	m_PrevFrameQPC.QuadPart = 0;

	//MIDI control-related
	m_MIDIINDevName[0] = _T('\0');

	//Playback status
	m_PlayStatus = NoData;
	m_isRepeat = false;
	m_isFolderPlayback = true;
	m_isRewind = false;
	m_isLoading = false;
	m_isOpenFileAfterStop = false;
	m_pendingPlaybackStartLog = false;
	ZeroMemory(&m_SequencerLastMsg, sizeof(MTSequencerLastMsg));
	m_PlaySpeedRatio = 100;

	//Display state
	m_isEnablePianoKeyboard = true;
	m_isEnableRipple = true;
	m_isEnablePitchBend = true;
	m_isEnableStars = true;
	m_isEnableCounter = true;
	m_isEnableFileName = false;
	m_isEnableBackgroundImage = true;
	m_isEnableGridLine = true;
	m_isEnableTimeIndicator = true;
	m_isEnableDiagOverlay = false;

	//Scene type
	m_SceneType = Title;
	m_SelectedSceneType = PianoRoll3D;

	//Auto-save viewpoint
	m_isAutoSaveViewpoint = false;

	//Player control
	m_AllowMultipleInstances = 0;
	m_AutoPlaybackAfterOpenFile = 0;

	//Rewind/skip control
	m_SkipBackTimeSpanInMsec = 10000;
	m_SkipForwardTimeSpanInMsec = 10000;

	//Playback speed control
	m_SpeedStepInPercent = 1;
	m_MaxSpeedInPercent = 400;

	//Playback control
	m_DelayBetweenSongsInMsec = 0;

	//File path to open next
	m_NextFilePath[0] = L'\0';

	//Viewpoint number for gamepad
	m_GamePadViewPointNo = 0;
}

//******************************************************************************
// Destructor
//******************************************************************************
MIDITrailApp::~MIDITrailApp(void)
{
	Terminate();
}

//******************************************************************************
// Initialize
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

	//Parse command line early (needed for --log-level before logger init)
	m_CmdLineParser.Initialize();

	//Initialize logging framework (before all other initialization)
	{
		const WCHAR* pLogLevel = nullptr;
		if (m_CmdLineParser.GetSwitch(CMDSW_LOG_LEVEL) == CMDSW_ON) {
			pLogLevel = m_CmdLineParser.GetLogLevel();
		}
		MTLogManager::Initialize(pLogLevel);
	}

	//String initialization
	LoadStringW(hInstance, IDS_APP_TITLE, m_TitleBase, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MIDITRAIL, m_WndClassName, MAX_LOADSTRING);

	const WCHAR* pVersion = NULL;

	//Version string
#ifdef _WIN64
	//64bit
	pVersion = MIDITRAIL_VERSION_STRING_X64;
#else
	//32bit
	pVersion = MIDITRAIL_VERSION_STRING_X86;
#endif
	wcscat_s(m_TitleBase, MAX_LOADSTRING, L" ");
	wcscat_s(m_TitleBase, MAX_LOADSTRING, (LPCWSTR)pVersion);
	wcscpy_s(m_Title, MAX_LOADSTRING, m_TitleBase);

	//Initialize config file
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//Load graphics settings
	result = _LoadGraphicConf();
	if (result != 0) goto EXIT;

	//Load player settings
	result = _LoadPlayerConf();
	if (result != 0) goto EXIT;

	//Duplicate launch check
	result = _CheckMultipleInstances(&m_isExitApp);
	if (result != 0) goto EXIT;

	//If duplicate launch is being prevented
	if (m_isExitApp) {
		_PostFilePathToFirstMIDITrail();
		goto EXIT;
	}

	//Build mailslot
	result = _CreateMailSlot();
	if (result != 0) goto EXIT;

	//Initialize message queue
	result = m_MsgQueue.Initialize(10000);
	if (result != 0) goto EXIT;

	//Register window class
	result = _RegisterClass(hInstance);
	if (result != 0) goto EXIT;

	//Create main window
	result = _CreateWindow(hInstance, nCmdShow);
	if (result != 0) goto EXIT;

	//Load accelerator table
	m_Accel = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDC_MIDITRAIL));
	if (m_Accel == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hInstance);
		goto EXIT;
	}

	//Change playback status
	result = _ChangePlayStatus(NoData);
	if (result != 0) goto EXIT;

	//Initialize renderer
	result = m_Renderer.Initialize(m_hWnd, m_MultiSampleType);
	if (result != 0) goto EXIT;

	//Initialize shared pipeline
	result = DXPrimitive11::InitPipeline(m_Renderer.GetDevice());
	if (result != 0) goto EXIT;

	//Initialize runtime diagnostics
	result = RDDiagManager::Initialize(m_Renderer.GetDevice(), m_Renderer.GetContext());
	if (result != 0) goto EXIT;
	RDDiagManager::SetLogIntervalMs(MTLogManager::GetRuntimeLogIntervalMs());

	//Set version and identity metrics
	RDDiagManager::SetString(RDMetricId::AppIdVersion, MIDITRAIL_VER_DISPLAY);
	RDDiagManager::SetString(RDMetricId::AppIdModVersion, MIDITRAIL_MOD_STRING_SHORT);
#ifdef _DEBUG
	RDDiagManager::SetString(RDMetricId::AppIdBuildConfig, "Debug");
#else
	RDDiagManager::SetString(RDMetricId::AppIdBuildConfig, "Release");
#endif
	{
		D3D_FEATURE_LEVEL fl = m_Renderer.GetDevice()->GetFeatureLevel();
		char flStr[32];
		snprintf(flStr, sizeof(flStr), "%d.%d", (fl >> 12) & 0xF, (fl >> 8) & 0xF);
		RDDiagManager::SetString(RDMetricId::AppIdDxFeatureLevel, flStr);
	}

	//Create scene object
	m_SceneType = Title;
	result = _CreateScene(m_SceneType, &m_SeqData);
	if (result != 0) goto EXIT;

	//Load scene type
	result = _LoadSceneType();
	if (result != 0) goto EXIT;

	//Load scene settings
	result = _LoadSceneConf();
	if (result != 0) goto EXIT;

	//Load display effect selection state
	result = _LoadEffectStatus();
	if (result != 0) goto EXIT;

	//Update menu selection mark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//Initialize RCP file converter
	result = m_RcpConv.Initialize();
	if (result != 0) goto EXIT;

	//Check renderer
	result = _CheckRenderer();
	if (result != 0) goto EXIT;

	//Auto-configure MIDI OUT
	result = _AutoConfigMIDIOUT();
	if (result != 0) goto EXIT;

	//Update window title
	_UpdateWindowTitle(NULL);

	//Parse and execute command line
	result = _ParseCmdLine();
	if (result != 0) goto EXIT;

	//Start timer
	result = _StartTimer();
	if (result != 0) goto EXIT;

	//Gamepad control: user index fixed at 0
	result = m_GamePadCtrl.Initialize(0);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Terminate
//******************************************************************************
int MIDITrailApp::Terminate()
{
	int result = 0;

	_StopTimer();

	RDDiagManager::Terminate();

	MTLogManager::Terminate();

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
// Run
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

	//Message loop
	while (TRUE) {
		isExist = PeekMessage(
						&msg,		//Retrieved message
						NULL,		//Source window handle
						0,			//Minimum message value to retrieve
						0,			//Maximum message value to retrieve
						PM_REMOVE	//Message handling: remove from queue
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
			//Sequencer message processing
			result = _SequencerMsgProc();
			if (result != 0) {
				YN_SHOW_ERR(m_hWnd);
			}

			//Gamepad operation processing
			result = _GamePadProc();
			if (result != 0) {
				YN_SHOW_ERR(m_hWnd);
			}

			//Only draw while the window is in a visible state
			GetWindowPlacement(m_hWnd, &wndpl);
			if ((wndpl.showCmd != SW_HIDE) &&
				(wndpl.showCmd != SW_MINIMIZE) &&
				(wndpl.showCmd != SW_SHOWMINIMIZED) &&
				(wndpl.showCmd != SW_SHOWMINNOACTIVE)) {

				LARGE_INTEGER frameNow, updateEnd;
				LARGE_INTEGER freq;
				QueryPerformanceFrequency(&freq);
				double toMs = 1000.0 / static_cast<double>(freq.QuadPart);

				QueryPerformanceCounter(&frameNow);
				if (m_PrevFrameQPC.QuadPart != 0) {
					double frameTimeMs = static_cast<double>(
						frameNow.QuadPart - m_PrevFrameQPC.QuadPart) * toMs;
					RDDiagManager::SetFloat(RDMetricId::RenderFrameTimeMs, frameTimeMs);
				}
				m_PrevFrameQPC = frameNow;

				//Update scene (camera input, playback position, component state)
				result = m_pScene->Update();
				if (result != 0) {
					YN_SHOW_ERR(m_hWnd);
					PostMessage(m_hWnd, WM_DESTROY, 0, 0);
				}

				QueryPerformanceCounter(&updateEnd);
				RDDiagManager::SetFloat(RDMetricId::RenderSceneUpdateTimeMs,
					static_cast<double>(updateEnd.QuadPart - frameNow.QuadPart) * toMs);

				//Draw
				result = m_Renderer.RenderScene(m_pScene, m_pScene->GetCamera());
				if (result != 0) {
					if (result == DXRENDERER11_ERR_DEVICE_LOST) {
						//Device lost
						//As a temporary workaround, recreate the scene
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

				//Update polyphony from sequencer (atomic read from timer thread)
				if (m_PlayStatus == Play) {
					int32_t polyphony = m_Sequencer.GetPolyphonyCount();
					RDDiagManager::SetInt(RDMetricId::PlaybackPolyphony, static_cast<int64_t>(polyphony));
					int64_t peak = RDDiagManager::GetInt(RDMetricId::PlaybackPolyphonyPeak);
					if (static_cast<int64_t>(polyphony) > peak) {
						RDDiagManager::SetInt(RDMetricId::PlaybackPolyphonyPeak, static_cast<int64_t>(polyphony));
					}
				}

				//Update runtime diagnostics (after scene update/draw/present)
				RDDiagManager::Update();

				// Deferred playback-start log: wait until synth audio data is valid
				if (m_pendingPlaybackStartLog) {
					if (RDDiagManager::IsSet(RDMetricId::SynthAudioFrequency)) {
						RDDiagManager::LogEvent(RDFormatProfile::PlaybackStart,
							RDFormatProfile::PlaybackStartCount, "playback-start");
						m_pendingPlaybackStartLog = false;
					}
				}
			}
		}
    }

EXIT:;
	//If the function receives WM_QUIT and terminates normally,
	//return the exit code stored in wParam
	//If it terminates before entering the message loop, return 0
	return quitCode;
}

//******************************************************************************
// Register window class
//******************************************************************************
int MIDITrailApp::_RegisterClass(
		HINSTANCE hInstance
	)
{
	int result = 0;
	ATOM aresult = 0;
	WNDCLASSEXW wcex;

	wcex.cbSize			= sizeof(WNDCLASSEXW);				//Structure size
	wcex.style			= CS_HREDRAW | CS_VREDRAW;			//Class style
	wcex.lpfnWndProc	= _WndProc;							//Window procedure
	wcex.cbClsExtra		= 0;								//Extra class info size
	wcex.cbWndExtra		= 0;								//Extra window info size
	wcex.hInstance		= hInstance;						//Application instance handle
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIDITRAIL));
															//Icon resource handle
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);		//Cursor resource handle
	wcex.hbrBackground  = CreateSolidBrush(RGB(0, 0, 0));	//Background brush handle: black
	wcex.lpszMenuName	= MAKEINTRESOURCEW(IDC_MIDITRAIL);	//Menu resource name
	wcex.lpszClassName	= m_WndClassName;					//Window class name
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
				 											//Small icon resource handle

	//Specifies redraw of the invalid window region on move/resize
	// CS_HREDRAW Redraw when the client area width changes
	// CS_VREDRAW Redraw when the client area height changes

	aresult = RegisterClassExW(&wcex);
	if (aresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Create main window
//******************************************************************************
int MIDITrailApp::_CreateWindow(
		HINSTANCE hInstance,
		int nCmdShow
	)
{
	int result = 0;

	m_hWnd = CreateWindowW(
				m_WndClassName,			//Window class name
				m_Title,				//Window name
				MIDITRAIL_WINDOW_STYLE,	//Window style
				CW_USEDEFAULT,			//Window horizontal position: default
				0,						//Window vertical position
				CW_USEDEFAULT,			//Window width: default
				0,						//Window height
				NULL,					//Parent or owner window handle
				NULL,					//Menu handle or child window ID
				hInstance,				//Application instance handle
				NULL					//Window creation data
			);
	if (m_hWnd == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//Get the handle immediately after window creation, for menu bar visibility toggling
	m_hMenu = GetMenu(m_hWnd);

	//Show window
	//The window must be shown before the shadow information can be obtained during the resize process
	ShowWindow(m_hWnd, nCmdShow);

	//Apply user-configured window size
	result = _SetWindowSize();
	if (result != 0) goto EXIT;

	//Suppress WM_PAINT calls
	ValidateRect(m_hWnd, 0);

	UpdateWindow(m_hWnd);

EXIT:;
	return result;
}

//******************************************************************************
// Window resize
//******************************************************************************
int MIDITrailApp::_SetWindowSize()
{
	int result = 0;
	BOOL bresult = FALSE;
	HRESULT hresult = 0;
	int width = 0;
	int height = 0;
	RECT rect_client;
	RECT rect_excludeShadow;
	RECT rect_includeShadow;
	RECT rect_shadow;
	int ww, wh, cw, ch, framew, frameh;
	int applyToViewArea = 0;
	LONG apiresult = 0;

	if (m_isFullScreen) {
		result = _SetWindowSizeFullScreen();
		goto EXIT;
	}

	//Get user-selected window size
	result = m_ViewConf.SetCurSection(_T("WindowSize"));
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Width"), &width, 0);
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Height"), &height, 0);
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("ApplyToViewArea"), &applyToViewArea, 0);
	if (result != 0) goto EXIT;
	
	//Set window style
	apiresult = SetWindowLong(m_hWnd, GWL_STYLE, MIDITRAIL_WINDOW_STYLE);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

	//Show menu bar
	if (m_isEnableMenuBar) {
		result = _ShowMenu();
		if (result != 0) goto EXIT;
	}
	else {
		result = _HideMenu();
		if (result != 0) goto EXIT;
	}

	//Window size on first launch
	if ((width <= 0) || (height <= 0)) {
		width = 800;
		height = 600;
	}

	//Window size (excluding shadow)
	hresult = DwmGetWindowAttribute(
					m_hWnd,							//Window handle
					DWMWA_EXTENDED_FRAME_BOUNDS,	//Flag indicating the value to retrieve: extended frame bounds
					&rect_excludeShadow, 			//Destination for the value
					sizeof(RECT)					//Size of the value
				);
	if (hresult != S_OK) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}
	ww = rect_excludeShadow.right  - rect_excludeShadow.left;
	wh = rect_excludeShadow.bottom - rect_excludeShadow.top;

	//Client area size
	GetClientRect(m_hWnd, &rect_client);
	cw = rect_client.right  - rect_client.left;
	ch = rect_client.bottom - rect_client.top;

	//Frame size
	framew = ww - cw;
	frameh = wh - ch;

	//When applying the specified size to the drawing area
	if (applyToViewArea != 0) {
		width = width + framew;
		height = height + frameh;
	}

	//Window size (including shadow)
	//  On Windows Vista and later, GetWindowRect returns a size that includes the shadow
	GetWindowRect(m_hWnd, &rect_includeShadow);

	//Shadow deviation
	rect_shadow.left   = rect_includeShadow.left   - rect_excludeShadow.left;	//e.g.: -7
	rect_shadow.right  = rect_includeShadow.right  - rect_excludeShadow.right;	//e.g.: +7
	rect_shadow.top    = rect_includeShadow.top    - rect_excludeShadow.top;	//e.g.: -7
	rect_shadow.bottom = rect_includeShadow.bottom - rect_excludeShadow.bottom;	//e.g.: +7

	//Reflect the shadow in the window size passed to SetWindowPos
	width  = width  + (rect_shadow.right  - rect_shadow.left);
	height = height + (rect_shadow.bottom - rect_shadow.top);

	//Window resize
	bresult = SetWindowPos(
					m_hWnd,			//Window handle
					HWND_TOP,		//Z order: top
					0,				//Horizontal position
					0,				//Vertical position
					width,			//Width
					height,			//Height
					SWP_NOMOVE | SWP_FRAMECHANGED | SWP_SHOWWINDOW	//Window position flags
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Window resize: fullscreen
//******************************************************************************
int MIDITrailApp::_SetWindowSizeFullScreen()
{
	int result = 0;
	BOOL bresult = FALSE;
	LONG apiresult = 0;
	POINT mouseCursorPoint;
	HMONITOR hMonitor = NULL;
	MONITORINFOEX monitorInfo;
	DEVMODE dm = {};
	UINT dpiX = 96, dpiY = 96;
	int physicalWidth = 0;
	int physicalHeight = 0;
	int physicalLeft = 0;
	int physicalTop = 0;

	//Get mouse cursor position
	bresult = GetCursorPos(&mouseCursorPoint);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Select the monitor corresponding to the mouse cursor position
	hMonitor = MonitorFromPoint(mouseCursorPoint, MONITOR_DEFAULTTONEAREST);

	//Get monitor info
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	bresult = GetMonitorInfo(hMonitor, &monitorInfo);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hMonitor);
		goto EXIT;
	}

	//Get physical resolution (EnumDisplaySettings always returns physical pixels)
	dm.dmSize = sizeof(DEVMODE);
	if (!EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	physicalWidth  = dm.dmPelsWidth;
	physicalHeight = dm.dmPelsHeight;

	//Get the DPI scale and convert logical coordinates to physical coordinates (PMv2: SetWindowPos uses physical pixels)
	GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
	physicalLeft = (int)(monitorInfo.rcMonitor.left * (dpiX / 96.0f));
	physicalTop  = (int)(monitorInfo.rcMonitor.top  * (dpiY / 96.0f));

	//Set window style
	apiresult = SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

	//Hide menu bar
	result = _HideMenu();
	if (result != 0) goto EXIT;

	//Window resize (physical pixels)
	bresult = SetWindowPos(
					m_hWnd,				//Window handle
					HWND_TOP,			//Z order: top
					physicalLeft,		//Horizontal position (physical pixels)
					physicalTop,		//Vertical position (physical pixels)
					physicalWidth,		//Width (physical pixels)
					physicalHeight,		//Height (physical pixels)
					SWP_FRAMECHANGED | SWP_SHOWWINDOW	//Window position flags
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Initialize config file
//******************************************************************************
int MIDITrailApp::_InitConfFile()
{
	int result = 0;
	TCHAR userConfDirPath[_MAX_PATH] = {_T('\0')};
	TCHAR viewConfPath[_MAX_PATH] = {_T('\0')};
	TCHAR midiOutConfPath[_MAX_PATH] = {_T('\0')};
	TCHAR graphicConfPath[_MAX_PATH] = {_T('\0')};

	//Build the user config file storage directory path
	result = YNPathUtil::GetAppDataDirPath(userConfDirPath, _MAX_PATH);
	if (result != 0) goto EXIT;
	_tcscat_s(userConfDirPath, _MAX_PATH, MT_USER_CONFFILE_DIR);

	//Create the user config file storage directory
	{
		int shr = SHCreateDirectoryEx(NULL, userConfDirPath, NULL);
		if (shr != ERROR_SUCCESS && shr != ERROR_ALREADY_EXISTS && shr != ERROR_FILE_EXISTS) {
			result = YN_SET_ERR("Windows API error.", shr, 0);
			goto EXIT;
		}
	}

	//View settings config file
	_tcscat_s(viewConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(viewConfPath, _MAX_PATH, MT_USER_CONFFILE_VIEW);
	result = m_ViewConf.Initialize(viewConfPath);
	if (result != 0) goto EXIT;

	//MIDI settings config file
	_tcscat_s(midiOutConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(midiOutConfPath, _MAX_PATH, MT_USER_CONFFILE_MIDI);
	result = m_MIDIConf.Initialize(midiOutConfPath);
	if (result != 0) goto EXIT;

	//Graphics settings config file
	_tcscat_s(graphicConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(graphicConfPath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);
	result = m_GraphicConf.Initialize(graphicConfPath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Main window: window procedure
//******************************************************************************
LRESULT CALLBACK MIDITrailApp::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// Main window: window procedure: implementation
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
					//Open file
					result = _OnMenuOpenFile();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPEN_FOLDER:
					//Open folder
					result = _OnMenuOpenFolder();
					if (result != 0) goto EXIT;
					break;
				case IDM_PREVIOUS_FILE:
					//Previous file
					result = _OnMenuPreviousFile();
					if (result != 0) goto EXIT;
					break;
				case IDM_NEXT_FILE:
					//Next file
					result = _OnMenuNextFile();
					if (result != 0) goto EXIT;
					break;
				case IDM_ADD_FILE:
					//Add file
					result = _OnMenuAddFile();
					if (result != 0) goto EXIT;
					break;
				case IDM_EXIT:
					//Quit
					DestroyWindow(hWnd);
					break;
				case IDM_PLAY:
					//Start/pause/resume playback
					result = _OnMenuPlay();
					if (result != 0) goto EXIT;
					break;
				case IDM_STOP:
					//Stop playback
					result = _OnMenuStop();
					if (result != 0) goto EXIT;
					break;
				case IDM_REPEAT:
					//Repeat
					result = _OnMenuRepeat();
					if (result != 0) goto EXIT;
					break;
				case IDM_FOLDER_PLAYBACK:
					//Folder playback
					result = _OnMenuFolderPlayback();
					if (result != 0) goto EXIT;
					break;
				case IDM_SKIP_BACK:
					//Playback skip back
					result = _OnMenuSkipBack();
					if (result != 0) goto EXIT;
					break;
				case IDM_SKIP_FORWARD:
					//Playback skip forward
					result = _OnMenuSkipForward();
					if (result != 0) goto EXIT;
					break;
				case IDM_PLAY_SPEED_DOWN:
					//Playback speed down
					result = _OnMenuPlaySpeedDown();
					if (result != 0) goto EXIT;
					break;
				case IDM_PLAY_SPEED_UP:
					//Playback speed up
					result = _OnMenuPlaySpeedUp();
					if (result != 0) goto EXIT;
					break;
				case IDM_START_MONITORING:
					//Start monitoring
					result = _OnMenuStartMonitoring();
					if (result != 0) goto EXIT;
					break;
				case IDM_STOP_MONITORING:
					//Stop monitoring
					result = _OnMenuStopMonitoring();
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_3DPIANOROLL:
					//Change view: 3D piano roll
					result = _OnMenuSelectSceneType(PianoRoll3D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_2DPIANOROLL:
					//Change view: 2D piano roll
					result = _OnMenuSelectSceneType(PianoRoll2D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRAIN:
					//Change view: piano roll rain
					result = _OnMenuSelectSceneType(PianoRollRain);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRAIN2D:
					//Change view: piano roll rain 2D
					result = _OnMenuSelectSceneType(PianoRollRain2D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRING:
					//Change view: piano roll ring
					result = _OnMenuSelectSceneType(PianoRollRing);
					if (result != 0) goto EXIT;
					break;
				//TAG: add scene
				case IDM_ENABLE_PIANOKEYBOARD:
					//Display effect: piano keyboard
					result = _OnMenuEnableEffect(MTEffectPianoKeyboard);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_RIPPLE:
					//Display effect: ripple
					result = _OnMenuEnableEffect(MTEffectRipple);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_PITCHBEND:
					//Display effect: pitch bend
					result = _OnMenuEnableEffect(MTEffectPitchBend);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_STARS:
					//Display effect: stars
					result = _OnMenuEnableEffect(MTEffectStars);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_COUNTER:
					//Display effect: counter
					result = _OnMenuEnableEffect(MTEffectCounter);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_BACKGROUNDIMAGE:
					//Display effect: background image
					result = _OnMenuEnableEffect(MTEffectBackgroundImage);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_GRIDLINE:
					//Display effect: grid lines
					result = _OnMenuEnableEffect(MTEffectGridBox);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_TIMEINDICATOR:
					//Display effect: time indicator
					result = _OnMenuEnableEffect(MTEffectTimeIndicator);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_DIAGOVERLAY:
					//Display effect: diagnostic overlay
					result = _OnMenuEnableEffect(MTEffectDiagOverlay);
					if (result != 0) goto EXIT;
					break;
				//Auto-save viewpoint and save viewpoint have been removed
				//case IDM_AUTO_SAVE_VIEWPOINT:
				//	//Auto-save viewpoint
				//	result = _OnMenuAutoSaveViewpoint();
				//	if (result != 0) goto EXIT;
				//	break;
				//case IDM_SAVE_VIEWPOINT:
				//	//Save viewpoint
				//	result = _OnMenuSaveViewpoint();
				//	if (result != 0) goto EXIT;
				//	break;
				case IDM_RESET_VIEWPOINT:
					//Move to static viewpoint 1 (reset viewpoint)
					result = _OnMenuResetViewpoint();
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEWPOINT2:
					//Move to static viewpoint 2
					result = _OnMenuViewpoint(2);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEWPOINT3:
					//Move to static viewpoint 3
					result = _OnMenuViewpoint(3);
					if (result != 0) goto EXIT;
					break;
				case IDM_MYVIEWPOINT1:
					//Move to my viewpoint 1
					result = _OnMenuMyViewpoint(1);
					if (result != 0) goto EXIT;
					break;
				case IDM_MYVIEWPOINT2:
					//Move to my viewpoint 2
					result = _OnMenuMyViewpoint(2);
					if (result != 0) goto EXIT;
					break;
				case IDM_MYVIEWPOINT3:
					//Move to my viewpoint 3
					result = _OnMenuMyViewpoint(3);
					if (result != 0) goto EXIT;
					break;
				case IDM_SAVE_MYVIEWPOINT1:
					//Save my viewpoint 1
					result = _OnMenuSaveMyViewpoint(1);
					if (result != 0) goto EXIT;
					break;
				case IDM_SAVE_MYVIEWPOINT2:
					//Save my viewpoint 2
					result = _OnMenuSaveMyViewpoint(2);
					if (result != 0) goto EXIT;
					break;
				case IDM_SAVE_MYVIEWPOINT3:
					//Save my viewpoint 3
					result = _OnMenuSaveMyViewpoint(3);
					if (result != 0) goto EXIT;
					break;
				case IDM_WINDOWSIZE:
					//Window size settings
					result = _OnMenuWindowSize();
					if (result != 0) goto EXIT;
					break;
				case IDM_FULLSCREEN:
					//Fullscreen
					result = _OnMenuFullScreen();
					if (result != 0) goto EXIT;
					break;
				case IDM_MENUBAR:
					//Menu bar
					result = _OnMenuMenuBar();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_MIDIOUT:
					//MIDI output device settings
					result = _OnMenuOptionMIDIOUT();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_MIDIIN:
					//MIDI input device settings
					result = _OnMenuOptionMIDIIN();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_GRAPHIC:
					//Graphics settings
					result = _OnMenuOptionGraphic();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_COLOR:
					//Set color
					result = _OnMenuOptionColor();
					if (result != 0) goto EXIT;
					break;
				case IDM_HOWTOVIEW:
					//Show how-to-view dialog
					m_HowToViewDlg.Show(m_hWnd);
					break;
				case IDM_MANUAL:
					//Show manual
					result = _OnMenuManual();
					if (result != 0) goto EXIT;
					break;
				case IDM_ABOUT:
					//Show about dialog
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
			//Key press message
			result = _OnKeyDown(wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			//Mouse button press message
			result = _OnMouseButtonDown(message, wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_MOUSEMOVE:
			result = _OnMouseMove(message, wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_DROPFILES:
			//File drop
			result = _OnDropFiles(wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_TIMER:
			//Timer
			result = _OnTimer(wParam);
			if (result != 0) goto EXIT;
			break;
		case WM_DESTROY:
			//Destroy
			result = _OnDestroy();
			//Ignore the return value
			PostQuitMessage(0);
			break;
		case WM_FILEPATH_POSTED:
			//File path post notification
			result = _OnFilePathPosted();
			if (result != 0) goto EXIT;
			break;
		case WM_SIZE:
		{
			//Window resize
			if (wParam == SIZE_MAXIMIZED) {
				//Maximize: fullscreen
				result = _OnMenuFullScreen();
				if (result != 0) goto EXIT;
			}
			else if (wParam != SIZE_MINIMIZED) {
				//Normal resize (including frame finalization after a DPI change)
				result = m_Renderer.OnResize();
				if (result != 0) goto EXIT;
				if (m_pScene != NULL) {
					m_pScene->OnWindowResize();
				}
			}
			break;
		}
		case WM_GETDPISCALEDSIZE:
		{
			//The OS queries for the new size before the DPI change (PMv2)
			//Return the current window size to preserve the physical pixel count
			SIZE* pNewSize = reinterpret_cast<SIZE*>(lParam);
			if (pNewSize != NULL && !m_isFullScreen) {
				RECT windowRect;
				GetWindowRect(m_hWnd, &windowRect);
				pNewSize->cx = windowRect.right  - windowRect.left;
				pNewSize->cy = windowRect.bottom - windowRect.top;
				lresult = TRUE;
			}
			break;
		}
		case WM_DPICHANGED:
		{
			//DPI change (when moving between monitors)
			//Apply suggestedRect as-is (standard PMv2 pattern)
			//OnResize/OnWindowResize run on WM_SIZE (after the frame size is finalized)
			RECT* pSuggested = reinterpret_cast<RECT*>(lParam);
			if (!m_isFullScreen && pSuggested != NULL) {
				SetWindowPos(m_hWnd, NULL,
					pSuggested->left,
					pSuggested->top,
					pSuggested->right  - pSuggested->left,
					pSuggested->bottom - pSuggested->top,
					SWP_NOZORDER | SWP_NOACTIVATE);
			}
			break;
		}
		case WM_SETTEXT:
			//Handle via the Unicode SetWindowTextW call
			lresult = DefWindowProcW(hWnd, message, wParam, lParam);
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
// Open file
//******************************************************************************
int MIDITrailApp::_OnMenuOpenFile()
{
	int result = 0;
	WCHAR filePath[_MAX_PATH] = { L'\0' };
	bool isSelected = false;

	////Do not allow opening a file during playback
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	//File open OK
	//}
	//else {
	//	//File open NG
	//	goto EXIT;
	//}

	//Allow opening a file even during playback

	//Show the file selection dialog
	result = _SelectMIDIFile(filePath, _MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//Processing when a file is selected
	if (isSelected) {
		//When a file is selected from the menu while in fullscreen
		//  the scene creation process references the client window size, so
		//  hide the menu that was temporarily shown
		if (m_isFullScreen) {
			_HideMenu();
		}

		//Destroy file list
		m_MIDIFileList.Clear();

		//Stop playback/monitoring and open file
		result = _StopPlaybackAndOpenFile(filePath);
		if (result != 0) goto EXIT;
	}

	//Update menu style
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Open folder
//******************************************************************************
int MIDITrailApp::_OnMenuOpenFolder()
{
	int result = 0;
	WCHAR folderPath[_MAX_PATH] = { L'\0' };
	bool isSelected = false;

	//Allow opening a folder even during playback

	//Show folderselectiondialog
	result = _SelectFolder(folderPath, _MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//Processing when a folder is selected
	if (isSelected) {
		//When a file is selected from the menu while in fullscreen
		//  the scene creation process references the client window size, so
		//  hide the menu that was temporarily shown
		if (m_isFullScreen) {
			_HideMenu();
		}

		//Stop playback/monitoring and open folder
		result = _StopPlaybackAndOpenFolder(folderPath);
		if (result != 0) goto EXIT;
	}

	//Update menu style
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Previous file
//******************************************************************************
int MIDITrailApp::_OnMenuPreviousFile()
{
	int result = 0;
	bool isExist = false;
	const WCHAR* pFilePath = NULL;

	//Do nothing if there is no file list
	if (m_MIDIFileList.GetFileCount() == 0) goto EXIT;

	//Select the previous file
	m_MIDIFileList.SelectPreviousFile(&isExist);

	//If the previous file exists
	if (isExist) {
		//Stop playback/monitoring and open file
		pFilePath = m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex());
		result = _StopPlaybackAndOpenFile(pFilePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Next file
//******************************************************************************
int MIDITrailApp::_OnMenuNextFile()
{
	int result = 0;
	bool isExist = false;
	const WCHAR* pFilePath = NULL;

	//Do nothing if there is no file list
	if (m_MIDIFileList.GetFileCount() == 0) goto EXIT;

	//Select the next file
	m_MIDIFileList.SelectNextFile(&isExist);

	//If the next file exists
	if (isExist) {
		//Stop playback/monitoring and open file
		pFilePath = m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex());
		result = _StopPlaybackAndOpenFile(pFilePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}


//******************************************************************************
// Add file
//******************************************************************************
int MIDITrailApp::_OnMenuAddFile()
{
	int result = 0;
	WCHAR filePath[_MAX_PATH] = { L'\0' };
	bool isSelected = false;

	//Do not allow opening a file during playback
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//File open OK
	}
	else {
		//File open NG
		goto EXIT;
	}

	//Show the file selection dialog
	result = _SelectMIDIFile(filePath, _MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//Processing when a file is selected
	if (isSelected) {
		//MIDI file loading process
		result = _AddMIDIFile(filePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}


//******************************************************************************
// Menu selection: play / pause / resume
//******************************************************************************
int MIDITrailApp::_OnMenuPlay()
{
	int result = 0;

	if (m_isLoading) goto EXIT;

	if (m_PlayStatus == Stop) {
		//Initialize sequencer
		result = m_Sequencer.Initialize(&m_MsgQueue);
		if (result != 0) goto EXIT;

		//Register port information with the sequencer
		result = _SetPortDev(&m_Sequencer);
		if (result != 0) goto EXIT;

		//Register sequence data with the sequencer
		result = m_Sequencer.SetSeqData(&m_SeqData);
		if (result != 0) goto EXIT;

		//Rewind
		if (m_isRewind) {
			m_isRewind = false;
			result = m_pScene->Rewind();
			if (result != 0) goto EXIT;
		}

		//Notify the scene that playback has started
		result = m_pScene->OnPlayStart();
		if (result != 0) goto EXIT;

		//Clear peak metrics for new playback session
		RDDiagManager::SetInt(RDMetricId::PlaybackNoteTrackingPeak, 0);
		RDDiagManager::SetInt(RDMetricId::PlaybackPolyphonyPeak, 0);

		//Clear the latest sequencer message
		ZeroMemory(&m_SequencerLastMsg, sizeof(MTSequencerLastMsg));

		//Playback speed
		m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);

		//Start playback
		result = m_Sequencer.Play();
		if (result != 0) goto EXIT;

		//Change playback status
		result = _ChangePlayStatus(Play);
		if (result != 0) goto EXIT;

		m_pendingPlaybackStartLog = true;
	}
	else if (m_PlayStatus == Play) {
		//Pause playback
		m_Sequencer.Pause();

		//Change playback status
		result = _ChangePlayStatus(Pause);
		if (result != 0) goto EXIT;
	}
	else if (m_PlayStatus == Pause) {
		//Resume playback
		result = m_Sequencer.Resume();
		if (result != 0) goto EXIT;

		//Change playback status
		result = _ChangePlayStatus(Play);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: stop
//******************************************************************************
int MIDITrailApp::_OnMenuStop()
{
	int result = 0;

	if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		m_Sequencer.Stop();
		//Treat as still playing until the playback status notification arrives
		//Do not change the playback status here

		//Rewind after it finishes
		m_isRewind = true;
	}

	return result;
}

//******************************************************************************
// Menu selection: repeat
//******************************************************************************
int MIDITrailApp::_OnMenuRepeat()
{
	int result = 0;

	//Toggle repeat
	if (m_isRepeat) {
		m_isRepeat = false;
	}
	else {
		m_isRepeat = true;
	}

	//Update menu selection mark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: folder playback
//******************************************************************************
int MIDITrailApp::_OnMenuFolderPlayback()
{
	int result = 0;

	//Toggle folderplayback
	if (m_isFolderPlayback) {
		m_isFolderPlayback = false;
	}
	else {
		m_isFolderPlayback = true;
	}

	//Update menu selection mark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: skip back
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
// Menu selection: skip forward
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
// Menu selection: speed down
//******************************************************************************
int MIDITrailApp::_OnMenuPlaySpeedDown()
{
	int result = 0;

	//Check playback status
	if ((m_PlayStatus == Stop) || (m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//Change OK
	}
	else {
		//Change NG
		goto EXIT;
	}

	//Decrease playback speed
	m_PlaySpeedRatio -= m_SpeedStepInPercent;

	//Limit
	if (m_PlaySpeedRatio < m_SpeedStepInPercent) {
		m_PlaySpeedRatio = m_SpeedStepInPercent;
	}

	//Set playbackspeed
	m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);
	spdlog::info("Play speed: {}%", m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: speed up
//******************************************************************************
int MIDITrailApp::_OnMenuPlaySpeedUp()
{
	int result = 0;

	//Check playback status
	if ((m_PlayStatus == Stop) || (m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//Change OK
	}
	else {
		//Change NG
		goto EXIT;
	}

	//Increase playback speed
	m_PlaySpeedRatio += m_SpeedStepInPercent;

	//Limit at 400%
	if (m_PlaySpeedRatio > m_MaxSpeedInPercent) {
		m_PlaySpeedRatio = m_MaxSpeedInPercent;
	}

	//Set playbackspeed
	m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);
	spdlog::info("Play speed: {}%", m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: start live monitor
//******************************************************************************
int MIDITrailApp::_OnMenuStartMonitoring()
{
	int result = 0;
	spdlog::info("Live monitor: starting");

	//Check playback status
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//Monitor start OK
	}
	else {
		//Monitor start NG
		goto EXIT;
	}

	//Initialize sequencer
	//  The sequencer does not close the device when playback ends, so
	//  close it by initializing it
	result = m_Sequencer.Initialize(&m_MsgQueue);
	if (result != 0) goto EXIT;

	//Create the scene for the live monitor
	if (m_PlayStatus != MonitorOFF) {
		//Save viewpoint
		if (m_isAutoSaveViewpoint) {
			result = _OnMenuSaveViewpoint();
			if (result != 0) goto EXIT;
		}
		
		//Scene type
		m_SceneType = m_SelectedSceneType;
		
		//Create scene
		result = _CreateScene(m_SceneType, NULL);
		if (result != 0) goto EXIT;
	}
	
	//Initialize live monitor
	result = m_LiveMonitor.Initialize(&m_MsgQueue);
	if (result != 0) goto EXIT;
	result = _SetMonitorPortDev(&m_LiveMonitor, m_pScene);
	if (result != 0) goto EXIT;

	//Notify the scene that playback (live monitor) has started
	result = m_pScene->OnPlayStart();
	if (result != 0) goto EXIT;

	//Start live monitor
	result = m_LiveMonitor.Start();
	if (result != 0) goto EXIT;
	
	//Change playback status
	result = _ChangePlayStatus(MonitorON);
	if (result != 0) goto EXIT;

	//Update window title
	_UpdateWindowTitle(NULL);

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: stop live monitor
//******************************************************************************
int MIDITrailApp::_OnMenuStopMonitoring()
{
	int result = 0;
	spdlog::info("Live monitor: stopping");

	//Check playback status
	if (m_PlayStatus == MonitorON) {
		//Monitor start OK
	}
	else {
		//Monitor start NG
		goto EXIT;
	}

	//Stop live monitor
	result = m_LiveMonitor.Stop();
	if (result != 0) goto EXIT;

	//Change playback status
	result = _ChangePlayStatus(MonitorOFF);
	if (result != 0) goto EXIT;

	//Notify the scene that playback has ended
	if (m_pScene != NULL) {
		result = m_pScene->OnPlayEnd();
		if (result != 0) goto EXIT;
	}

	//Clear Live metrics (Monitor OFF)
	RDDiagManager::SetInt(RDMetricId::PlaybackPolyphony, 0);
	RDDiagManager::SetInt(RDMetricId::PlaybackPolyphonyPeak, 0);

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: scene type
//******************************************************************************
int MIDITrailApp::_OnMenuSelectSceneType(
		MIDITrailApp::SceneType type
	)
{
	int result = 0;
	const char* sceneNames[] = { "Title", "PianoRoll3D", "PianoRoll2D", "PianoRollRain",
		"PianoRollRain2D", "PianoRollRing", "PianoRoll3DLive", "PianoRoll2DLive",
		"PianoRollRainLive", "PianoRollRain2DLive", "PianoRollRingLive" };
	spdlog::info("Scene type: {}", sceneNames[type]);

	//Check playback status
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//Scene type selection OK
	}
	else {
		//Scene type selection NG
		goto EXIT;
	}

	//Save
	m_SelectedSceneType = type;
	result = _SaveSceneType();
	if (result != 0) goto EXIT;

	//Update menu selection mark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//Rebuild the scene when stopped
	if ((m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//Save viewpoint
		if (m_isAutoSaveViewpoint) {
			result = _OnMenuSaveViewpoint();
			if (result != 0) goto EXIT;
		}

		m_SceneType = m_SelectedSceneType;
		if (m_PlayStatus == Stop) {
			m_isLoading = true;
			m_LoadingScreen.Create(m_hWnd, &m_Renderer);
			MTLoadProgressContext topCtx(&_OnLoadingProgress, this);
			MTProgressBand fullBand = { &topCtx, 0.0f, 1.0f };
			MTLoadProgressContext buildCtx = fullBand.ToContext();
			result = _CreateScene(m_SceneType, &m_SeqData, &buildCtx);
			m_LoadingScreen.Update(1.0f, "Complete");
			m_LoadingScreen.Release();
			m_isLoading = false;
			if (result != 0) goto EXIT;
		}
		else {
			//Switch the live monitor's scene type
			result = _CreateScene(m_SceneType, NULL);
			if (result != 0) goto EXIT;

			//Set the MIDI IN device name
			result = m_pScene->OnMIDIINDeviceChanged(m_MIDIINDevName);
		if (result != 0) goto EXIT;

			//Notify the scene that playback has ended (live monitor stopped) so the device name is reflected on screen
			result = m_pScene->OnPlayEnd();
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: auto-save viewpoint
//******************************************************************************
int MIDITrailApp::_OnMenuAutoSaveViewpoint()
{
	int result = 0;

	m_isAutoSaveViewpoint = m_isAutoSaveViewpoint ? false : true;

	//Update menu selection mark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//Save scene settings
	result = _SaveSceneConf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: move to static viewpoint
//******************************************************************************
int MIDITrailApp::_OnMenuViewpoint(
		unsigned long viewpointNo
	)
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//Move to the static viewpoint
	m_pScene->MoveToStaticViewpoint(viewpointNo);

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: move to my viewpoint
//******************************************************************************
int MIDITrailApp::_OnMenuMyViewpoint(
		unsigned long viewpointNo
	)
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//Move to my viewpoint
	result = _MoveToMyViewpoint(viewpointNo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: save my viewpoint
//******************************************************************************
int MIDITrailApp::_OnMenuSaveMyViewpoint(
		unsigned long viewpointNo
	)
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//Save my viewpoint
	result = _SaveMyViewpoint(viewpointNo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: reset viewpoint
//******************************************************************************
int MIDITrailApp::_OnMenuResetViewpoint()
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//Reset the scene's viewpoint
	m_pScene->ResetViewpoint();

	//Save viewpoint
	result = _SaveViewpoint();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: save viewpoint
//******************************************************************************
int MIDITrailApp::_OnMenuSaveViewpoint()
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//Save viewpoint
	result = _SaveViewpoint();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: display effect settings
//******************************************************************************
int MIDITrailApp::_OnMenuEnableEffect(
		MTEffectType type
	)
{
	int result = 0;

	switch (type) {
		case MTEffectPianoKeyboard:
			m_isEnablePianoKeyboard = m_isEnablePianoKeyboard ? false : true;
			break;
		case MTEffectRipple:
			m_isEnableRipple = m_isEnableRipple ? false : true;
			break;
		case MTEffectPitchBend:
			m_isEnablePitchBend = m_isEnablePitchBend ? false : true;
			break;
		case MTEffectStars:
			m_isEnableStars = m_isEnableStars ? false : true;
			break;
		case MTEffectCounter:
			m_isEnableCounter = m_isEnableCounter ? false : true;
			break;
		case MTEffectBackgroundImage:
			m_isEnableBackgroundImage = m_isEnableBackgroundImage ? false : true;
			break;
		case MTEffectGridBox:
			m_isEnableGridLine = m_isEnableGridLine ? false : true;
			break;
		case MTEffectTimeIndicator:
			m_isEnableTimeIndicator = m_isEnableTimeIndicator ? false : true;
			break;
		case MTEffectDiagOverlay:
			m_isEnableDiagOverlay = m_isEnableDiagOverlay ? false : true;
			break;
		default:
			break;
	}

	//Apply display effect
	_UpdateEffect();

	//Update menu selection mark
	_UpdateMenuCheckmark();

	//Save the display effect selection state
	result = _SaveEffectStatus();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: change window size
//******************************************************************************
int MIDITrailApp::_OnMenuWindowSize()
{
	int result = 0;

	//Show settings dialog
	result = m_WindowSizeCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//If changed, update the window size
	if (m_WindowSizeCfgDlg.IsChanged()) {
		result = _ChangeWindowSize();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: fullscreen
//******************************************************************************
int MIDITrailApp::_OnMenuFullScreen()
{
	int result = 0;
	spdlog::info("Fullscreen: toggle");

	//Toggle fullscreen
	result = _ToggleFullScreen();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: menu bar
//******************************************************************************
int MIDITrailApp::_OnMenuMenuBar()
{
	int result = 0;

	//Toggle menu bar visibility
	result = _ToggleMenuBar();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: MIDI output device settings
//******************************************************************************
int MIDITrailApp::_OnMenuOptionMIDIOUT()
{
	int result = 0;

	//Show settings dialog
	result = m_MIDIOUTCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// Menu selection: MIDI input device settings
//******************************************************************************
int MIDITrailApp::_OnMenuOptionMIDIIN()
{
	int result = 0;

	//Show settings dialog
	result = m_MIDIINCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//Notify scene of device name change for immediate dashboard update
	if (m_pScene != NULL && m_PlayStatus == MonitorOFF) {
		result = m_MIDIConf.SetCurSection(_T("MIDIIN"));
		if (result != 0) goto EXIT;
		result = m_MIDIConf.GetStr("PortA", m_MIDIINDevName, MAXPNAMELEN, _T(""));
		if (result != 0) goto EXIT;
		result = m_pScene->OnMIDIINDeviceChanged(m_MIDIINDevName);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: graphics settings
//******************************************************************************
int MIDITrailApp::_OnMenuOptionGraphic()
{
	int result = 0;
	unsigned long multiSampleType = 0;
	bool isSupport = false;

	//Set antialiasing support information on the dialog
	for (multiSampleType = DX_MULTI_SAMPLE_TYPE_MIN; multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX; multiSampleType++) {
		result = m_Renderer.IsSupportAntialias(multiSampleType, &isSupport);
		if (result != 0) goto EXIT;
		m_GraphicCfgDlg.SetAntialiasSupport(multiSampleType, isSupport);
	}

	//Show settings dialog
	result = m_GraphicCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//If changed, recreate the renderer and scene object
	//MSAA settings are determined at SwapChain creation, so they cannot be changed via ResizeBuffers
	if (m_GraphicCfgDlg.IsChanged()) {
		bool isMonitor = (m_PlayStatus == MonitorOFF) || (m_PlayStatus == MonitorON);
		MTViewParamMap viewParamMap;

		result = _LoadGraphicConf();
		if (result != 0) goto EXIT;

		//Save the current viewpoint
		if (m_pScene != NULL) {
			m_pScene->GetViewParam(&viewParamMap);
		}

		//Destroy scene
		if (m_pScene != NULL) {
			m_pScene->Release();
			delete m_pScene;
			m_pScene = NULL;
		}

		//Terminate and reinitialize the renderer (recreate the SwapChain with the new MSAA setting)
		m_Renderer.Terminate();

		result = _SetWindowSize();
		if (result != 0) goto EXIT;

		result = m_Renderer.Initialize(m_hWnd, m_MultiSampleType);
		if (result != 0) goto EXIT;

		result = DXPrimitive11::InitPipeline(m_Renderer.GetDevice());
		if (result != 0) goto EXIT;

		//Recreate the scene
		if (!isMonitor) {
			result = _CreateScene(m_SceneType, &m_SeqData);
			if (result != 0) goto EXIT;
		}
		else {
			result = _CreateScene(m_SceneType, NULL);
			if (result != 0) goto EXIT;
			result = m_pScene->OnMIDIINDeviceChanged(m_MIDIINDevName);
		if (result != 0) goto EXIT;
			if (m_PlayStatus == MonitorON) {
				result = m_pScene->OnPlayStart();
				if (result != 0) goto EXIT;
			}
			else {
				result = m_pScene->OnPlayEnd();
				if (result != 0) goto EXIT;
			}
		}

		//Restore the viewpoint
		if (m_pScene != NULL) {
			m_pScene->SetViewParam(&viewParamMap);
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Menu selection: set color
//******************************************************************************
int MIDITrailApp::_OnMenuOptionColor()
{
	int result = 0;

	//Show settings dialog
	result = m_ColorCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//If changed, recreate the renderer and scene object
	if (m_ColorCfgDlg.IsChanged()) {
		result = _ChangeWindowSize();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Show manual
//******************************************************************************
int MIDITrailApp::_OnMenuManual()
{
	int result = 0;
	HINSTANCE hresult = 0;
	TCHAR manualPath[_MAX_PATH] = {_T('\0')};

	//Get the process executable directory path
	result = YNPathUtil::GetModuleDirPath(manualPath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//Build the manual file path
	_tcscat_s(manualPath, _MAX_PATH, MT_MANUALFILE);

	//Open the manual file
	hresult = ShellExecute(
					NULL,			//Parent window handle
					_T("open"),		//Operation
					manualPath,		//Target file for the operation
					NULL,			//Operation parameters
					NULL,			//Default directory
					SW_SHOWNORMAL	//Display state
				);
	if (hresult <= (HINSTANCE)32) {
		result = YN_SET_ERR("File open error.", (DWORD64)hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Sequencer message processing
//******************************************************************************
int MIDITrailApp::_SequencerMsgProc()
{
	int result = 0;
	bool isExist = false;
	unsigned long param1 = 0;
	unsigned long param2 = 0;
	SMMsgParser parser;
	
	while (true) {
		//Retrieve message
		result = m_MsgQueue.GetMessage(&isExist, &param1, &param2);
		if (result != 0) goto EXIT;

		//Exit if there is no message
		if (!isExist) break;

		//Sequencer message reception processing
		result = _OnRecvSequencerMsg(param1, param2);
		if (result != 0) goto EXIT;	
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Sequencer message reception
//******************************************************************************
int MIDITrailApp::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;
	bool isExist = false;

	//Pass the sequencer message to the scene
	if (m_pScene != NULL) {
		result = m_pScene->OnRecvSequencerMsg(param1, param2);
		if (result != 0) goto EXIT;
	}

	//Handle the playback status change notification
	parser.Parse(param1, param2);
	if (parser.GetMsg() == SMMsgParser::MsgPlayStatus) {
		//Paused
		if (parser.GetPlayStatus() == SMMsgParser::StatusPause) {
			result = _ChangePlayStatus(Pause);
			if (result != 0) goto EXIT;
		}
		//Stop (playback ended)
		if (parser.GetPlayStatus() == SMMsgParser::StatusStop) {
			result = _ChangePlayStatus(Stop);
			if (result != 0) goto EXIT;

			//Notify the scene that playback has ended
			if (m_pScene != NULL) {
				result = m_pScene->OnPlayEnd();
				if (result != 0) goto EXIT;
			}

			//Clear Playback metrics (consolidated from 3 scene base classes)
			RDDiagManager::SetInt(RDMetricId::PlaybackNoteTracking, 0);
			RDDiagManager::SetInt(RDMetricId::PlaybackPolyphony, 0);

			//Save viewpoint
			if (m_isAutoSaveViewpoint) {
				result = _OnMenuSaveViewpoint();
				if (result != 0) goto EXIT;
			}

			//Rewind if playback was stopped at the user's request
			if ((m_isRewind) && (m_pScene != NULL)) {
				m_isRewind = false;
				result = m_pScene->Rewind();
				if (result != 0) goto EXIT;
			}
			//If a file open after stop has been specified
			else if ((m_isOpenFileAfterStop) && (m_pScene != NULL)) {
				m_isOpenFileAfterStop = false;
				//File loading process
				result = _FileOpenProc(m_NextFilePath);
				if (result != 0) goto EXIT;
			}
			//For a normal end of playback
			else {
				//For a normal end of playback, rewind for the next playback
				m_isRewind = true;
				//If there is no file list
				if (m_MIDIFileList.GetFileCount() == 0) {
					//Start playback if repeat is enabled
					if (m_isRepeat) {
						//Start MIDI file playback via repeat
						result = _StartTimer_Play(m_DelayBetweenSongsInMsec);
						if (result != 0) goto EXIT;
					}
				}
				//If there is a file list
				else {
					//Start playback if folder playback is disabled and repeat is enabled
					if (!m_isFolderPlayback && m_isRepeat) {
						//Start MIDI file playback via repeat
						result = _StartTimer_Play(m_DelayBetweenSongsInMsec);
						if (result != 0) goto EXIT;
					}
					//Automatically select the next file if folder playback is enabled
					else if (m_isFolderPlayback) {
						m_MIDIFileList.SelectNextFile(&isExist);
						if (isExist) {
							//If the next file exists
							//Start playback of the next MIDI file via folder playback
							result = _StartTimer_OpenFileAndPlay(m_DelayBetweenSongsInMsec);
							if (result != 0) goto EXIT;
						}
						else if (m_isRepeat) {
							//If the next file does not exist but repeat is enabled
							//Start playback of the next MIDI file via folder playback
							result = _StartTimer_OpenFileAndPlay(m_DelayBetweenSongsInMsec);
							if (result != 0) goto EXIT;
							//Select the first file
							m_MIDIFileList.SelectFirstFile();
						}
					}
				}
			}

			//If exit is specified on the command line
			if (m_CmdLineParser.GetSwitch(CMDSW_QUIET) == CMDSW_ON) {
				DestroyWindow(m_hWnd);
			}
		}
	}

	//Device lost countermeasure
	//Record the latest message passed to the scene
	if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		//Playback tick time notification
		m_SequencerLastMsg.isRecvPlayTime = true;
		m_SequencerLastMsg.playTime.param1 = param1;
		m_SequencerLastMsg.playTime.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		//Tempo change notification
		m_SequencerLastMsg.isRecvTempo = true;
		m_SequencerLastMsg.tempo.param1 = param1;
		m_SequencerLastMsg.tempo.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		//Bar number notification
		m_SequencerLastMsg.isRecvBar = true;
		m_SequencerLastMsg.bar.param1 = param1;
		m_SequencerLastMsg.bar.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		//Time signature change notification
		m_SequencerLastMsg.isRecvBeat = true;
		m_SequencerLastMsg.beat.param1 = param1;
		m_SequencerLastMsg.beat.param2 = param2;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Window click event
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
// Mouse move event
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
	
	//In fullscreen mode
	if (m_isFullScreen) {
		//If the mouse cursor moves to the top edge of the screen
		if (point.y == 0) {
			//Show menu bar
			result = _ShowMenu();
			if (result != 0) goto EXIT;
		}
		else {
			//Hide menu bar
			result = _HideMenu();
			if (result != 0) goto EXIT;
		}
	}
	//In windowed mode
	else {
		//If the menu is hidden
		//Show the menu only when the mouse cursor moves near the top edge of the window
		if (!m_isEnableMenuBar) {
			if (point.y <= 5) {
				//Show menu bar
				result = _ShowMenu();
				if (result != 0) goto EXIT;
			}
			else {
				//Hide menu bar
				result = _HideMenu();
				if (result != 0) goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Key input event
//******************************************************************************
int MIDITrailApp::_OnKeyDown(
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	unsigned short keycode = 0;

	keycode = LOWORD((DWORD)wParam);

	if (m_isLoading) goto EXIT;

	switch (keycode) {
		case VK_SPACE:
		case VK_NUMPAD0:
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				//Start monitoring
				result = _OnMenuStartMonitoring();
				if (result != 0) goto EXIT;
			}
			else {
				//Start/pause playback
				result = _OnMenuPlay();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_ESCAPE:
			if (m_PlayStatus == MonitorON) {
				//Stop monitoring
				result = _OnMenuStopMonitoring();
				if (result != 0) goto EXIT;
			}
			else {
				//Stop playback
				result = _OnMenuStop();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_RETURN:
			//Stop playback: when using ENTER on the numeric keypad with NUMLOCK on
			if ((HIWORD((DWORD)lParam) & KF_EXTENDED) && (GetKeyState(VK_NUMLOCK) & 0x01)) {
				result = _OnMenuStop();
				if (result != 0) goto EXIT;
			}
			break;
		case '1':
		case VK_NUMPAD1:
			//Playback skip back
			result = _OnMenuSkipBack();
			if (result != 0) goto EXIT;
			break;
		case '2':
		case VK_NUMPAD2:
			//Playback skip forward
			result = _OnMenuSkipForward();
			if (result != 0) goto EXIT;
			break;
		case '4':
		case VK_NUMPAD4:
			//Playback speed down
			result = _OnMenuPlaySpeedDown();
			if (result != 0) goto EXIT;
			break;
		case '5':
		case VK_NUMPAD5:
			//Playback speed up
			result = _OnMenuPlaySpeedUp();
			if (result != 0) goto EXIT;
			break;
		case '7':
		case VK_NUMPAD7:
			if ((GetKeyState(VK_SHIFT) & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000)) {
				//Save my viewpoint 1
				result = _OnMenuSaveMyViewpoint(1);
				if (result != 0) goto EXIT;
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000) {
				//Move to my viewpoint 1
				result = _OnMenuMyViewpoint(1);
				if (result != 0) goto EXIT;
			}
			else {
				//Reset viewpoint
				result = _OnMenuResetViewpoint();
				if (result != 0) goto EXIT;
			}
			break;
		case '8':
		case VK_NUMPAD8:
			if ((GetKeyState(VK_SHIFT) & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000)) {
				//Save my viewpoint 2
				result = _OnMenuSaveMyViewpoint(2);
				if (result != 0) goto EXIT;
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000) {
				//Move to my viewpoint 2
				result = _OnMenuMyViewpoint(2);
				if (result != 0) goto EXIT;
			}
			else {
				//Move to static viewpoint 2
				result = _OnMenuViewpoint(2);
				if (result != 0) goto EXIT;
			}
			break;
		case '9':
		case VK_NUMPAD9:
			if ((GetKeyState(VK_SHIFT) & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000)) {
				//Save my viewpoint 3
				result = _OnMenuSaveMyViewpoint(3);
				if (result != 0) goto EXIT;
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000) {
				//Move to my viewpoint 3
				result = _OnMenuMyViewpoint(3);
				if (result != 0) goto EXIT;
			}
			else {
				//Move to static viewpoint 3
				result = _OnMenuViewpoint(3);
				if (result != 0) goto EXIT;
			}
			break;
		case 'O':
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				//Open file
				result = _OnMenuOpenFile();
				if (result != 0) goto EXIT;
			}
			break;
		case 'B':
		case 'P':
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				//Previous file
				result = _OnMenuPreviousFile();
				if (result != 0) goto EXIT;
			}
			break;
		case 'N':
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				//Next file
				result = _OnMenuNextFile();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_F11:
			//Fullscreen
			result = _OnMenuFullScreen();
			if (result != 0) goto EXIT;
			break;
		case VK_F12:
			//Menu bar
			result = _OnMenuMenuBar();
			if (result != 0) goto EXIT;
			break;
		default:
			break;
	}

EXIT:;
	return result;
}

//******************************************************************************
// File drop event
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
	WCHAR path[_MAX_PATH] = { L'\0' };
	bool isMIDIDataFile = false;

	////Ignore file drops unless stopped
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	//File drop OK
	//}
	//else {
	//	//File drop NG
	//	goto EXIT;
	//}

	//Always allow file drops

	hDrop = (HDROP)wParam;

	//Check the number of files
	fileNum = DragQueryFile(
					hDrop,		//wParam
					0xFFFFFFFF,	//File index
					NULL,		//File name retrieval buffer
					0			//Buffer size
				);

	//Ignore if there are multiple files
	if (fileNum != 1) goto EXIT;

	//Get file path
	charNum = DragQueryFileW(
					hDrop,		//wParam
					0,			//File index
					path,		//File name retrieval buffer
					_MAX_PATH	//Buffer size
				);
	if (charNum == 0) {
		result = YN_SET_ERR("Windows API error.", wParam, lParam);
		goto EXIT;
	}

	//If a folder was dropped
	if (PathIsDirectoryW(path)) {
		//Stop playback/monitoring and open folder
		result = _StopPlaybackAndOpenFolder(path);
		if (result != 0) goto EXIT;
	}
	//If a file was dropped
	else {
		//Check the file extension
		if (YNPathUtil::IsFileExtMatch(path, L".mid")) {
			isMIDIDataFile = true;
		}
		//If rcpcv.dll is available, additionally check whether the file is a supported type
		else if (m_RcpConv.IsAvailable() && m_RcpConv.IsSupportFileExt(path)) {
			isMIDIDataFile = true;
		}

		//Do nothing if the file is not a supported type
		if (!isMIDIDataFile) goto EXIT;

		//Destroy file list
		m_MIDIFileList.Clear();

		//Stop playback/monitoring and open file
		result = _StopPlaybackAndOpenFile(path);
		if (result != 0) goto EXIT;
	}

	//Update menu style
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	if (hDrop != NULL) {
		DragFinish(hDrop);
	}
	return result;
}

//******************************************************************************
// Select file
//******************************************************************************
int MIDITrailApp::_SelectMIDIFile(
		WCHAR* pFilePath,
		unsigned long bufSize,
		bool* pIsSelected
	)
{
	int result = 0;
	BOOL apiresult = FALSE;
	OPENFILENAMEW ofn;

	if ((pFilePath == NULL) || (bufSize == 0) || (pIsSelected ==NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pFilePath[0] = L'\0';
	ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner   = m_hWnd;
	ofn.lpstrFilter = L"Standard MIDI File (*.mid)\0*.mid\0\0";
	ofn.lpstrFile   = pFilePath;
	ofn.nMaxFile    = bufSize;
	ofn.lpstrTitle  = L"Select Standard MIDI File.";
	ofn.Flags       = OFN_FILEMUSTEXIST;  //OFN_HIDEREADONLY

	//If rcpcv.dll is available, change the file filter
	if (m_RcpConv.IsAvailable()) {
		ofn.lpstrFilter = m_RcpConv.GetOpenFileNameFilter();
	}

	//Show the file selection dialog
	apiresult = GetOpenFileNameW(&ofn);
	if (!apiresult) {
		//Canceled or an error occurred: the error is not checked
		*pIsSelected = false;
		goto EXIT;
	}

	*pIsSelected = true;

EXIT:;
	return result;
}

//******************************************************************************
// Select folder
//******************************************************************************
int MIDITrailApp::_SelectFolder(
		WCHAR* pFolderPath,
		unsigned long bufSize,
		bool* pIsSelected
	)
{
	int result = 0;
	int apiresult = 0;
	errno_t eresult = 0;
	HRESULT hresult = 0;
	DWORD options = 0;
	IFileOpenDialog* pFileOpenDialog = NULL;
	LPWSTR pFolderPathW = NULL;
	IShellItem* pShellItem = NULL;
	BOOL isUsedDefaultChar = FALSE;
	
	if ((pFolderPath == NULL) || (bufSize == 0) || (pIsSelected == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	*pIsSelected = false;

	//Create dialog
	hresult = CoCreateInstance(
					CLSID_FileOpenDialog,			//CLSID
					NULL,							//Aggregate object
					CLSCTX_INPROC_SERVER,			//Context
					IID_PPV_ARGS(&pFileOpenDialog)	//IID and variable address
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}

	//Set options
	pFileOpenDialog->GetOptions(&options);
	pFileOpenDialog->SetOptions(options | FOS_PICKFOLDERS);

	//Show the dialog (folder selection only)
	//  Specifying m_hWnd causes the dialog to hang when shown after playback starts (cause unknown)
	hresult = pFileOpenDialog->Show(NULL);
	if (hresult == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		//If canceled, exit without doing anything
		goto EXIT;
	}
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), hresult);
		goto EXIT;
	}

	//If a folder is selected, get its path
	//Note that it can only be obtained as a wide string
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
	if (pFolderPathW != NULL) {
		CoTaskMemFree(pFolderPathW);
	}
	if (pShellItem != NULL) {
		pShellItem->Release();
	}
	if (pFileOpenDialog != NULL) {
		pFileOpenDialog->Release();
	}
	return result;
}

//******************************************************************************
// Load MIDIfile
//******************************************************************************
int MIDITrailApp::_LoadMIDIFile(
		const WCHAR* pFilePath
	)
{
	int result = 0;
	WCHAR* pPath = NULL;
	WCHAR smfTempPath[_MAX_PATH] = { L'\0' };
	WCHAR smfDumpPath[_MAX_PATH] = { L'\0' };
	SMFileReader smfReader;
	MTLoadProgressContext topCtx;

	m_isLoading = true;

	LARGE_INTEGER perfFreq, perfT0, perfT1;
	QueryPerformanceFrequency(&perfFreq);
	spdlog::info("=== Load started ===");
	spdlog::info(L"File: {}", pFilePath);

	//Update window title
	//Show it before loading the file so the file name can be checked if an error occurs
	_UpdateWindowTitle(PathFindFileNameW(pFilePath));

	//If the extension is *.mid
	if (YNPathUtil::IsFileExtMatch(pFilePath, L".mid")) {
		pPath = (WCHAR*)pFilePath;
	}
	//If rcpcv.dll is available and the file is a supported type, convert to SMF
	else if (m_RcpConv.IsAvailable() && m_RcpConv.IsSupportFileExt(pFilePath)) {
		result = YNPathUtil::GetTempFilePath(smfTempPath, _MAX_PATH, L"RCP");
		if (result != 0) goto EXIT;
		result = m_RcpConv.Convert(pFilePath, smfTempPath);
		if (result != 0) goto EXIT;
		pPath = smfTempPath;
	}
	//If neither applies
	else {
		//Load as-is
		pPath = (WCHAR*)pFilePath;
	}

	//Dump the MIDI file parsing result if in debug mode
	if (m_CmdLineParser.GetSwitch(CMDSW_DUMP_MIDI) == CMDSW_ON) {
		wcscat_s(smfDumpPath, _MAX_PATH, pPath);
		wcscat_s(smfDumpPath, _MAX_PATH, L".dump.txt");
		smfReader.SetLogPath(smfDumpPath);
	}

	//Initialize loading screen
	m_LoadingScreen.Create(m_hWnd, &m_Renderer);
	m_LoadingScreen.Update(0.0f, "Opening MIDI file...");

	topCtx = MTLoadProgressContext(&_OnLoadingProgress, this);

	spdlog::debug("SMFileReader::Load begin");
	QueryPerformanceCounter(&perfT0);
	{
		MTProgressBand parseBand = { &topCtx, 0.0f, MTLoadBand::PARSE_END };
		MTLoadProgressContext parseCtx = parseBand.ToContext();
		result = smfReader.Load(pPath, &m_SeqData, &MTProgressBand::Callback, &parseBand);
		if (result != 0) goto EXIT;
	}
	QueryPerformanceCounter(&perfT1);
	spdlog::debug("SMFileReader::Load: {} ms", (perfT1.QuadPart - perfT0.QuadPart) * 1000 / perfFreq.QuadPart);

	//Register file name
	m_SeqData.SetFileName(PathFindFileNameW(pFilePath));

	m_LoadingScreen.Update(MTLoadBand::PARSE_END, "Building scene...");

	m_PlaySpeedRatio = 100;

	spdlog::debug("_CreateScene begin");
	m_SceneType = m_SelectedSceneType;
	{
		MTProgressBand buildBand = { &topCtx, MTLoadBand::PARSE_END, MTLoadBand::BUILD_END };
		MTLoadProgressContext buildCtx = buildBand.ToContext();
		QueryPerformanceCounter(&perfT0);
		result = _CreateScene(m_SceneType, &m_SeqData, &buildCtx);
		QueryPerformanceCounter(&perfT1);
	}
	spdlog::debug("_CreateScene: {} ms", (perfT1.QuadPart - perfT0.QuadPart) * 1000 / perfFreq.QuadPart);
	if (result != 0) goto EXIT;

	{
		static const char* sceneNames[] = { "Title", "PianoRoll3D", "PianoRoll2D", "PianoRollRain",
			"PianoRollRain2D", "PianoRollRing", "PianoRoll3DLive", "PianoRoll2DLive",
			"PianoRollRainLive", "PianoRollRain2DLive", "PianoRollRingLive" };

		char fileNameA[256] = {0};
		WideCharToMultiByte(CP_UTF8, 0, m_SeqData.GetFileName(), -1, fileNameA, 256, NULL, NULL);
		RDDiagManager::SetString(RDMetricId::PlaybackLoadedFileName, fileNameA);
		RDDiagManager::SetInt(RDMetricId::PlaybackTotalPlayTimeMs,
			static_cast<int64_t>(m_SeqData.GetTotalPlayTime()));
		RDDiagManager::SetString(RDMetricId::PlaybackSceneType, sceneNames[m_SceneType]);

		RDDiagManager::LogEvent(RDFormatProfile::FileLoaded,
			RDFormatProfile::FileLoadedCount, "file-loaded");
	}

	//Stop sequencer if it was started during loading (BUG-19 safety net)
	if (m_Sequencer.GetStatus() != SMSequencer::StatusStop) {
		m_Sequencer.Stop();
	}

	//Change playback status
	result = _ChangePlayStatus(Stop);
	if (result != 0) goto EXIT;

	m_LoadingScreen.Update(1.0f, "Complete");
	m_LoadingScreen.Release();

	m_isRewind = false;

EXIT:;
	m_isLoading = false;
	spdlog::info("=== Load {} ===", (result == 0) ? "completed" : "failed");
	if (wcslen(smfTempPath) != 0) {
		DeleteFileW(smfTempPath);
	}
	return result;
}

//******************************************************************************
// Loading progress callback (unified, no band conversion)
//******************************************************************************
void MIDITrailApp::_OnLoadingProgress(
		unsigned long current,
		unsigned long total,
		const char* message,
		void* userData
	)
{
	auto* app = static_cast<MIDITrailApp*>(userData);
	float progress = (total > 0) ? (float)current / (float)total : 0.0f;
	app->m_LoadingScreen.Update(progress, message);
}


//******************************************************************************
// Additional MIDI file loading
// If the file name contains "portX", X is treated as the port number (a-Z: case-insensitive)
// If the file name contains "chXX", XX is treated as the channel number (00-99)
//******************************************************************************
int MIDITrailApp::_AddMIDIFile(
		const WCHAR* pFilePath
	)
{
	int result = 0;
	WCHAR* pPath = NULL;
	WCHAR smfTempPath[_MAX_PATH] = { L'\0' };
	WCHAR smfDumpPath[_MAX_PATH] = { L'\0' };
	SMSeqData tmpSeqData;
	SMFileReader smfReader;
	int portNo = -1;
	int chNo = -1;
	WCHAR* pPortNo = NULL;
	WCHAR* pChNo = NULL;

	//If the extension is *.mid
	if (YNPathUtil::IsFileExtMatch(pFilePath, L".mid")) {
		pPath = (WCHAR*)pFilePath;
	}
	//If rcpcv.dll is available and the file is a supported type, convert to SMF
	else if (m_RcpConv.IsAvailable() && m_RcpConv.IsSupportFileExt(pFilePath)) {
		result = YNPathUtil::GetTempFilePath(smfTempPath, _MAX_PATH, L"RCP");
		if (result != 0) goto EXIT;
		result = m_RcpConv.Convert(pFilePath, smfTempPath);
		if (result != 0) goto EXIT;
		pPath = smfTempPath;
	}
	//If neither applies
	else {
		//Load as-is
		pPath = (WCHAR*)pFilePath;
	}

	//Dump the MIDI file parsing result if in debug mode
	if (m_CmdLineParser.GetSwitch(CMDSW_DUMP_MIDI) == CMDSW_ON) {
		wcscat_s(smfDumpPath, _MAX_PATH, pPath);
		wcscat_s(smfDumpPath, _MAX_PATH, L".dump.txt");
		smfReader.SetLogPath(smfDumpPath);
	}

	//Load the file into a temporary sequence
	result = smfReader.Load(pPath, &tmpSeqData);
	if (result != 0) goto EXIT;

	//Extract the port number if it is included in the file name
	pPortNo = wcsstr(pPath, L"port");
	if(pPortNo != NULL) {
		portNo = towlower(*(pPortNo + 4)) - L'a';
		spdlog::debug("port {} added.", (char)('A' + portNo));
	}

	//Extract the channel number if it is included in the file name
	pChNo = wcsstr(pPath, L"ch");
	if(pChNo != NULL) {
		WCHAR bufChNo[3];
		wcsncpy_s(bufChNo, 3, pChNo + 2, 2);
		bufChNo[2] = L'\0';
		chNo = _wtoi(bufChNo) - 1;
		spdlog::debug("chNo {} added.", chNo);
	}

	//Merge the temporary sequence
	m_SeqData.AddSequence(tmpSeqData, portNo, chNo);

	//Reset the playback speed to 100% when loading a file: reflected in the counter by _CreateScene
	m_PlaySpeedRatio = 100;

	//Create scene object
	m_SceneType = m_SelectedSceneType;
	result = _CreateScene(m_SceneType, &m_SeqData);
	if (result != 0) goto EXIT;

	//Change playback status
	result = _ChangePlayStatus(Stop);
	if (result != 0) goto EXIT;

	m_isRewind = false;

EXIT:;
	if (wcslen(smfTempPath) != 0) {
		DeleteFileW(smfTempPath);
	}
	return result;
}


//******************************************************************************
// Update window title
//******************************************************************************
void MIDITrailApp::_UpdateWindowTitle(const WCHAR* pFileName)
{
	WCHAR format[MAX_LOADSTRING];

	wcscpy_s(format, MAX_LOADSTRING, m_TitleBase);

	//If there is no file name
	if (pFileName == NULL) {
		swprintf_s(
			m_Title,
			MAX_LOADSTRING,
			format
		);
	}
	else {
		//If there is no file list
		if (m_MIDIFileList.GetFileCount() == 0) {
			wcscat_s(
				format,
				MAX_LOADSTRING,
				MIDITRAIL_WINDOW_TITLE_FILE
			);

			swprintf_s(
				m_Title,
				MAX_LOADSTRING,
				format,
				pFileName
			);
		}
		//If there is a file list
		else {
			wcscat_s(
				format,
				MAX_LOADSTRING,
				MIDITRAIL_WINDOW_TITLE_FILES
			);

			swprintf_s(
				m_Title,
				MAX_LOADSTRING,
				format,
				(int)m_MIDIFileList.GetSelectedFileIndex() + 1,
				(int)m_MIDIFileList.GetFileCount(),
				pFileName
			);
		}
	}

	//Set window title
	SetWindowTextW(m_hWnd, m_Title);

	return;
}

//******************************************************************************
// Update FPS
//******************************************************************************
void MIDITrailApp::_UpdateFPS()
{
	unsigned long curTime = 0;
	unsigned long diffTime = 0;
	double fps = 0;
	WCHAR title[MAX_LOADSTRING];

	curTime = timeGetTime();
	m_FPSCount += 1;

	//Calculate FPS every second
	diffTime = curTime - m_PrevTime;
	if (diffTime > 1000) {

		//FPS
		fps = (double)m_FPSCount / ((double)diffTime / 1000.0f);
		m_PrevTime = curTime;
		m_FPSCount = 0;

		//Set on window title
		swprintf_s(title, MAX_LOADSTRING, MIDITRAIL_WINDOW_TITLE_FPS, m_Title, fps);
		SetWindowTextW(m_hWnd, title);
	}

	return;
}

//******************************************************************************
// Register portinfo
//******************************************************************************
int MIDITrailApp::_SetPortDev(
		SMSequencer* pSequencer
	)
{
	int result = 0;
	unsigned char portNo = 0;
	TCHAR devName[MAXPNAMELEN];
	const char* portName[] = {"PortA", "PortB", "PortC", "PortD", "PortE", "PortF"};

	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//Get the user-selected device name from the config file and register it with the sequencer
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
// Register MIDI IN monitor information
//******************************************************************************
int MIDITrailApp::_SetMonitorPortDev(
		SMLiveMonitor* pLiveMonitor,
		IMTScene11* pScene
	)
{
	int result = 0;
	TCHAR midiOutDevName[MAXPNAMELEN];
	int checkMIDITHRU = 0;
	bool isMIDITHRU = false;

	//--------------------------------------
	// MIDI IN
	//--------------------------------------
	//Category/section setting
	result = m_MIDIConf.SetCurSection(_T("MIDIIN"));
	if (result != 0) goto EXIT;

	//Get the user-selected device name from the config file and register it with the sequencer
	result = m_MIDIConf.GetStr("PortA", m_MIDIINDevName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;
	result = m_MIDIConf.GetInt("MIDITHRU", &checkMIDITHRU, 1);
	if (result != 0) goto EXIT;

	if (checkMIDITHRU > 0) {
		isMIDITHRU = true;
	}
	if (_tcslen(m_MIDIINDevName) > 0) {
		result = pLiveMonitor->SetInPortDev(m_MIDIINDevName, isMIDITHRU);
		if (result != 0) goto EXIT;
	}

	//Register the MIDI IN device name with the scene
	result = pScene->OnMIDIINDeviceChanged(m_MIDIINDevName);
	if (result != 0) goto EXIT;

	//--------------------------------------
	// MIDI OUT (MIDITHRU)
	//--------------------------------------
	//Category/section setting
	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//Get the user-selected device name from the config file and register it with the sequencer
	result = m_MIDIConf.GetStr("PortA", midiOutDevName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;

	if ((_tcslen(midiOutDevName) > 0) && (isMIDITHRU)) {
		result = pLiveMonitor->SetOutPortDev(midiOutDevName);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Window resize
//******************************************************************************
int MIDITrailApp::_ChangeWindowSize()
{
	int result = 0;

	//Change window style/position (resize the window before OnResize)
	//The cache update is performed inside _SetWindowSize
	result = _SetWindowSize();
	if (result != 0) goto EXIT;

	//ResizeBuffers (no Device recreation)
	result = m_Renderer.OnResize();
	if (result != 0) goto EXIT;

	//Notify size-dependent components
	if (m_pScene != NULL) {
		m_pScene->OnWindowResize();
	}

EXIT:;
	return result;
}

//******************************************************************************
// Change playback status
//******************************************************************************
int MIDITrailApp::_ChangePlayStatus(
		PlayStatus status
	)
{
	int result = 0;

	{
		const char* names[] = { "NoData", "Stop", "Play", "Pause", "MonitorON", "MonitorOFF" };
		spdlog::info("PlayStatus: {} -> {}", names[m_PlayStatus], names[status]);
	}

	//Change playback status
	m_PlayStatus = status;

	//Update MIDI output transport state metric
	{
		if (status == Play || status == MonitorON) {
			auto transport = m_Sequencer.GetTransportType();
			const char* transportStr = _TransportTypeToString(transport);
			RDDiagManager::SetString(RDMetricId::MidiOutTransport, transportStr);
			spdlog::info("MIDI OUT transport: {}", transportStr);

			// MIDI OUT device info
			TCHAR devName[MAXPNAMELEN];
			std::string devNames;
			int activePorts = 0;
			if (m_MIDIConf.SetCurSection(_T("MIDIOUT")) == 0) {
				const char* portName[] = {"PortA", "PortB", "PortC", "PortD", "PortE", "PortF"};
				for (int i = 0; i < SM_MIDIOUT_PORT_NUM_MAX; i++) {
					if (m_MIDIConf.GetStr(portName[i], devName, MAXPNAMELEN, _T("")) == 0 && devName[0] != '\0') {
						if (!devNames.empty()) devNames += ", ";
						devNames += devName;
						activePorts++;
					}
				}
			}
			RDDiagManager::SetString(RDMetricId::MidiOutDeviceName, devNames.empty() ? "N/A" : devNames.c_str());
			RDDiagManager::SetInt(RDMetricId::MidiOutActivePorts, activePorts);
		}
		else if (status == Stop || status == NoData || status == MonitorOFF) {
			RDDiagManager::SetString(RDMetricId::MidiOutTransport, "Deactivated");
			spdlog::info("MIDI OUT deactivated");
		}
	}

	////Allow file drag
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	DragAcceptFiles(m_hWnd, TRUE);
	//}
	//else {
	//	DragAcceptFiles(m_hWnd, FALSE);
	//}

	//Always allow file drag
	DragAcceptFiles(m_hWnd, TRUE);

	//Update menu style
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Update menu style
//******************************************************************************
int MIDITrailApp::_ChangeMenuStyle()
{
	int result = 0;
	unsigned long menuIndex = 0;
	unsigned long statusIndex = 0;
	unsigned long style = 0;

	//Menu ID list
	//TAG: add scene
	unsigned long menuID[MT_MENU_NUM] = {
		IDM_OPEN_FILE,
		IDM_ADD_FILE,
		IDM_OPEN_FOLDER,
		IDM_PREVIOUS_FILE,
		IDM_NEXT_FILE,
		IDM_EXIT,
		IDM_PLAY,
		IDM_STOP,
		IDM_REPEAT,
		IDM_FOLDER_PLAYBACK,
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
		IDM_ENABLE_GRIDLINE,
		IDM_ENABLE_TIMEINDICATOR,
		IDM_ENABLE_DIAGOVERLAY,
		IDM_RESET_VIEWPOINT,
		IDM_VIEWPOINT2,
		IDM_VIEWPOINT3,
		IDM_MYVIEWPOINT1,
		IDM_MYVIEWPOINT2,
		IDM_MYVIEWPOINT3,
		IDM_SAVE_MYVIEWPOINT1,
		IDM_SAVE_MYVIEWPOINT2,
		IDM_SAVE_MYVIEWPOINT3,
		IDM_WINDOWSIZE,
		IDM_FULLSCREEN,
		IDM_MENUBAR,
		IDM_OPTION_MIDIOUT,
		IDM_OPTION_MIDIIN,
		IDM_OPTION_GRAPHIC,
		IDM_OPTION_COLOR,
		IDM_HOWTOVIEW,
		IDM_MANUAL,
		IDM_ABOUT
	};

	//Menu style list
	unsigned long menuStyle[MT_MENU_NUM][MT_PLAYSTATUS_NUM] = {
		//NoData,       Stop,       Playing,    Paused,     MonitorOFF, MonitorON
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_OPEN_FILE
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_ADD_FILE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_OPEN_FOLDER
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_PREVIOUS_FILE
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_NEXT_FILE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_EXIT
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_PLAY
		{	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_STOP
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_REPEAT
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED	},	//IDM_FOLDER_PLAYBACK
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
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_GRIDLINE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_TIMEINDICATOR
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_DIAGOVERLAY
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_RESET_VIEWPOINT
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_VIEWPOINT2
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_VIEWPOINT3
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_MYVIEWPOINT1
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_MYVIEWPOINT2
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_MYVIEWPOINT3
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_SAVE_MYVIEWPOINT1
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_SAVE_MYVIEWPOINT2
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_SAVE_MYVIEWPOINT3
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_WINDOWSIZE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_FULLSCREEN
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_MENUBAR
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPTION_MIDIOUT
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPTION_MIDIIN
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPTION_GRAPHIC
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPTION_COLOR
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

	//Update menu style
	for (menuIndex = 0; menuIndex < MT_MENU_NUM; menuIndex++) {
		style = menuStyle[menuIndex][statusIndex];
		EnableMenuItem(GetMenu(m_hWnd), menuID[menuIndex], style);
	}

	//"Previous File" and "Next File" cannot be selected when there is no file list
	if (m_MIDIFileList.GetFileCount() == 0) {
		EnableMenuItem(GetMenu(m_hWnd), IDM_PREVIOUS_FILE, MF_GRAYED);
		EnableMenuItem(GetMenu(m_hWnd), IDM_NEXT_FILE, MF_GRAYED);
		EnableMenuItem(GetMenu(m_hWnd), IDM_FOLDER_PLAYBACK, MF_GRAYED);
	}

	return result;
}

//******************************************************************************
// Create scene
//******************************************************************************
int MIDITrailApp::_CreateScene(
		SceneType type,
		SMSeqData* pSeqData,  //NULL for live monitor
		const MTLoadProgressContext* pProgress
	)
{
	int result = 0;

	RDDiagManager::ResetFrameMetrics();
	m_PrevFrameQPC.QuadPart = 0;

	//Destroy scene
	if (m_pScene != NULL) {
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}

	//Create scene object
	//TAG: add scene
	// TODO: Add after implementing DX11 versions of the Title/2D/Rain/Ring/Live scenes
	try {
		if (type == Title) {
			m_pScene = new MTSceneTitle11();
		}
		else {
			//Create the player scene
			if (pSeqData != NULL) {
				if (type == PianoRoll3D) {
					m_pScene = new MTScenePianoRoll3D11();
				}
				else if (type == PianoRoll2D) {
					m_pScene = new MTScenePianoRoll3D11(true);
				}
				else if (type == PianoRollRain) {
					m_pScene = new MTScenePianoRollRain11();
				}
				else if (type == PianoRollRain2D) {
					m_pScene = new MTScenePianoRollRain11(true);
				}
				else if (type == PianoRollRing) {
					m_pScene = new MTScenePianoRollRing11();
				}
			}
			//Create the live monitor scene (DX11: isLive=true)
			else {
				if (type == PianoRoll3D) {
					m_pScene = new MTScenePianoRoll3DLive11();
				}
				else if (type == PianoRoll2D) {
					m_pScene = new MTScenePianoRoll3DLive11(true);
				}
				else if (type == PianoRollRain) {
					m_pScene = new MTScenePianoRollRainLive11();
				}
				else if (type == PianoRollRain2D) {
					m_pScene = new MTScenePianoRollRainLive11(true);
				}
				else if (type == PianoRollRing) {
					m_pScene = new MTScenePianoRollRingLive11();
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

	//Create the scene
	// The Title scene has no data: pass pSeqData as NULL
	{
		LARGE_INTEGER tS0, tS1, tFreq;
		QueryPerformanceFrequency(&tFreq);

		SMSeqData* pCreateSeqData = (type == Title) ? NULL : pSeqData;
		spdlog::debug("Scene::Create begin");
		QueryPerformanceCounter(&tS0);
		result = m_pScene->Create(m_hWnd, m_Renderer.GetDevice(), m_Renderer.GetContext(), pCreateSeqData, pProgress);
		QueryPerformanceCounter(&tS1);
		spdlog::debug("Scene::Create: {} ms", (tS1.QuadPart - tS0.QuadPart) * 1000 / tFreq.QuadPart);
		if (result != 0) goto EXIT;

		spdlog::debug("Post-scene begin");
		QueryPerformanceCounter(&tS0);

		//Apply the saved viewpoint to the scene
		if (type != Title) {
			result = _LoadViewpoint();
			if (result != 0) goto EXIT;
		}

		_UpdateEffect();
		m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);

		QueryPerformanceCounter(&tS1);
		spdlog::debug("Post-scene: {} ms", (tS1.QuadPart - tS0.QuadPart) * 1000 / tFreq.QuadPart);
	}

	if (result == 0 && type != Title) {
		static const char* sceneNames[] = { "Title", "PianoRoll3D", "PianoRoll2D", "PianoRollRain",
			"PianoRollRain2D", "PianoRollRing", "PianoRoll3DLive", "PianoRoll2DLive",
			"PianoRollRainLive", "PianoRollRain2DLive", "PianoRollRingLive" };
		RDDiagManager::SetString(RDMetricId::PlaybackSceneType, sceneNames[type]);
		RDDiagManager::LogEvent(RDFormatProfile::SceneReady,
			RDFormatProfile::SceneReadyCount, "scene-ready");
	}

EXIT:;
	return result;
}

//******************************************************************************
// Load scene type
//******************************************************************************
int MIDITrailApp::_LoadSceneType()
{
	int result = 0;
	TCHAR type[256];

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	result = m_ViewConf.GetStr(_T("Type"), type, 256, _T(""));
	if (result != 0) goto EXIT;

	//TAG: add scene
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
// Save scenetype
//******************************************************************************
int MIDITrailApp::_SaveSceneType()
{
	int result = 0;
	const TCHAR* pType = _T("");

	//TAG: add scene
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
// Load scene settings
//******************************************************************************
int MIDITrailApp::_LoadSceneConf()
{
	int result = 0;
	int autoSaveViewpoint = 0;

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	//Auto-save viewpoint
	//result = m_ViewConf.GetInt(_T("AutoSaveViewpoint"), &autoSaveViewpoint, 0);
	//if (result != 0) goto EXIT;
	//
	//m_isAutoSaveViewpoint = false;
	//if (autoSaveViewpoint == 1) {
	//	m_isAutoSaveViewpoint = true;
	//}

	//Auto-save viewpoint: always enabled
	m_isAutoSaveViewpoint = true;

EXIT:;
	return result;
}

//******************************************************************************
// Save scene settings
//******************************************************************************
int MIDITrailApp::_SaveSceneConf()
{
	int result = 0;
	int autoSaveViewpoint = 0;

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	//Auto-save viewpoint
	autoSaveViewpoint = m_isAutoSaveViewpoint ? 1 : 0;
	result = m_ViewConf.SetInt(_T("AutoSaveViewpoint"), autoSaveViewpoint);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Load display effect selection state
//******************************************************************************
int MIDITrailApp::_LoadEffectStatus()
{
	int result = 0;
	int value = 0;

	result = m_ViewConf.SetCurSection(_T("Effect"));
	if (result != 0) goto EXIT;

	result = m_ViewConf.GetInt(_T("PianoKeyboard"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnablePianoKeyboard = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("Ripple"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnableRipple = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("PitchBend"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnablePitchBend = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("Stars"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnableStars = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("Counter"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnableCounter = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("BackgroundImage"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnableBackgroundImage = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("GridLine"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnableGridLine = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("TimeIndicator"), &value, 1);
	if (result != 0) goto EXIT;
	m_isEnableTimeIndicator = (value > 0)? true : false;

	result = m_ViewConf.GetInt(_T("DiagOverlay"), &value, 0);
	if (result != 0) goto EXIT;
	m_isEnableDiagOverlay = (value > 0)? true : false;

EXIT:;
	return result;
}

//******************************************************************************
// Save the display effect selection state
//******************************************************************************
int MIDITrailApp::_SaveEffectStatus()
{
	int result = 0;
	int value = 0;

	result = m_ViewConf.SetCurSection(_T("Effect"));
	if (result != 0) goto EXIT;

	value = m_isEnablePianoKeyboard ? 1 : 0;
	result = m_ViewConf.SetInt(_T("PianoKeyboard"), value);
	if (result != 0) goto EXIT;

	value = m_isEnableRipple ? 1 : 0;
	result = m_ViewConf.SetInt(_T("Ripple"), value);
	if (result != 0) goto EXIT;

	value = m_isEnablePitchBend ? 1 : 0;
	result = m_ViewConf.SetInt(_T("PitchBend"), value);
	if (result != 0) goto EXIT;

	value = m_isEnableStars ? 1 : 0;
	result = m_ViewConf.SetInt(_T("Stars"), value);
	if (result != 0) goto EXIT;

	value = m_isEnableCounter ? 1 : 0;
	result = m_ViewConf.SetInt(_T("Counter"), value);
	if (result != 0) goto EXIT;

	value = m_isEnableBackgroundImage ? 1 : 0;
	result = m_ViewConf.SetInt(_T("BackgroundImage"), value);
	if (result != 0) goto EXIT;

	value = m_isEnableGridLine ? 1 : 0;
	result = m_ViewConf.SetInt(_T("GridLine"), value);
	if (result != 0) goto EXIT;

	value = m_isEnableTimeIndicator ? 1 : 0;
	result = m_ViewConf.SetInt(_T("TimeIndicator"), value);
	if (result != 0) goto EXIT;

	value = m_isEnableDiagOverlay ? 1 : 0;
	result = m_ViewConf.SetInt(_T("DiagOverlay"), value);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Load viewpoint
//******************************************************************************
int MIDITrailApp::_LoadViewpoint()
{
	int result = 0;
	MTViewParamMap defParamMap;
	MTViewParamMap viewParamMap;
	MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};
	float param = 0.0f;

	//Get the default viewpoint from the scene
	m_pScene->GetDefaultViewParam(&defParamMap);

	//Section name
	_tcscat_s(section, 256, _T("Viewpoint-"));
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//Get the parameters from the config file
	for (itr = defParamMap.begin(); itr != defParamMap.end(); itr++) {
		result = m_ViewConf.GetFloat((itr->first).c_str(), &param, itr->second);
		if (result != 0) goto EXIT;
		viewParamMap.insert(MTViewParamMapPair((itr->first).c_str(), param));
	}

	//Register the viewpoint with the scene
	m_pScene->SetViewParam(&viewParamMap);

EXIT:;
	return result;
}

//******************************************************************************
// Save viewpoint
//******************************************************************************
int MIDITrailApp::_SaveViewpoint()
{
	int result = 0;
	MTViewParamMap viewParamMap;
	MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};

	//Get the current viewpoint from the scene
	m_pScene->GetViewParam(&viewParamMap);

	//Section name
	_tcscat_s(section, 256, _T("Viewpoint-"));
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//Register the parameters with the config file
	for (itr = viewParamMap.begin(); itr != viewParamMap.end(); itr++) {
		result = m_ViewConf.SetFloat((itr->first).c_str(), itr->second);
		if (result != 0) goto EXIT;
	}

	//Notify the scene that the viewpoint has been switched
	m_pScene->SetViewParam(&viewParamMap);

EXIT:;
	return result;
}

//******************************************************************************
// Move to my viewpoint
//******************************************************************************
int MIDITrailApp::_MoveToMyViewpoint(
		unsigned long viewpointNo
	)
{
	int result = 0;
	MTViewParamMap defParamMap;
	MTViewParamMap viewParamMap;
	MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};
	float param = 0.0f;

	//Get the default viewpoint from the scene
	m_pScene->GetDefaultViewParam(&defParamMap);

	//Section name
	_stprintf_s(section, 256, _T("MyViewpoint-%d-"), viewpointNo);
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//Get the parameters from the config file (apply the default viewpoint if not set)
	for (itr = defParamMap.begin(); itr != defParamMap.end(); itr++) {
		result = m_ViewConf.GetFloat((itr->first).c_str(), &param, itr->second);
		if (result != 0) goto EXIT;
		viewParamMap.insert(MTViewParamMapPair((itr->first).c_str(), param));
	}

	//Notify the scene that the viewpoint has been switched
	m_pScene->SetViewParam(&viewParamMap);

EXIT:;
	return result;
}

//******************************************************************************
// Save my viewpoint
//******************************************************************************
int MIDITrailApp::_SaveMyViewpoint(
		unsigned long viewpointNo
	)
{
	int result = 0;
	MTViewParamMap viewParamMap;
	MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};

	//Get the current viewpoint from the scene
	m_pScene->GetViewParam(&viewParamMap);

	//Section name
	_stprintf_s(section, 256, _T("MyViewpoint-%d-"), viewpointNo);
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//Register the parameters with the config file
	for (itr = viewParamMap.begin(); itr != viewParamMap.end(); itr++) {
		result = m_ViewConf.SetFloat((itr->first).c_str(), itr->second);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Load graphics settings
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

	//An invalid value turns antialiasing OFF
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
// Load player settings
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
	//Player control
	//----------------------------------
	result = confFile.SetCurSection("PlayerControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("AllowMultipleInstances", &m_AllowMultipleInstances, 0);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("AutoPlaybackAfterOpenFile", &m_AutoPlaybackAfterOpenFile, 0);
	if (result != 0) goto EXIT;

	//----------------------------------
	//Display control
	//----------------------------------
	result = confFile.SetCurSection("ViewControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("ShowFileName", &showFileName, 0);
	if (result != 0) goto EXIT;
	m_isEnableFileName = (showFileName > 0) ? true : false;

	//----------------------------------
	//Rewind/skip control
	//----------------------------------
	result = confFile.SetCurSection("SkipControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SkipBackTimeSpanInMsec", &m_SkipBackTimeSpanInMsec, 10000);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SkipForwardTimeSpanInMsec", &m_SkipForwardTimeSpanInMsec, 10000);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("MovingTimeSpanInMsec", &timeSpan, 400);
	if (result != 0) goto EXIT;

	//Set the rewind/skip move time on the sequencer
	m_Sequencer.SetMovingTimeSpanInMsec(timeSpan);

	//----------------------------------
	//Playback speed control
	//----------------------------------
	result = confFile.SetCurSection("PlaybackSpeedControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SpeedStepInPercent", &speedStepInPercent, 1);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("MaxSpeedInPercent", &maxSpeedInPercent, 400);
	if (result != 0) goto EXIT;

	m_SpeedStepInPercent = (unsigned long)speedStepInPercent;
	m_MaxSpeedInPercent = (unsigned long)maxSpeedInPercent;

	//----------------------------------
	//Playback control
	//----------------------------------
	result = confFile.SetCurSection("Playback");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("DelayBetweenSongsInMsec", &m_DelayBetweenSongsInMsec, 0);
	if (result != 0) goto EXIT;
	if (m_DelayBetweenSongsInMsec < 0) {
		m_DelayBetweenSongsInMsec = 0;
	}
	else if (m_DelayBetweenSongsInMsec > 10000) {
		m_DelayBetweenSongsInMsec = 10000;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Destroy window
//******************************************************************************
int MIDITrailApp::_OnDestroy()
{
	int result = 0;

	//Save viewpoint
	if (m_isAutoSaveViewpoint) {
		result = _OnMenuSaveViewpoint();
		//if (result != 0) goto EXIT;
		//Continue processing even if an error occurs
	}

	//Stop playback
	if (m_PlayStatus == Play) {
		m_Sequencer.Stop();
		//Ideally we should wait for the sequencer-side thread to finish, but we cut corners here
		Sleep(100);
	}
	else if (m_PlayStatus == MonitorON) {
		m_LiveMonitor.Stop();
		//Strictly we should wait for the callback function to finish, but we cut corners here
		Sleep(100);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// Recreate the scene
//******************************************************************************
int MIDITrailApp::_RebuildScene()
{
	int result = 0;
	int apiresult = 0;
	bool m_isResume = false;
	bool m_isResumeMonitoring = false;
	MTViewParamMap viewParamMap;
	spdlog::critical("Device lost detected, rebuilding scene");

	// DX11: DXGI_ERROR_DEVICE_REMOVED occurs due to a driver crash or similar cause.
	// Unlike DX9, waiting for a device state transition does not recover the device,
	// so we notify the situation via a message box and then fully recreate the device.

	//Save the current viewpoint
	if (m_pScene != NULL) {
		m_pScene->GetViewParam(&viewParamMap);
	}

	//Pause playback
	if (m_PlayStatus == Play) {
		m_Sequencer.Pause();
		m_isResume = true;
	}
	else if (m_PlayStatus == MonitorON) {
		//Monitor off
		result = _OnMenuStopMonitoring();
		if (result != 0) goto EXIT;
		m_isResumeMonitoring = true;
	}

	//Show message box
	apiresult = MessageBox(
					m_hWnd,						//Owner window
					MIDITRAIL_MSG_DEVICELOST,	//Message
					_T("WARNING"),				//Title
					MB_OK | MB_ICONWARNING		//Flags
				);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Recreate the renderer and scene object
	result = _ChangeWindowSize();
	if (result != 0) goto EXIT;

	//Reconfigure the scene
	if (m_pScene != NULL) {
		//Restore the viewpoint
		m_pScene->SetViewParam(&viewParamMap);

		//Notify the scene that playback has started if it is currently playing
		if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
			result = m_pScene->OnPlayStart();
			if (result != 0) goto EXIT;
		}
		//Playback tick time notification
		if (m_SequencerLastMsg.isRecvPlayTime) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.playTime.param1,
							m_SequencerLastMsg.playTime.param2
						);
			if (result != 0) goto EXIT;
		}
		//Tempo change notification
		if (m_SequencerLastMsg.isRecvTempo) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.tempo.param1,
							m_SequencerLastMsg.tempo.param2
						);
			if (result != 0) goto EXIT;
		}
		//Bar number notification
		if (m_SequencerLastMsg.isRecvBar) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.bar.param1,
							m_SequencerLastMsg.bar.param2
						);
			if (result != 0) goto EXIT;
		}
		//Time signature change notification
		if (m_SequencerLastMsg.isRecvBeat) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.beat.param1,
							m_SequencerLastMsg.beat.param2
						);
			if (result != 0) goto EXIT;
		}
	}

	//Resume playback if it was paused
	if (m_isResume) {
		result = m_Sequencer.Resume();
		if (result != 0) goto EXIT;
	}
	else if (m_isResumeMonitoring) {
		//Resume monitoring
		result = _OnMenuStartMonitoring();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Show HowToView
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
		//Show how-to-view dialog
		m_HowToViewDlg.Show(m_hWnd);
	}

	count = 2;
	result = m_ViewConf.SetInt(_T("DispCount"), count);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Update menu selection mark
//******************************************************************************
int MIDITrailApp::_UpdateMenuCheckmark()
{
	int result = 0;

	//Repeat
	_CheckMenuItem(IDM_REPEAT, m_isRepeat);
	
	//Folder playback
	_CheckMenuItem(IDM_FOLDER_PLAYBACK, m_isFolderPlayback);

	//Select scenetype
	//TAG: add scene
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

	//Piano keyboard display
	_CheckMenuItem(IDM_ENABLE_PIANOKEYBOARD, m_isEnablePianoKeyboard);

	//Ripple effect
	_CheckMenuItem(IDM_ENABLE_RIPPLE, m_isEnableRipple);

	//Pitch bend effect
	_CheckMenuItem(IDM_ENABLE_PITCHBEND, m_isEnablePitchBend);

	//Star display
	_CheckMenuItem(IDM_ENABLE_STARS, m_isEnableStars);

	//Show counter
	_CheckMenuItem(IDM_ENABLE_COUNTER, m_isEnableCounter);

	//Background image display
	_CheckMenuItem(IDM_ENABLE_BACKGROUNDIMAGE, m_isEnableBackgroundImage);

	//Auto-save viewpoint has been removed
	//_CheckMenuItem(IDM_AUTO_SAVE_VIEWPOINT, m_isAutoSaveViewpoint);

	//Grid line
	_CheckMenuItem(IDM_ENABLE_GRIDLINE, m_isEnableGridLine);

	//Time indicator
	_CheckMenuItem(IDM_ENABLE_TIMEINDICATOR, m_isEnableTimeIndicator);

	//Diagnostic overlay
	_CheckMenuItem(IDM_ENABLE_DIAGOVERLAY, m_isEnableDiagOverlay);

	//Fullscreen
	_CheckMenuItem(IDM_FULLSCREEN, m_isFullScreen);

	//Menu bar
	_CheckMenuItem(IDM_MENUBAR, m_isEnableMenuBar);

EXIT:;
	return result;
}

//******************************************************************************
// Set menu selection mark
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
// Apply display effect
//******************************************************************************
void MIDITrailApp::_UpdateEffect()
{
	if (m_pScene != NULL) {
		m_pScene->SetEffect(MTEffectPianoKeyboard, m_isEnablePianoKeyboard);
		m_pScene->SetEffect(MTEffectRipple, m_isEnableRipple);
		m_pScene->SetEffect(MTEffectPitchBend, m_isEnablePitchBend);
		m_pScene->SetEffect(MTEffectStars, m_isEnableStars);
		m_pScene->SetEffect(MTEffectCounter, m_isEnableCounter);
		m_pScene->SetEffect(MTEffectBackgroundImage, m_isEnableBackgroundImage);
		m_pScene->SetEffect(MTEffectGridBox, m_isEnableGridLine);
		m_pScene->SetEffect(MTEffectTimeIndicator, m_isEnableTimeIndicator);
		m_pScene->SetEffect(MTEffectFileName, m_isEnableFileName);
		m_pScene->SetEffect(MTEffectDiagOverlay, m_isEnableDiagOverlay);
	}
	return;
}

//******************************************************************************
// Parse command line
//******************************************************************************
int MIDITrailApp::_ParseCmdLine()
{
	int result = 0;
	DWORD dwResult = 0;
	WCHAR filePath[_MAX_PATH];

	//Parse command line
	result = m_CmdLineParser.Initialize();
	if (result != 0) goto EXIT;

	//If a file is specified on the command line
	if (m_CmdLineParser.GetSwitch(CMDSW_FILE_PATH) == CMDSW_ON) {
		//Convert to a full path
		dwResult = GetFullPathNameW(
						m_CmdLineParser.GetFilePath(),	//Source file name (relative paths allowed)
						_MAX_PATH,		//Buffer size for the full path
						filePath,		//Buffer for the full path
						NULL			//Pointer to the file name
					);
		if (dwResult == 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}

		//Open the file
		result = _LoadMIDIFile(filePath);
		if (result != 0) goto EXIT;

		//Start playback if specified
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
// Start timer
//******************************************************************************
int MIDITrailApp::_StartTimer()
{
	int result = 0;
	UINT_PTR apiresult = 0;

	//Key state check timer
	apiresult = SetTimer(
						m_hWnd,			//Notification target window
						MIDITRAIL_TIMER_CHECK_KEY,	//Timer IDs
						200,			//Timeout value (milliseconds)
						NULL			//Timer function
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Stop timer
//******************************************************************************
int MIDITrailApp::_StopTimer()
{
	int result = 0;

	KillTimer(m_hWnd, MIDITRAIL_TIMER_CHECK_KEY);
	KillTimer(m_hWnd, MIDITRAIL_TIMER_PLAY);
	KillTimer(m_hWnd, MIDITRAIL_TIMER_OPEN_FILE_AND_PLAY);

	return result;
}

//******************************************************************************
// Start timer: begin playback
//******************************************************************************
int MIDITrailApp::_StartTimer_Play(int delayBetweenSongsInMsec)
{
	int result = 0;
	UINT_PTR apiresult = 0;

	//Register timer
	apiresult = SetTimer(
						m_hWnd,						//Notification target window
						MIDITRAIL_TIMER_PLAY,		//Timer IDs
						delayBetweenSongsInMsec,	//Timeout value (milliseconds)
						NULL						//Timer function
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Start timer: open file and begin playback
//******************************************************************************
int MIDITrailApp::_StartTimer_OpenFileAndPlay(int delayBetweenSongsInMsec)
{
	int result = 0;
	UINT_PTR apiresult = 0;

	//Register timer
	apiresult = SetTimer(
						m_hWnd,						//Notification target window
						MIDITRAIL_TIMER_OPEN_FILE_AND_PLAY,	//Timer IDs
						delayBetweenSongsInMsec,	//Timeout value (milliseconds)
						NULL						//Timer function
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Timer callback
//******************************************************************************
int MIDITrailApp::_OnTimer(
		WPARAM timerId
	)
{
	int result = 0;

	//Key state check timer
	if (timerId == MIDITRAIL_TIMER_CHECK_KEY) {
		//Playback speed control
		if ((GetKeyState(VK_F2) & 0x8000) && (GetForegroundWindow() == m_hWnd)) {
			m_Sequencer.SetPlaybackSpeed(2);  //2x speed
		}
		else {
			m_Sequencer.SetPlaybackSpeed(1);
		}
	}
	//Playback start timer
	else if (timerId == MIDITRAIL_TIMER_PLAY) {
		//Stop timer
		KillTimer(m_hWnd, MIDITRAIL_TIMER_PLAY);
		//Start playback
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}
	//File open & playback start timer
	else if (timerId == MIDITRAIL_TIMER_OPEN_FILE_AND_PLAY) {
		//Stop timer
		KillTimer(m_hWnd, MIDITRAIL_TIMER_OPEN_FILE_AND_PLAY);
		//Open the file
		result = _LoadMIDIFile(m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex()));
		if (result != 0) goto EXIT;
		//Start playback
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Check renderer
//******************************************************************************
int MIDITrailApp::_CheckRenderer()
{
	// DX11: index buffer is always supported.
	return 0;
}

//******************************************************************************
// Auto-configure MIDI OUT
//******************************************************************************
int MIDITrailApp::_AutoConfigMIDIOUT()
{
	int result = 0;
	int apiresult = 0;
	TCHAR devName[MAXPNAMELEN];
	TCHAR message[512];
	int autoConfigConfirm = 0;
	std::string productName;

	//Category/section setting
	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//Get the user-selected MIDI OUT device name from the config file
	result = m_MIDIConf.GetStr("PortA", devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;

	if (_tcslen(devName) == 0) {
		//If not configured
		result = m_MIDIConf.GetInt("AutoConfigConfirm", &autoConfigConfirm, 0);
		if (result != 0) goto EXIT;

		if (autoConfigConfirm == 0) {
			//If auto-configuration has not yet been confirmed, auto-configure the MIDI OUT device
			result = m_MIDIConf.SetInt("AutoConfigConfirm", 1);
			if (result != 0) goto EXIT;

			//Search for Microsoft GS Wavetable Synth
			result = _SearchMicrosoftWavetableSynth(productName);
			if (result != 0) goto EXIT;

			//If found, register it as the MIDI OUT device
			if (productName.size() > 0) {
				result = m_MIDIConf.SetStr("PortA", productName.c_str());
				if (result != 0) goto EXIT;

				//Show the auto-configuration confirmation alert panel
				_stprintf_s(
						message,
						512,
						_T("MIDITrail selected %s to MIDI OUT.\n")
						_T("If you have any other MIDI device, please configure MIDI OUT."),
						productName.c_str()
					);
				apiresult = MessageBox(
								m_hWnd,						//Owner window
								message,					//Message
								_T("INFORMATION"),			//Title
								MB_OK | MB_ICONINFORMATION	//Flags
							);
				if (apiresult == 0) {
					result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
					goto EXIT;
				}
			}
		}
		else {
			//Do nothing since auto-configuration has already been confirmed
		}
	}
	else {
		//If already configured
		result = m_MIDIConf.SetInt("AutoConfigConfirm", 1);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Search for Microsoft GS Wavetable Synth
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

	//MIDI device to search for
	//  Windows XP and earlier    : Microsoft GS Wavetable SW Synth
	//  Windows Vista and later   : Microsoft GS Wavetable Synth

	//String to search for
	target = "Microsoft GS Wavetable";

	result = outDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	productName = "";
	for (index = 0; index < outDevCtrl.GetDevNum(); index++) {
		result = outDevCtrl.GetDevProductName(index, name);
		if (result != 0) goto EXIT;

		pos = name.find(target);
		if (pos != string::npos) {
			//Found
			productName = name;
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Duplicate launch check
//******************************************************************************
int MIDITrailApp::_CheckMultipleInstances(
		 bool* pIsExitApp
	)
{
	int result = 0;
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES secAttribute;

	*pIsExitApp = false;

	//Do nothing if duplicate launches are allowed
	if (m_AllowMultipleInstances > 0) {
		goto EXIT;
	}

	//Initialize security descriptor
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

	//Set a discretionary access control list (DACL) on the security descriptor
	SetSecurityDescriptorDacl(
			&sd,	//Address of the security descriptor
			TRUE,	//DACL present flag
			NULL,	//DACL address: allow all access to the object
			FALSE	//DACL default flag
		);

	//Security attributes
	secAttribute.nLength = sizeof(SECURITY_ATTRIBUTES);	//Structure size
	secAttribute.lpSecurityDescriptor = &sd;			//Security descriptor
	secAttribute.bInheritHandle = TRUE; 				//Inheritance flag

	//Build mutex
	//  Specify security attributes because mutex creation fails
	//  when "Run as different user" is selected
	m_hAppMutex = CreateMutex(
						&secAttribute,	//Security attributes
						FALSE,			//Do not take ownership of the object
						MIDITRAIL_MUTEX	//Object name
					);
	if (m_hAppMutex == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	else if (GetLastError() ==  ERROR_ALREADY_EXISTS) {
		//If it already exists
		CloseHandle(m_hAppMutex);
		m_hAppMutex = NULL;
		*pIsExitApp = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Build mailslot
//******************************************************************************
int MIDITrailApp::_CreateMailSlot()
{
	int result = 0;

	//Do nothing if duplicate launches are allowed
	if (m_AllowMultipleInstances > 0) {
		goto EXIT;
	}

	//Build mailslot
	m_hMailSlot = CreateMailslot(
						MIDITRAIL_MAILSLOT,	//Mailslot name
						1024,				//Maximum message size (bytes): no limit
						0,					//Read timeout (ms): return control immediately if there is no message
						NULL				//Inheritance options
					);
	if (m_hMailSlot == INVALID_HANDLE_VALUE) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Post the file path to the already-running MIDITrail process
//******************************************************************************
int MIDITrailApp::_PostFilePathToFirstMIDITrail()
{
	int result = 0;
	BOOL bresult = FALSE;
	HWND hWnd = NULL;
	HANDLE hFile = NULL;
	size_t size = 0;
	DWORD written = 0;
	WCHAR* pFilePart = NULL;
	WCHAR filePath[_MAX_PATH] = { L'\0' };

	//Search for the already-running MIDITrail window
	hWnd = FindWindowW(
				m_WndClassName,	//Class name
				NULL			//Window name
			);
	if (hWnd == NULL) {
		//Do nothing if the window is not found
		goto EXIT;
	}

	//Bring the already-running MIDITrail window to the foreground
	SetForegroundWindow(hWnd);

	//Parse command line
	result = m_CmdLineParser.Initialize();
	if (result != 0) goto EXIT;

	//Do nothing if no file is specified on the command line
	if (m_CmdLineParser.GetSwitch(CMDSW_FILE_PATH) != CMDSW_ON) {
		goto EXIT;
	}

	//Convert the file path to a full path
	written = GetFullPathNameW(
					m_CmdLineParser.GetFilePath(),	//File path
					_MAX_PATH,		//Buffer size: in TCHAR units
					filePath,		//Buffer address
					&pFilePart		//Position of the file name
				);
	if (written == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	else if (written > _MAX_PATH) {
		result = YN_SET_ERR("File path is too long.", written, 0);
		goto EXIT;
	}

	//Open the mailslot of the already-running process
	hFile = CreateFile(
				MIDITRAIL_MAILSLOT,		//Mailslot name
				GENERIC_WRITE,			//Access type
				FILE_SHARE_READ,		//Share mode
				NULL,					//Security attributes
				OPEN_EXISTING,			//Creation disposition
				FILE_ATTRIBUTE_NORMAL,	//File attributes and flags
				NULL					//Template file handle
			);
	if (hFile == INVALID_HANDLE_VALUE) {
		//Do nothing if the mailslot cannot be opened
		//This can fail depending on the state of the already-running process
		goto EXIT;
	}

	//Write the file path to the mailslot
	//_tcscat_s(filePath, _MAX_PATH, m_CmdLineParser.GetFilePath());
	size = (wcslen(filePath) + 1) * sizeof(WCHAR);
	bresult = WriteFile(
				hFile,		//File handle
				filePath,	//Data buffer
				(DWORD)size,	//Size to write (bytes)
				&written,	//Size written (bytes)
				NULL		//Overlapped structure
			);
	if (!bresult) {
		//Do nothing if the write fails
		//This can fail depending on the state of the already-running process
		goto EXIT;
	}

	//Notify the already-running MIDITrail window of the posted file path
	PostMessage(hWnd, WM_FILEPATH_POSTED, 0, 0);

EXIT:;
	//The window handle must not be closed
	//if (hWnd != NULL) {
	//	CloseHandle(hWnd);
	//}
	if (hFile != NULL) {
		CloseHandle(hFile);
	}
	return result;
}

//******************************************************************************
// Notification of a file path posted from a subsequent process
//******************************************************************************
int MIDITrailApp::_OnFilePathPosted()
{
	int result = 0;
	BOOL bresult = FALSE;
	DWORD nextSize = 0;
	DWORD readSize = 0;
	DWORD count = 0;
	WCHAR filePath[_MAX_PATH + 4];

	ZeroMemory(filePath, sizeof(WCHAR)*(_MAX_PATH + 4));

	//Do nothing if the mailslot does not exist
	if (m_hMailSlot == NULL) goto EXIT;

	//Get mailslotinfo
	bresult = GetMailslotInfo(
					m_hMailSlot,	//Mailslot
					NULL,			//Maximum message size
					&nextSize,		//Next message size
					&count,			//Message count
					NULL			//Read timeout
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Do nothing if there is no message
	if (nextSize == MAILSLOT_NO_MESSAGE) goto EXIT;

	//Check the message size for consistency
	if (nextSize > (sizeof(WCHAR) * (_MAX_PATH))) {
		result = YN_SET_ERR("Program error.", nextSize, 0);
		goto EXIT;
	}

	//Read the message
	bresult = ReadFile(
					m_hMailSlot,	//Mailslot
					filePath,		//Buffer
					nextSize,		//Read size (bytes)
					&readSize,		//Size read (bytes)
					NULL			//Overlapped structure
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Stop playback/monitoring and open file
	result = _StopPlaybackAndOpenFile(filePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Stop playback/monitoring and open a MIDI file
//******************************************************************************
int MIDITrailApp::_StopPlaybackAndOpenFile(
		const WCHAR* pFilePath
	)
{
	int result = 0;

	//How to handle each playback status
	//  NoData     -> Open the file immediately
	//  Stop       -> Open the file immediately
	//  Play       -> Issue a stop request to the sequencer -> open the file after receiving the stop notification
	//  Pause      -> Issue a stop request to the sequencer -> open the file after receiving the stop notification
	//  MonitorOFF -> Open the file immediately
	//  MonitorON  -> Stop monitoring and transition to MonitorOFF -> open the file immediately

	//Save viewpoint
	if (m_isAutoSaveViewpoint) {
		result = _OnMenuSaveViewpoint();
		if (result != 0) goto EXIT;
	}

	//Stop if monitoring is active
	if (m_PlayStatus == MonitorON) {
		result = _OnMenuStopMonitoring();
		if (result != 0) goto EXIT;
		//Already transitioned to MonitorOFF at this point
	}

	//Open the file immediately if stopped
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//File loading process
		result = _FileOpenProc(pFilePath);
		if (result != 0) goto EXIT;
	}
	//If playing, open the file after playback stops
	else if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//Treat as still playing until the playback status notification arrives
		//Do not change the playback status here
		m_Sequencer.Stop();

		//Open the file once the stop completes
		wcscpy_s(m_NextFilePath, _MAX_PATH, pFilePath);
		m_isOpenFileAfterStop = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Stop playback/monitoring and open folder
//******************************************************************************
int MIDITrailApp::_StopPlaybackAndOpenFolder(
		const WCHAR* pFolderPath
	)
{
	int result = 0;
	int apiresult = 0;
	MTFileList midiFileList;
	const WCHAR* pFilePath = NULL;
	
	//Build a list of MIDI data files that exist in the same directory as the specified file
	//Use a temporary list object for the preliminary check
	result = _MakeFileListWithFolder(pFolderPath, &midiFileList);
	if (result != 0) goto EXIT;

	//If no MIDI data files exist, show a message and exit
	if (midiFileList.GetFileCount() == 0) {
		//Show message box
		apiresult = MessageBox(
							m_hWnd,							//Owner window
							MIDITRAIL_MSG_FILE_NOT_FOUND,	//Message
							_T("WARNING"),					//Title
							MB_OK | MB_ICONWARNING			//Flags
						);
		if (apiresult == 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		goto EXIT;
	}

	//Build a list of MIDI data files that exist in the same directory as the specified file
	result = _MakeFileListWithFolder(pFolderPath, &m_MIDIFileList);
	if (result != 0) goto EXIT;

	//Select and open the first file in the file list
	m_MIDIFileList.SelectFirstFile();
	pFilePath = m_MIDIFileList.GetFilePath(m_MIDIFileList.GetSelectedFileIndex());
	result = _StopPlaybackAndOpenFile(pFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI file open processing
//******************************************************************************
int MIDITrailApp::_FileOpenProc(
		const WCHAR* pFilePath
	)
{
	int result = 0;

	//Load MIDIfile
	result = _LoadMIDIFile(pFilePath);
	if (result != 0) goto EXIT;

	//Show HowToView
	result = _DispHowToView();
	if (result != 0) goto EXIT;

	//Start playback if specified
	if (m_AutoPlaybackAfterOpenFile > 0) {
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Toggle fullscreen
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
// Toggle menu bar visibility
//******************************************************************************
int MIDITrailApp::_ToggleMenuBar()
{
	int result = 0;
	
	m_isEnableMenuBar = m_isEnableMenuBar ? false : true;
	
	result = _ChangeWindowSize();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// Show menu
//******************************************************************************
int MIDITrailApp::_ShowMenu()
{
	int result = 0;
	LONG apiresult = 0;
	
	//Show menu bar processing
	if (GetMenu(m_hWnd) == NULL) {
		apiresult = SetMenu(m_hWnd, m_hMenu);
		if (apiresult == 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
	}

	//Update menu selection mark
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//Update menu style
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Hide menu
//******************************************************************************
int MIDITrailApp::_HideMenu()
{
	int result = 0;
	LONG apiresult = 0;

	//Hide menu bar processing
	//Do nothing if the menu bar is already hidden
	if (GetMenu(m_hWnd) != NULL) {
		//The handle obtained via GetMenu is not destroyed
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
// Gamepad operation processing
//******************************************************************************
int MIDITrailApp::_GamePadProc()
{
	int result = 0;

	result = m_GamePadCtrl.UpdateState();
	if (result != 0) goto EXIT;
	
	//_RPTN(_CRT_WARN, "GamePad: %d %d\n", m_GamePadCtrl.DidPressNow_A(), m_GamePadCtrl.DidPressNow_B());

	//Start button pressed
	if (m_GamePadCtrl.DidPressNow_Start()) {
		//Start/pause playback
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}

	//buttonA  pressed
	if (m_GamePadCtrl.DidPressNow_A()) {
		//Start/pause playback
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}
	
	//buttonB  pressed
	if (m_GamePadCtrl.DidPressNow_B()) {
		//Stop playback
		result = _OnMenuStop();
		if (result != 0) goto EXIT;
	}
	
	//Left shoulder pressed
	if (m_GamePadCtrl.DidPressNow_LShoulder()) {
		//Toggle viewpoint
		result = _ChangeViewPoint(-1);
		if (result != 0) goto EXIT;
	}
	
	//Right shoulder pressed
	if (m_GamePadCtrl.DidPressNow_RShoulder()) {
		//Toggle viewpoint
		result = _ChangeViewPoint(+1);
		if (result != 0) goto EXIT;
	}
	
	//Left trigger pressed
	if (m_GamePadCtrl.DidPressNow_LTrigger()) {
		//Playback rewind
		result = _OnMenuSkipBack();
		if (result != 0) goto EXIT;
	}

	//Right trigger pressed
	if (m_GamePadCtrl.DidPressNow_RTrigger()) {
		//Playback skip
		result = _OnMenuSkipForward();
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Toggle viewpoint
//******************************************************************************
int MIDITrailApp::_ChangeViewPoint(int step)
{
	int result = 0;

	//Update the viewpoint number for the gamepad
	m_GamePadViewPointNo += step;

	if (m_GamePadViewPointNo < 0) {
		m_GamePadViewPointNo = 2;
	}
	else if (m_GamePadViewPointNo > 2) {
		m_GamePadViewPointNo = 0;
	}

	//Toggle viewpoint
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

//******************************************************************************
// Build the file list within a folder
//******************************************************************************
int MIDITrailApp::_MakeFileListWithFolder(
		const WCHAR* pFolderPath,
		MTFileList* pFileList
	)
{
	int result = 0;

	if ((pFolderPath == NULL) || (pFileList == NULL)) {
		result = YN_SET_ERR("Program Error.", 0, 0);
		goto EXIT;
	}

	//Build a list of MIDI data files that exist directly under the specified folder
	result = pFileList->MakeFileListWithDirectory(pFolderPath, &m_RcpConv);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


