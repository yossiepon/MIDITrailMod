//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail �A�v���P�[�V�����N���X
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
// �E�B���h�E�v���V�[�W������p�p�����[�^�ݒ�
//******************************************************************************
MIDITrailApp* MIDITrailApp::m_pThis = NULL;

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MIDITrailApp::MIDITrailApp(void)
{
	m_pThis = this;
	m_hInstance = NULL;
	m_hAppMutex = NULL;
	m_hMailSlot = NULL;
	m_isExitApp = false;

	//�E�B���h�E�n
	m_hWnd = NULL;
	m_Accel = NULL;
	m_Title[0] = _T('\0');
	m_WndClassName[0] = _T('\0');
	m_isFullScreen = false;
	m_hMenu = NULL;

	//�����_�����O�n
	m_pScene = NULL;
	m_MultiSampleType = 0;

	//FPS�\���n
	m_PrevTime = 0;
	m_FPSCount = 0;

	//���t���
	m_PlayStatus = NoData;
	m_isRepeat = false;
	m_isRewind = false;
	m_isOpenFileAfterStop = false;
	ZeroMemory(&m_SequencerLastMsg, sizeof(MTSequencerLastMsg));
	m_PlaySpeedRatio = 100;

	//�\�����
	m_isEnablePianoKeyboard = true;
	m_isEnableRipple = true;
	m_isEnablePitchBend = true;
	m_isEnableStars = true;
	m_isEnableCounter = true;
	m_isEnableFileName = false;
	m_isEnableBackgroundImage = true;

	//�V�[�����
	m_SceneType = Title;
	m_SelectedSceneType = PianoRoll3D;

	//�������_�ۑ�
	m_isAutoSaveViewpoint = false;

	//�v���[���[����
	m_AllowMultipleInstances = 0;
	m_AutoPlaybackAfterOpenFile = 0;

	//�����C���h�^�X�L�b�v����
	m_SkipBackTimeSpanInMsec = 10000;
	m_SkipForwardTimeSpanInMsec = 10000;

	//���t�X�s�[�h����
	m_SpeedStepInPercent = 1;
	m_MaxSpeedInPercent = 400;

	//����I�[�v���Ώۃt�@�C���p�X
	m_NextFilePath[0] = _T('\0');

	//�Q�[���p�b�h�p���_�ԍ�
	m_GamePadViewPointNo = 0;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MIDITrailApp::~MIDITrailApp(void)
{
	Terminate();
}

//******************************************************************************
// ������
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

	//�����񏉊���
	LoadString(hInstance, IDS_APP_TITLE, m_Title, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MIDITRAIL, m_WndClassName, MAX_LOADSTRING);

	//�ݒ�t�@�C��������
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//�O���t�B�b�N�ݒ�ǂݍ���
	result = _LoadGraphicConf();
	if (result != 0) goto EXIT;

	//�v���[���[�ݒ�ǂݍ���
	result = _LoadPlayerConf();
	if (result != 0) goto EXIT;

	//��d�N���`�F�b�N
	result = _CheckMultipleInstances(&m_isExitApp);
	if (result != 0) goto EXIT;

	//��d�N���}�~�̏ꍇ
	if (m_isExitApp) {
		_PostFilePathToFirstMIDITrail(pCmdLine);
		goto EXIT;
	}

	//���[���X���b�g�쐬
	result = _CreateMailSlot();
	if (result != 0) goto EXIT;

	//���b�Z�[�W�L���[������
	result = m_MsgQueue.Initialize(10000);
	if (result != 0) goto EXIT;

	//�E�B���h�E�N���X�o�^
	result = _RegisterClass(hInstance);
	if (result != 0) goto EXIT;

	//���C���E�B���h�E����
	result = _CreateWindow(hInstance, nCmdShow);
	if (result != 0) goto EXIT;

	//�A�N�Z�����[�^�e�[�u���ǂݍ���
	m_Accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIDITRAIL));
	if (m_Accel == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hInstance);
		goto EXIT;
	}

	//���t��ԕύX
	result = _ChangePlayStatus(NoData);
	if (result != 0) goto EXIT;

	//�����_��������
	result = m_Renderer.Initialize(m_hWnd, m_MultiSampleType);
	if (result != 0) goto EXIT;

	//�V�[���I�u�W�F�N�g����
	m_SceneType = Title;
	result = _CreateScene(m_SceneType, &m_SeqData);
	if (result != 0) goto EXIT;

	//�V�[����ʓǂݍ���
	result = _LoadSceneType();
	if (result != 0) goto EXIT;

	//�V�[���ݒ�ǂݍ���
	result = _LoadSceneConf();
	if (result != 0) goto EXIT;

	//���j���[�I���}�[�N�X�V
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//RCP�t�@�C���R���o�[�^������
	result = m_RcpConv.Initialize();
	if (result != 0) goto EXIT;

	//�����_���`�F�b�N
	result = _CheckRenderer();
	if (result != 0) goto EXIT;

	//MIDI OUT �����ݒ�
	result = _AutoConfigMIDIOUT();
	if (result != 0) goto EXIT;

	//�R�}���h���C����͂Ǝ��s
	result = _ParseCmdLine(pCmdLine);
	if (result != 0) goto EXIT;

	//�^�C�}�[�J�n
	result = _StartTimer();
	if (result != 0) goto EXIT;

	//�Q�[���p�b�h����F���[�U�C���f�b�N�X0�Œ�
	result = m_GamePadCtrl.Initialize(0);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �I������
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
// ���s
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

	//���b�Z�[�W���[�v
	while (TRUE) {
		isExist = PeekMessage(
						&msg,		//�擾�������b�Z�[�W
						NULL,		//�擾���E�B���h�E�n���h��
						0,			//�擾�Ώۃ��b�Z�[�W�ŏ��l
						0,			//�擾�Ώۃ��b�Z�[�W�ő�l
						PM_REMOVE	//���b�Z�[�W�������@�F�L���[����폜
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
			//�V�[�P���T�[���b�Z�[�W����
			result = _SequencerMsgProc();
			if (result != 0) {
				YN_SHOW_ERR(m_hWnd);
			}

			//�Q�[���p�b�h���쏈��
			result = _GamePadProc();
			if (result != 0) {
				YN_SHOW_ERR(m_hWnd);
			}

			//�E�B���h�E�\����Ԃł̂ݕ`����s��
			GetWindowPlacement(m_hWnd, &wndpl);
			if ((wndpl.showCmd != SW_HIDE) &&
				(wndpl.showCmd != SW_MINIMIZE) &&
				(wndpl.showCmd != SW_SHOWMINIMIZED) &&
				(wndpl.showCmd != SW_SHOWMINNOACTIVE)) {
				//�`��
				result = m_Renderer.RenderScene(m_pScene);
				if (result != 0) {
					if (result == DXRENDERER_ERR_DEVICE_LOST) {
						//�f�o�C�X���X�g
						//�b��I�΍�Ƃ��ăV�[�����Đ�������
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
	//�֐���WM_QUIT���b�Z�[�W���󂯎���Đ���ɏI������ꍇ��
	//wParam�Ɋi�[����Ă���I���R�[�h��Ԃ�
	//���b�Z�[�W���[�v�ɓ���O�ɏI������ꍇ��0��Ԃ�
	return quitCode;
}

//******************************************************************************
// �E�B���h�E�N���X�o�^
//******************************************************************************
int MIDITrailApp::_RegisterClass(
		HINSTANCE hInstance
	)
{
	int result = 0;
	ATOM aresult = 0;
	WNDCLASSEX wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);				//�\���̃T�C�Y
	wcex.style			= CS_HREDRAW | CS_VREDRAW;			//�N���X�X�^�C��
	wcex.lpfnWndProc	= _WndProc;							//�E�B���h�E�v���V�[�W��
	wcex.cbClsExtra		= 0;								//�ǉ����̃T�C�Y
	wcex.cbWndExtra		= 0;								//�ǉ����̃T�C�Y
	wcex.hInstance		= hInstance;						//�A�v���P�[�V�����C���X�^���X�n���h��
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIDITRAIL));
															//�A�C�R�����\�[�X�n���h��
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);		//�J�[�\�����\�[�X�n���h��
	wcex.hbrBackground  = CreateSolidBrush(RGB(0, 0, 0));	//�w�i�p�u���V�n���h���F��
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MIDITRAIL);	//���j���[���\�[�X����
	wcex.lpszClassName	= m_WndClassName;					//�E�B���h�E�N���X����
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
				 											//���A�C�R�����\�[�X�n���h��

	//�ړ���T�C�Y�ύX�ɂ�����E�C���h�E�����̈�̍ĕ`��w��
	// CS_HREDRAW �N���C�A���g�̈�̕����ω������Ƃ��ɍĕ`�悷��
	// CS_VREDRAW �N���C�A���g�̈�̍������ω������Ƃ��ɍĕ`�悷��

	aresult = RegisterClassEx(&wcex);
	if (aresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���C���E�B���h�E����
//******************************************************************************
int MIDITrailApp::_CreateWindow(
		HINSTANCE hInstance,
		int nCmdShow
	)
{
	int result = 0;

	m_hWnd = CreateWindow(
				m_WndClassName,			//�E�B���h�E�N���X��
				m_Title,				//�E�B���h�E��
				MIDITRAIL_WINDOW_STYLE,	//�E�B���h�E�X�^�C��
				CW_USEDEFAULT,			//�E�B���h�E�̉������̈ʒu�F�f�t�H���g
				0,						//�E�B���h�E�̏c�����̈ʒu
				CW_USEDEFAULT,			//�E�B���h�E�̕��F�f�t�H���g
				0,						//�E�B���h�E�̍���
				NULL,					//�e�܂��̓I�[�i�[�̃E�B���h�E�n���h��
				NULL,					//���j���[�n���h���܂��͎q�E�B���h�EID
				hInstance,				//�A�v���P�[�V�����C���X�^���X�n���h��
				NULL					//�E�B���h�E�쐬�f�[�^
			);
	if (m_hWnd == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	
	//���j���[�o�[�\���ؑւ̂��߃E�B���h�E��������Ƀn���h�����擾���Ă���
	m_hMenu = GetMenu(m_hWnd);

	//���[�U�[�ݒ�E�B���h�E�T�C�Y�ύX
	result = _SetWindowSize();
	if (result != 0) goto EXIT;

	//�E�B���h�E�\��
	ShowWindow(m_hWnd, nCmdShow);

	//WM_PAINT�Ăяo�����~�߂�
	ValidateRect(m_hWnd, 0);

	UpdateWindow(m_hWnd);

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y�ύX
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

	//���[�U�I���E�B���h�E�T�C�Y�擾
	result = m_ViewConf.SetCurSection(_T("WindowSize"));
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Width"), &width, 0);
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Height"), &height, 0);
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("ApplyToViewArea"), &applyToViewArea, 0);
	if (result != 0) goto EXIT;

	//����N�����̃E�B���h�E�T�C�Y
	if ((width <= 0) || (height <= 0)) {
		width = 800;
		height = 600;
	}

	//�E�B���h�E�̃T�C�Y
	GetWindowRect(m_hWnd, &wrect);
	ww = wrect.right - wrect.left;
	wh = wrect.bottom - wrect.top;

	//�N���C�A���g�̈�̃T�C�Y
	GetClientRect(m_hWnd, &crect);
	cw = crect.right - crect.left;
	ch = crect.bottom - crect.top;

	//�g�̃T�C�Y
	framew = ww - cw;
	frameh = wh - ch;

	//�`��̈�Ɏw��T�C�Y��K�p����ꍇ
	if (applyToViewArea != 0) {
		width = width + framew;
		height = height + frameh;
	}
	
	//�E�B���h�E�X�^�C���ݒ�
	apiresult = SetWindowLong(m_hWnd, GWL_STYLE, MIDITRAIL_WINDOW_STYLE);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}
	
	//���j���[�o�[�\��
	result = _ShowMenu();
	if (result != 0) goto EXIT;
	
	//�E�B���h�E�T�C�Y�ύX
	bresult = SetWindowPos(
					m_hWnd,			//�E�B���h�E�n���h��
					HWND_TOP,		//�z�u�����FZ�I�[�_�[�擪
					0,				//�������̈ʒu
					0,				//�c�����̈ʒu
					width,			//��
					height,			//����
					SWP_NOMOVE | SWP_FRAMECHANGED | SWP_SHOWWINDOW	//�E�B���h�E�ʒu�w��
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�T�C�Y�ύX�F�t���X�N���[��
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

	//�}�E�X�J�[�\���ʒu���擾
	bresult = GetCursorPos(&mouseCursorPoint);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�}�E�X�J�[�\���̈ʒu�ɊY�����郂�j�^��I��
	hMonitor = MonitorFromPoint(mouseCursorPoint, MONITOR_DEFAULTTONEAREST);

	//���j�^���擾
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	bresult = GetMonitorInfo(hMonitor, &monitorInfo);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hMonitor);
		goto EXIT;
	}

	//�E�B���h�E�c���T�C�Y
	width  = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	//�E�B���h�E�X�^�C���ݒ�
	apiresult = SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

	//���j���[�o�[��\��
	result = _HideMenu();
	if (result != 0) goto EXIT;

	//�E�B���h�E�T�C�Y�ύX
	bresult = SetWindowPos(
					m_hWnd,						//�E�B���h�E�n���h��
					HWND_TOP,					//�z�u�����FZ�I�[�_�[�擪
					monitorInfo.rcMonitor.left,	//�������̈ʒu
					monitorInfo.rcMonitor.top,	//�c�����̈ʒu
					width,						//��
					height,						//����
					SWP_FRAMECHANGED | SWP_SHOWWINDOW	//�E�B���h�E�ʒu�w��
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�t�@�C��������
//******************************************************************************
int MIDITrailApp::_InitConfFile()
{
	int result = 0;
	BOOL bresult = FALSE;
	TCHAR userConfDirPath[_MAX_PATH] = {_T('\0')};
	TCHAR viewConfPath[_MAX_PATH] = {_T('\0')};
	TCHAR midiOutConfPath[_MAX_PATH] = {_T('\0')};
	TCHAR graphicConfPath[_MAX_PATH] = {_T('\0')};

	//���[�U�ݒ�t�@�C���i�[�f�B���N�g���p�X�쐬
	result = YNPathUtil::GetAppDataDirPath(userConfDirPath, _MAX_PATH);
	if (result != 0) goto EXIT;
	_tcscat_s(userConfDirPath, _MAX_PATH, MT_USER_CONFFILE_DIR);

	//���[�U�ݒ�t�@�C���i�[�f�B���N�g���쐬
	bresult = MakeSureDirectoryPathExists(userConfDirPath);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�r���[���ݒ�t�@�C��
	_tcscat_s(viewConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(viewConfPath, _MAX_PATH, MT_USER_CONFFILE_VIEW);
	result = m_ViewConf.Initialize(viewConfPath);
	if (result != 0) goto EXIT;

	//MIDI���ݒ�t�@�C��
	_tcscat_s(midiOutConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(midiOutConfPath, _MAX_PATH, MT_USER_CONFFILE_MIDI);
	result = m_MIDIConf.Initialize(midiOutConfPath);
	if (result != 0) goto EXIT;

	//�O���t�B�b�N���ݒ�t�@�C��
	_tcscat_s(graphicConfPath, _MAX_PATH, userConfDirPath);
	_tcscat_s(graphicConfPath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);
	result = m_GraphicConf.Initialize(graphicConfPath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���C���E�B���h�E�F�E�B���h�E�v���V�[�W��
//******************************************************************************
LRESULT CALLBACK MIDITrailApp::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

//******************************************************************************
// ���C���E�B���h�E�F�E�B���h�E�v���V�[�W���F����
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
					//�t�@�C���I�[�v��
					result = _OnMenuFileOpen();
					if (result != 0) goto EXIT;
					break;
				case IDM_EXIT:
					//�I��
					DestroyWindow(hWnd);
					break;
				case IDM_PLAY:
					//���t�J�n�^�ꎞ��~�^�ĊJ
					result = _OnMenuPlay();
					if (result != 0) goto EXIT;
					break;
				case IDM_STOP:
					//���t��~
					result = _OnMenuStop();
					if (result != 0) goto EXIT;
					break;
				case IDM_REPEAT:
					//���s�[�g
					result = _OnMenuRepeat();
					if (result != 0) goto EXIT;
					break;
				case IDM_SKIP_BACK:
					//�Đ��X�L�b�v�o�b�N
					result = _OnMenuSkipBack();
					if (result != 0) goto EXIT;
					break;
				case IDM_SKIP_FORWARD:
					//�Đ��X�L�b�v�t�H���[�h
					result = _OnMenuSkipForward();
					if (result != 0) goto EXIT;
					break;
				case IDM_PLAY_SPEED_DOWN:
					//�Đ��X�s�[�h�_�E��
					result = _OnMenuPlaySpeedDown();
					if (result != 0) goto EXIT;
					break;
				case IDM_PLAY_SPEED_UP:
					//�Đ��X�s�[�h�A�b�v
					result = _OnMenuPlaySpeedUp();
					if (result != 0) goto EXIT;
					break;
				case IDM_START_MONITORING:
					//���j�^�����O�J�n
					result = _OnMenuStartMonitoring();
					if (result != 0) goto EXIT;
					break;
				case IDM_STOP_MONITORING:
					//���j�^�����O��~
					result = _OnMenuStopMonitoring();
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_3DPIANOROLL:
					//�r���[�ύX�F3D�s�A�m���[��
					result = _OnMenuSelectSceneType(PianoRoll3D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_2DPIANOROLL:
					//�r���[�ύX�F2D�s�A�m���[��
					result = _OnMenuSelectSceneType(PianoRoll2D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRAIN:
					//�r���[�ύX�F�s�A�m���[�����C��
					result = _OnMenuSelectSceneType(PianoRollRain);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRAIN2D:
					//�r���[�ύX�F�s�A�m���[�����C��2D
					result = _OnMenuSelectSceneType(PianoRollRain2D);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEW_PIANOROLLRING:
					//�r���[�ύX�F�s�A�m���[�������O
					result = _OnMenuSelectSceneType(PianoRollRing);
					if (result != 0) goto EXIT;
					break;
				//TAG: �V�[���ǉ�
				case IDM_ENABLE_PIANOKEYBOARD:
					//�\�����ʁF�s�A�m�L�[�{�[�h
					result = _OnMenuEnableEffect(MTScene::EffectPianoKeyboard);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_RIPPLE:
					//�\�����ʁF�g��
					result = _OnMenuEnableEffect(MTScene::EffectRipple);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_PITCHBEND:
					//�\�����ʁF�s�b�`�x���h
					result = _OnMenuEnableEffect(MTScene::EffectPitchBend);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_STARS:
					//�\�����ʁF��
					result = _OnMenuEnableEffect(MTScene::EffectStars);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_COUNTER:
					//�\�����ʁF�J�E���^
					result = _OnMenuEnableEffect(MTScene::EffectCounter);
					if (result != 0) goto EXIT;
					break;
				case IDM_ENABLE_BACKGROUNDIMAGE:
					//�\�����ʁF�w�i�摜
					result = _OnMenuEnableEffect(MTScene::EffectBackgroundImage);
					if (result != 0) goto EXIT;
					break;
				//�������_�ۑ��Ǝ��_�ۑ��͔p�~
				//case IDM_AUTO_SAVE_VIEWPOINT:
				//	//�������_�ۑ�
				//	result = _OnMenuAutoSaveViewpoint();
				//	if (result != 0) goto EXIT;
				//	break;
				//case IDM_SAVE_VIEWPOINT:
				//	//���_�ۑ�
				//	result = _OnMenuSaveViewpoint();
				//	if (result != 0) goto EXIT;
				//	break;
				case IDM_RESET_VIEWPOINT:
					//�ÓI���_1�Ɉړ��i���_���Z�b�g�j
					result = _OnMenuResetViewpoint();
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEWPOINT2:
					//�ÓI���_2�Ɉړ�
					result = _OnMenuViewpoint(2);
					if (result != 0) goto EXIT;
					break;
				case IDM_VIEWPOINT3:
					//�ÓI���_3�Ɉړ�
					result = _OnMenuViewpoint(3);
					if (result != 0) goto EXIT;
					break;
				case IDM_WINDOWSIZE:
					//�E�B���h�E�T�C�Y�ݒ�
					result = _OnMenuWindowSize();
					if (result != 0) goto EXIT;
					break;
				case IDM_FULLSCREEN:
					//�t���X�N���[��
					result = _OnMenuFullScreen();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_MIDIOUT:
					//MIDI�o�̓f�o�C�X�ݒ�
					result = _OnMenuOptionMIDIOUT();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_MIDIIN:
					//MIDI���̓f�o�C�X�ݒ�
					result = _OnMenuOptionMIDIIN();
					if (result != 0) goto EXIT;
					break;
				case IDM_OPTION_GRAPHIC:
					//�O���t�B�b�N�ݒ�
					result = _OnMenuOptionGraphic();
					if (result != 0) goto EXIT;
					break;
				case IDM_HOWTOVIEW:
					//������@�_�C�A���O�\��
					m_HowToViewDlg.Show(m_hWnd);
					break;
				case IDM_MANUAL:
					//�}�j���A���\��
					result = _OnMenuManual();
					if (result != 0) goto EXIT;
					break;
				case IDM_ABOUT:
					//�o�[�W�������_�C�A���O�\��
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
			//�L�[�������b�Z�[�W
			result = _OnKeyDown(wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			//�}�E�X�{�^���������b�Z�[�W
			result = _OnMouseButtonDown(message, wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_MOUSEMOVE:
			result = _OnMouseMove(message, wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_DROPFILES:
			//�t�@�C���h���b�v
			result = _OnDropFiles(wParam, lParam);
			if (result != 0) goto EXIT;
			break;
		case WM_TIMER:
			//�^�C�}�[
			result = _OnTimer(wParam);
			if (result != 0) goto EXIT;
			break;
		case WM_DESTROY:
			//�j��
			result = _OnDestroy();
			//�߂�l�͖�������
			PostQuitMessage(0);
			break;
		case WM_FILEPATH_POSTED:
			//�t�@�C���p�X�|�X�g�ʒm
			result = _OnFilePathPosted();
			if (result != 0) goto EXIT;
			break;
		case WM_SIZE:
			//�E�B���h�E�T�C�Y�ύX
			if (wParam == SIZE_MAXIMIZED) {
				//�ő剻�F�t���X�N���[��
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
// �t�@�C���I�[�v��
//******************************************************************************
int MIDITrailApp::_OnMenuFileOpen()
{
	int result = 0;
	TCHAR filePath[MAX_PATH] = {_T('\0')};
	bool isSelected = false;

	////���t���̓t�@�C���I�[�v�������Ȃ�
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	//�t�@�C���I�[�v��OK
	//}
	//else {
	//	//�t�@�C���I�[�v��NG
	//	goto EXIT;
	//}

	//���t���ł��t�@�C���I�[�v���Ƃ���

	//�t�@�C���I���_�C�A���O�\��
	result = _SelectMIDIFile(filePath, MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//�t�@�C���I�����̏���
	if (isSelected) {
		//�t���X�N���[���Ń��j���[����t�@�C���I�������ꍇ
		//  �V�[�����������ŃN���C�A���g�E�B���h�E�̃T�C�Y���Q�Ƃ��Ă��邽��
		//  �ꎞ�I�ɕ\���������j���[���\���ɖ߂��Ă���
		if (m_isFullScreen) {
			_HideMenu();
		}

		//���t/���j�^��~�ƃt�@�C���I�[�v������
		result = _StopPlaybackAndOpenFile(filePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�Đ��^�ꎞ��~�^�ĊJ
//******************************************************************************
int MIDITrailApp::_OnMenuPlay()
{
	int result = 0;

	if (m_PlayStatus == Stop) {
		//�V�[�P���T������
		result = m_Sequencer.Initialize(&m_MsgQueue);
		if (result != 0) goto EXIT;

		//�V�[�P���T�Ƀ|�[�g����o�^
		result = _SetPortDev(&m_Sequencer);
		if (result != 0) goto EXIT;

		//�V�[�P���T�ɃV�[�P���X�f�[�^��o�^
		result = m_Sequencer.SetSeqData(&m_SeqData);
		if (result != 0) goto EXIT;

		//�����߂�
		if (m_isRewind) {
			m_isRewind = false;
			result = m_pScene->Rewind();
			if (result != 0) goto EXIT;
		}

		//�V�[���ɉ��t�J�n��ʒm
		result = m_pScene->OnPlayStart(m_Renderer.GetDevice());
		if (result != 0) goto EXIT;

		//�ŐV�V�[�P���T���b�Z�[�W�N���A
		ZeroMemory(&m_SequencerLastMsg, sizeof(MTSequencerLastMsg));

		//���t���x
		m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);

		//���t�J�n
		result = m_Sequencer.Play();
		if (result != 0) goto EXIT;

		//���t��ԕύX
		result = _ChangePlayStatus(Play);
		if (result != 0) goto EXIT;
	}
	else if (m_PlayStatus == Play) {
		//���t�ꎞ��~
		m_Sequencer.Pause();

		//���t��ԕύX
		result = _ChangePlayStatus(Pause);
		if (result != 0) goto EXIT;
	}
	else if (m_PlayStatus == Pause) {
		//���t�ĊJ
		result = m_Sequencer.Resume();
		if (result != 0) goto EXIT;

		//���t��ԕύX
		result = _ChangePlayStatus(Play);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F��~
//******************************************************************************
int MIDITrailApp::_OnMenuStop()
{
	int result = 0;

	if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		m_Sequencer.Stop();
		//���t��Ԓʒm���͂��܂ōĐ����Ƃ݂Ȃ�
		//�����ł͉��t��Ԃ�ύX���Ȃ�

		//�I����Ɋ����߂�
		m_isRewind = true;
	}

	return result;
}

//******************************************************************************
// ���j���[�I���F���s�[�g
//******************************************************************************
int MIDITrailApp::_OnMenuRepeat()
{
	int result = 0;

	//���s�[�g�؂�ւ�
	if (m_isRepeat) {
		m_isRepeat = false;
	}
	else {
		m_isRepeat = true;
	}

	//���j���[�I���}�[�N�X�V
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�X�L�b�v�o�b�N
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
// ���j���[�I���F�X�L�b�v�t�H���[�h
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
// ���j���[�I���F�X�s�[�h�_�E��
//******************************************************************************
int MIDITrailApp::_OnMenuPlaySpeedDown()
{
	int result = 0;

	//���t��Ԋm�F
	if ((m_PlayStatus == Stop) || (m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//�ύXOK
	}
	else {
		//�ύXNG
		goto EXIT;
	}

	//���t���x�_�E��
	m_PlaySpeedRatio -= m_SpeedStepInPercent;

	//���~�b�g
	if (m_PlaySpeedRatio < m_SpeedStepInPercent) {
		m_PlaySpeedRatio = m_SpeedStepInPercent;
	}

	//���t���x�ݒ�
	m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�X�s�[�h�A�b�v
//******************************************************************************
int MIDITrailApp::_OnMenuPlaySpeedUp()
{
	int result = 0;

	//���t��Ԋm�F
	if ((m_PlayStatus == Stop) || (m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//�ύXOK
	}
	else {
		//�ύXNG
		goto EXIT;
	}

	//���t���x�A�b�v
	m_PlaySpeedRatio += m_SpeedStepInPercent;

	//���~�b�g 400%
	if (m_PlaySpeedRatio > m_MaxSpeedInPercent) {
		m_PlaySpeedRatio = m_MaxSpeedInPercent;
	}

	//���t���x�ݒ�
	m_Sequencer.SetPlaySpeedRatio(m_PlaySpeedRatio);
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F���C�u���j�^�J�n
//******************************************************************************
int MIDITrailApp::_OnMenuStartMonitoring()
{
	int result = 0;
	
	//���t��Ԋm�F
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//���j�^�J�nOK
	}
	else {
		//���j�^�J�nNG
		goto EXIT;
	}
	
	//�V�[�P���T������
	//  �V�[�P���T�͍Đ��I�����Ƀf�o�C�X���N���[�Y���Ȃ�����
	//  ���������邱�Ƃɂ���ăN���[�Y������
	result = m_Sequencer.Initialize(&m_MsgQueue);
	if (result != 0) goto EXIT;
	
	//���C�u���j�^�p�V�[������
	if (m_PlayStatus != MonitorOFF) {
		//���_�ۑ�
		if (m_isAutoSaveViewpoint) {
			result = _OnMenuSaveViewpoint();
			if (result != 0) goto EXIT;
		}
		
		//�V�[�����
		m_SceneType = m_SelectedSceneType;
		
		//�V�[������
		result = _CreateScene(m_SceneType, NULL);
		if (result != 0) goto EXIT;
	}
	
	//���C�u���j�^������
	result = m_LiveMonitor.Initialize(&m_MsgQueue);
	if (result != 0) goto EXIT;
	result = _SetMonitorPortDev(&m_LiveMonitor, m_pScene);
	if (result != 0) goto EXIT;
	
	//�V�[���ɉ��t�J�n��ʒm
	result = m_pScene->OnPlayStart(m_Renderer.GetDevice());
	if (result != 0) goto EXIT;
	
	//���C�u���j�^�J�n
	result = m_LiveMonitor.Start();
	if (result != 0) goto EXIT;
	
	//���t��ԕύX
	result = _ChangePlayStatus(MonitorON);
	if (result != 0) goto EXIT;
		
EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F���C�u���j�^��~
//******************************************************************************
int MIDITrailApp::_OnMenuStopMonitoring()
{
	int result = 0;
	
	//���t��Ԋm�F
	if (m_PlayStatus == MonitorON) {
		//���j�^�J�nOK
	}
	else {
		//���j�^�J�nNG
		goto EXIT;
	}
	
	//���C�u���j�^��~
	result = m_LiveMonitor.Stop();
	if (result != 0) goto EXIT;
	
	//���t��ԕύX
	result = _ChangePlayStatus(MonitorOFF);
	if (result != 0) goto EXIT;
	
	//�V�[���ɉ��t�I����ʒm
	if (m_pScene != NULL) {
		result = m_pScene->OnPlayEnd(m_Renderer.GetDevice());
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�V�[�����
//******************************************************************************
int MIDITrailApp::_OnMenuSelectSceneType(
		MIDITrailApp::SceneType type
	)
{
	int result = 0;

	//���t��Ԋm�F
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//�V�[���^�C�v�I��OK
	}
	else {
		//�V�[���^�C�v�I��NG
		goto EXIT;
	}

	//�ۑ�
	m_SelectedSceneType = type;
	result = _SaveSceneType();
	if (result != 0) goto EXIT;

	//���j���[�I���}�[�N�X�V
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//��~���̏ꍇ�̓V�[�����č\�z
	if ((m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//���_�ۑ�
		if (m_isAutoSaveViewpoint) {
			result = _OnMenuSaveViewpoint();
			if (result != 0) goto EXIT;
		}

		m_SceneType = m_SelectedSceneType;
		if (m_PlayStatus == Stop) {
			//�v���C���̃V�[����ʐ؂�ւ�
			result = _CreateScene(m_SceneType, &m_SeqData);
			if (result != 0) goto EXIT;
		}
		else {
			//���C�u���j�^�̃V�[����ʐ؂�ւ�
			result = _CreateScene(m_SceneType, NULL);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�������_�ۑ�
//******************************************************************************
int MIDITrailApp::_OnMenuAutoSaveViewpoint()
{
	int result = 0;

	m_isAutoSaveViewpoint = m_isAutoSaveViewpoint ? false : true;

	//���j���[�I���}�[�N�X�V
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//�V�[���ݒ�ۑ�
	result = _SaveSceneConf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�ÓI���_�ړ�
//******************************************************************************
int MIDITrailApp::_OnMenuViewpoint(
		unsigned long viewpointNo
	)
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//�ÓI���_�Ɉړ�
	m_pScene->MoveToStaticViewpoint(viewpointNo);

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F���_���Z�b�g
//******************************************************************************
int MIDITrailApp::_OnMenuResetViewpoint()
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//�V�[���̎��_�����Z�b�g
	m_pScene->ResetViewpoint();

	//���_�ۑ�
	result = _SaveViewpoint();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F���_�ۑ�
//******************************************************************************
int MIDITrailApp::_OnMenuSaveViewpoint()
{
	int result = 0;

	if (m_PlayStatus == NoData) goto EXIT;

	//���_�ۑ�
	result = _SaveViewpoint();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�\�����ʐݒ�
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
// ���j���[�I���F�E�B���h�E�T�C�Y�ύX
//******************************************************************************
int MIDITrailApp::_OnMenuWindowSize()
{
	int result = 0;

	//�ݒ�_�C�A���O�\��
	result = m_WindowSizeCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//�ύX���ꂽ�ꍇ�̓E�B���h�E�T�C�Y���X�V
	if (m_WindowSizeCfgDlg.IsChanged()) {
		result = _ChangeWindowSize();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�t���X�N���[��
//******************************************************************************
int MIDITrailApp::_OnMenuFullScreen()
{
	int result = 0;

	//�t���X�N���[���ؑ�
	result = _ToggleFullScreen();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���FMIDI�o�̓f�o�C�X�ݒ�
//******************************************************************************
int MIDITrailApp::_OnMenuOptionMIDIOUT()
{
	int result = 0;

	//�ݒ�_�C�A���O�\��
	result = m_MIDIOUTCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// ���j���[�I���FMIDI���̓f�o�C�X�ݒ�
//******************************************************************************
int MIDITrailApp::_OnMenuOptionMIDIIN()
{
	int result = 0;

	//�ݒ�_�C�A���O�\��
	result = m_MIDIINCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���F�O���t�B�b�N�ݒ�
//******************************************************************************
int MIDITrailApp::_OnMenuOptionGraphic()
{
	int result = 0;
	unsigned long multiSampleType = 0;
	bool isSupport = false;

	//�A���`�G�C���A�X�T�|�[�g�����_�C�A���O�ɐݒ�
	for (multiSampleType = DX_MULTI_SAMPLE_TYPE_MIN; multiSampleType <= DX_MULTI_SAMPLE_TYPE_MAX; multiSampleType++) {
		result = m_Renderer.IsSupportAntialias(multiSampleType, &isSupport);
		if (result != 0) goto EXIT;
		m_GraphicCfgDlg.SetAntialiasSupport(multiSampleType, isSupport);
	}

	//�ݒ�_�C�A���O�\��
	result = m_GraphicCfgDlg.Show(m_hWnd);
	if (result != 0) goto EXIT;

	//�ύX���ꂽ�ꍇ�̓����_���ƃV�[���I�u�W�F�N�g���Đ���
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
// �}�j���A���\��
//******************************************************************************
int MIDITrailApp::_OnMenuManual()
{
	int result = 0;
	HINSTANCE hresult = 0;
	TCHAR manualPath[_MAX_PATH] = {_T('\0')};

	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	result = YNPathUtil::GetModuleDirPath(manualPath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//�}�j���A���t�@�C���p�X�쐬
	_tcscat_s(manualPath, _MAX_PATH, MT_MANUALFILE);

	//�}�j���A���t�@�C�����J��
	hresult = ShellExecute(
					NULL,			//�e�E�B���h�E�n���h��
					_T("open"),		//����
					manualPath,		//����Ώۂ̃t�@�C��
					NULL,			//����p�����[�^
					NULL,			//����f�B���N�g��
					SW_SHOWNORMAL	//�\�����
				);
	if (hresult <= (HINSTANCE)32) {
		result = YN_SET_ERR("File open error.", (DWORD64)hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �V�[�P���T���b�Z�[�W����
//******************************************************************************
int MIDITrailApp::_SequencerMsgProc()
{
	int result = 0;
	bool isExist = false;
	unsigned long param1 = 0;
	unsigned long param2 = 0;
	SMMsgParser parser;
	
	while (true) {
		//���b�Z�[�W���o��
		result = m_MsgQueue.GetMessage(&isExist, &param1, &param2);
		if (result != 0) goto EXIT;
		
		//���b�Z�[�W���Ȃ���ΏI��
		if (!isExist) break;
		
		//�V�[�P���T���b�Z�[�W��M����
		result = _OnRecvSequencerMsg(param1, param2);
		if (result != 0) goto EXIT;	
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �V�[�P���T���b�Z�[�W��M
//******************************************************************************
int MIDITrailApp::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	//�V�[���ɃV�[�P���T���b�Z�[�W��n��
	if (m_pScene != NULL) {
		result = m_pScene->OnRecvSequencerMsg(param1, param2);
		if (result != 0) goto EXIT;
	}

	//���t��ԕύX�ʒm�ւ̑Ή�
	parser.Parse(param1, param2);
	if (parser.GetMsg() == SMMsgParser::MsgPlayStatus) {
		//�ꎞ��~
		if (parser.GetPlayStatus() == SMMsgParser::StatusPause) {
			result = _ChangePlayStatus(Pause);
			if (result != 0) goto EXIT;
		}
		//��~�i���t�I���j
		if (parser.GetPlayStatus() == SMMsgParser::StatusStop) {
			result = _ChangePlayStatus(Stop);
			if (result != 0) goto EXIT;

			//�V�[���ɉ��t�I����ʒm
			if (m_pScene != NULL) {
				result = m_pScene->OnPlayEnd(m_Renderer.GetDevice());
				if (result != 0) goto EXIT;
			}

			//���_�ۑ�
			if (m_isAutoSaveViewpoint) {
				result = _OnMenuSaveViewpoint();
				if (result != 0) goto EXIT;
			}

			//���[�U�[�̗v���ɂ���Ē�~�����ꍇ�͊����߂�
			if ((m_isRewind) && (m_pScene != NULL)) {
				m_isRewind = false;
				result = m_pScene->Rewind();
				if (result != 0) goto EXIT;
			}
			//��~��̃t�@�C���I�[�v�����w�肳��Ă���ꍇ
			else if ((m_isOpenFileAfterStop) && (m_pScene != NULL)) {
				m_isOpenFileAfterStop = false;
				//�t�@�C���ǂݍ��ݏ���
				result = _FileOpenProc(m_NextFilePath);
				if (result != 0) goto EXIT;
			}
			//�ʏ�̉��t�I���̏ꍇ�͎���̉��t���Ɋ����߂�
			else {
				m_isRewind = true;
				//���s�[�g�L���Ȃ�Đ��J�n
				if (m_isRepeat) {
					result = _OnMenuPlay();
					if (result != 0) goto EXIT;
				}
			}

			//�R�}���h���C���ŏI���w�肳��Ă���ꍇ
			if (m_CmdLineParser.GetSwitch(CMDSW_QUIET) == CMDSW_ON) {
				DestroyWindow(m_hWnd);
			}
		}
	}

	//�f�o�C�X���X�g�΍�
	//�V�[���ɓn�����ŐV���b�Z�[�W���L�^���Ă���
	if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		//���t�`�b�N�^�C���ʒm
		m_SequencerLastMsg.isRecvPlayTime = true;
		m_SequencerLastMsg.playTime.param1 = param1;
		m_SequencerLastMsg.playTime.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		//�e���|�ύX�ʒm
		m_SequencerLastMsg.isRecvTempo = true;
		m_SequencerLastMsg.tempo.param1 = param1;
		m_SequencerLastMsg.tempo.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		//���ߔԍ��ʒm
		m_SequencerLastMsg.isRecvBar = true;
		m_SequencerLastMsg.bar.param1 = param1;
		m_SequencerLastMsg.bar.param2 = param2;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		//���q�L���ύX�ʒm
		m_SequencerLastMsg.isRecvBeat = true;
		m_SequencerLastMsg.beat.param1 = param1;
		m_SequencerLastMsg.beat.param2 = param2;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�N���b�N�C�x���g
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
// �}�E�X�ړ��C�x���g
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
	
	//�t���X�N���[���̏ꍇ
	if (m_isFullScreen) {
		//�}�E�X�J�[�\�����X�N���[����[�Ɉړ������ꍇ
		if (point.y == 0) {
			//���j���[�o�[�\��
			result = _ShowMenu();
			if (result != 0) goto EXIT;
		}
		else {
			//���j���[�o�[��\��
			result = _HideMenu();
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �L�[���̓C�x���g
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
				//���j�^�����O�J�n
				result = _OnMenuStartMonitoring();
				if (result != 0) goto EXIT;
			}
			else {
				//���t�J�n�^�ꎞ��~
				result = _OnMenuPlay();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_ESCAPE:
			if (m_PlayStatus == MonitorON) {
				//���j�^�����O��~
				result = _OnMenuStopMonitoring();
				if (result != 0) goto EXIT;
			}
			else {
				//���t��~
				result = _OnMenuStop();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_RETURN:
			//���t��~�F�e���L�[��ENTER�ł���NUMLOCK�I���̏ꍇ
			if ((HIWORD((DWORD)lParam) & KF_EXTENDED) && (GetKeyState(VK_NUMLOCK) & 0x01)) {
				result = _OnMenuStop();
				if (result != 0) goto EXIT;
			}
			break;
		case '1':
		case VK_NUMPAD1:
			//�Đ��X�L�b�v�o�b�N
			result = _OnMenuSkipBack();
			if (result != 0) goto EXIT;
			break;
		case '2':
		case VK_NUMPAD2:
			//�Đ��X�L�b�v�t�H���[�h
			result = _OnMenuSkipForward();
			if (result != 0) goto EXIT;
			break;
		case '4':
		case VK_NUMPAD4:
			//�Đ��X�s�[�h�_�E��
			result = _OnMenuPlaySpeedDown();
			if (result != 0) goto EXIT;
			break;
		case '5':
		case VK_NUMPAD5:
			//�Đ��X�s�[�h�A�b�v
			result = _OnMenuPlaySpeedUp();
			if (result != 0) goto EXIT;
			break;
		case '7':
		case VK_NUMPAD7:
			//���_���Z�b�g
			result = _OnMenuResetViewpoint();
			if (result != 0) goto EXIT;
			break;
		case '8':
		case VK_NUMPAD8:
			//�ÓI���_2�ړ�
			result = _OnMenuViewpoint(2);
			if (result != 0) goto EXIT;
			break;
		case '9':
		case VK_NUMPAD9:
			//�ÓI���_3�ړ�
			result = _OnMenuViewpoint(3);
			if (result != 0) goto EXIT;
			break;
		case 'O':
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				//�t�@�C���I�[�v��
				result = _OnMenuFileOpen();
				if (result != 0) goto EXIT;
			}
			break;
		case VK_F11:
			//�t���X�N���[��
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
// �t�@�C���h���b�v�C�x���g
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

	////��~���łȂ���΃t�@�C���h���b�v�͖�������
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	//�t�@�C���h���b�vOK
	//}
	//else {
	//	//�t�@�C���h���b�vNG
	//	goto EXIT;
	//}

	//��Ƀt�@�C���h���b�v��������

	hDrop = (HDROP)wParam;

	//�t�@�C�����m�F
	fileNum = DragQueryFile(
					hDrop,		//wParam
					0xFFFFFFFF,	//�t�@�C���C���f�b�N�X
					NULL,		//�t�@�C�����擾�o�b�t�@
					0			//�o�b�t�@�T�C�Y
				);

	//�����t�@�C���̏ꍇ�͖�������
	if (fileNum != 1) goto EXIT;

	//�t�@�C���p�X�擾
	charNum = DragQueryFile(
					hDrop,		//wParam
					0,			//�t�@�C���C���f�b�N�X
					path,		//�t�@�C�����擾�o�b�t�@
					_MAX_PATH	//�o�b�t�@�T�C�Y
				);
	if (charNum == 0) {
		result = YN_SET_ERR("Windows API error.", wParam, lParam);
		goto EXIT;
	}

	//�t�@�C���g���q�̊m�F
	if (YNPathUtil::IsFileExtMatch(path, _T(".mid"))) {
		isMIDIDataFile = true;
	}
	//rcpcv.dll���L���Ȃ�T�|�[�g�Ώۃt�@�C���ł��邩�ǉ��m�F����
	else if (m_RcpConv.IsAvailable() && m_RcpConv.IsSupportFileExt(path)) {
		isMIDIDataFile = true;
	}

	//�T�|�[�g�Ώۃt�@�C���łȂ���Ή������Ȃ�
	if (!isMIDIDataFile) goto EXIT;

	//���t/���j�^��~�ƃt�@�C���I�[�v������
	result = _StopPlaybackAndOpenFile(path);
	if (result != 0) goto EXIT;

EXIT:;
	if (hDrop != NULL) {
		DragFinish(hDrop);
	}
	return result;
}

//******************************************************************************
// �t�@�C���I��
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

	//rcpcv.dll���L���Ȃ�t�@�C���t�B���^��ύX����
	if (m_RcpConv.IsAvailable()) {
		ofn.lpstrFilter = m_RcpConv.GetOpenFileNameFilter();
	}

	//�t�@�C���I���_�C�A���O�\��
	apiresult = GetOpenFileName(&ofn);
	if (!apiresult) {
		//�L�����Z���܂��̓G���[�����F�G���[�̓`�F�b�N���Ȃ�
		*pIsSelected = false;
		goto EXIT;
	}

	*pIsSelected = true;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�t�@�C���ǂݍ���
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

	//�g���q��*.mid�̏ꍇ
	if (YNPathUtil::IsFileExtMatch(pFilePath, _T(".mid"))) {
		pPath = (TCHAR*)pFilePath;
	}
	//�g���q��*.mid�ȊO�̏ꍇ
	else {
		//���R���|�[�U�̃f�[�^�t�@�C���Ƃ݂Ȃ���SMF�ɕϊ�����
		result = YNPathUtil::GetTempFilePath(smfTempPath, _MAX_PATH, _T("RCP"));
		if (result != 0) goto EXIT;
		result = m_RcpConv.Convert(pFilePath, smfTempPath);
		if (result != 0) goto EXIT;
		pPath = smfTempPath;
	}

	//�f�o�b�O���[�h�ł����MIDI�t�@�C����͌��ʂ��_���v����
	if (m_CmdLineParser.GetSwitch(CMDSW_DEBUG) == CMDSW_ON) {
		_tcscat_s(smfDumpPath, _MAX_PATH, pPath);
		_tcscat_s(smfDumpPath, _MAX_PATH, _T(".dump.txt"));
		smfReader.SetLogPath(smfDumpPath);
	}

	//�t�@�C���ǂݍ���
	result = smfReader.Load(pPath, &m_SeqData);
	if (result != 0) goto EXIT;

	//�t�@�C���ǂݍ��ݎ��ɍĐ��X�s�[�h��100%�ɖ߂��F_CreateScene�ŃJ�E���^�ɔ��f
	m_PlaySpeedRatio = 100;

	//�V�[���I�u�W�F�N�g����
	m_SceneType = m_SelectedSceneType;
	result = _CreateScene(m_SceneType, &m_SeqData);
	if (result != 0) goto EXIT;

	//���t��ԕύX
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
// FPS�X�V
//******************************************************************************
void MIDITrailApp::_UpdateFPS()
{
	unsigned long curTime = 0;
	unsigned long diffTime = 0;
	double fps = 0;
	TCHAR title[256];

	curTime = timeGetTime();
	m_FPSCount += 1;

	//1�b���Ƃ�FPS���v�Z
	diffTime = curTime - m_PrevTime;
	if (diffTime > 1000) {

		//FPS
		fps = (double)m_FPSCount / ((double)diffTime / 1000.0f);
		m_PrevTime = curTime;
		m_FPSCount = 0;

		//�E�B���h�E�^�C�g���ɐݒ�
		_stprintf_s(title, 256, _T("%s - FPS:%.1f"), m_Title, fps);
		SetWindowText(m_hWnd, title);
	}

	return;
}

//******************************************************************************
// �|�[�g���o�^
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

	//�ݒ�t�@�C�����烆�[�U�I���f�o�C�X�����擾���ăV�[�P���T�ɓo�^
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
// MIDI IN ���j�^���o�^
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
	//�J�e�S���^�Z�N�V�����ݒ�
	result = m_MIDIConf.SetCurSection(_T("MIDIIN"));
	if (result != 0) goto EXIT;

	//�ݒ�t�@�C�����烆�[�U�I���f�o�C�X�����擾���ăV�[�P���T�ɓo�^
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

	//�V�[���� MIDI IN �f�o�C�X����o�^
	result = pScene->SetParam("MIDI_IN_DEVICE_NAME", devName);
	if (result != 0) goto EXIT;

	//--------------------------------------
	// MIDI OUT (MIDITHRU)
	//--------------------------------------
	//�J�e�S���^�Z�N�V�����ݒ�
	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//�ݒ�t�@�C�����烆�[�U�I���f�o�C�X�����擾���ăV�[�P���T�ɓo�^
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
// �E�B���h�E�T�C�Y�ύX
//******************************************************************************
int MIDITrailApp::_ChangeWindowSize()
{
	int result = 0;
	bool isMonitor = false;

	//���j�^��Ԃ̊m�F
	if ((m_PlayStatus == MonitorOFF) || (m_PlayStatus == MonitorON)) {
		isMonitor = true;
	}

	//�V�[���j��
	if (m_pScene != NULL) {
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}

	//�����_���I��
	m_Renderer.Terminate();

	//���[�U�[�ݒ�E�B���h�E�T�C�Y�ύX
	result = _SetWindowSize();
	if (result != 0) goto EXIT;

	//�����_��������
	result = m_Renderer.Initialize(m_hWnd, m_MultiSampleType);
	if (result != 0) goto EXIT;

	//�V�[���I�u�W�F�N�g����
	if (!isMonitor) {
		//�v���C���̃V�[������
		result = _CreateScene(m_SceneType, &m_SeqData);
		if (result != 0) goto EXIT;
	}
	else {
		//���C�u���j�^�̃V�[������
		result = _CreateScene(m_SceneType, NULL);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���t��ԕύX
//******************************************************************************
int MIDITrailApp::_ChangePlayStatus(
		PlayStatus status
	)
{
	int result = 0;

	//���t��ԕύX
	m_PlayStatus = status;

	////�t�@�C���h���b�N����
	//if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
	//	DragAcceptFiles(m_hWnd, TRUE);
	//}
	//else {
	//	DragAcceptFiles(m_hWnd, FALSE);
	//}

	//��Ƀt�@�C���h���b�O����
	DragAcceptFiles(m_hWnd, TRUE);

	//���j���[�X�^�C���X�V
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�X�^�C���X�V
//******************************************************************************
int MIDITrailApp::_ChangeMenuStyle()
{
	int result = 0;
	unsigned long menuIndex = 0;
	unsigned long statusIndex = 0;
	unsigned long style = 0;

	//���j���[ID�ꗗ
	//TAG:�V�[���ǉ�
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

	//���j���[�X�^�C���ꗗ
	unsigned long menuStyle[MT_MENU_NUM][MT_PLAYSTATUS_NUM] = {
		//�f�[�^��, ��~, �Đ���, �ꎞ��~, ���j�^��~, ���j�^��
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

	//���j���[�X�^�C���X�V
	for (menuIndex = 0; menuIndex < MT_MENU_NUM; menuIndex++) {
		style = menuStyle[menuIndex][statusIndex];
		EnableMenuItem(GetMenu(m_hWnd), menuID[menuIndex], style);
	}

	return result;
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MIDITrailApp::_CreateScene(
		SceneType type,
		SMSeqData* pSeqData  //���C�u���j�^����NULL
	)
{
	int result = 0;

	//�V�[���j��
	if (m_pScene != NULL) {
		m_pScene->Release();
		delete m_pScene;
		m_pScene = NULL;
	}

	//�V�[���I�u�W�F�N�g����
	//TAG:�V�[���ǉ�
	try {
		if (type == Title) {
			m_pScene = new MTSceneTitle();
		}
		else {
			//�v���C���p�V�[������
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
			//���C�u���j�^�p�V�[������
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

	//�V�[���̐���
	result = m_pScene->Create(m_hWnd, m_Renderer.GetDevice(), pSeqData);
	if (result != 0) goto EXIT;

	//�ۑ�����Ă��鎋�_���V�[���ɔ��f����
	if (type != Title) {
		result = _LoadViewpoint();
		if (result != 0) goto EXIT;
	}

	//�\�����ʔ��f
	_UpdateEffect();

	//���t���x�ݒ�
	m_pScene->SetPlaySpeedRatio(m_PlaySpeedRatio);

EXIT:;
	return result;
}

//******************************************************************************
// �V�[����ʓǂݍ���
//******************************************************************************
int MIDITrailApp::_LoadSceneType()
{
	int result = 0;
	TCHAR type[256];

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	result = m_ViewConf.GetStr(_T("Type"), type, 256, _T(""));
	if (result != 0) goto EXIT;

	//TAG:�V�[���ǉ�
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
// �V�[����ʕۑ�
//******************************************************************************
int MIDITrailApp::_SaveSceneType()
{
	int result = 0;
	TCHAR* pType = _T("");

	//TAG:�V�[���ǉ�
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
// �V�[���ݒ�ǂݍ���
//******************************************************************************
int MIDITrailApp::_LoadSceneConf()
{
	int result = 0;
	int autoSaveViewpoint = 0;

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	//�������_�ۑ�
	//result = m_ViewConf.GetInt(_T("AutoSaveViewpoint"), &autoSaveViewpoint, 0);
	//if (result != 0) goto EXIT;
	//
	//m_isAutoSaveViewpoint = false;
	//if (autoSaveViewpoint == 1) {
	//	m_isAutoSaveViewpoint = true;
	//}

	//�������_�ۑ��F��ɗL���Ƃ���
	m_isAutoSaveViewpoint = true;

EXIT:;
	return result;
}

//******************************************************************************
// �V�[���ݒ�ۑ�
//******************************************************************************
int MIDITrailApp::_SaveSceneConf()
{
	int result = 0;
	int autoSaveViewpoint = 0;

	result = m_ViewConf.SetCurSection(_T("Scene"));
	if (result != 0) goto EXIT;

	//�������_�ۑ�
	autoSaveViewpoint = m_isAutoSaveViewpoint ? 1 : 0;
	result = m_ViewConf.SetInt(_T("AutoSaveViewpoint"), autoSaveViewpoint);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���_�ǂݍ���
//******************************************************************************
int MIDITrailApp::_LoadViewpoint()
{
	int result = 0;
	MTScene::MTViewParamMap defParamMap;
	MTScene::MTViewParamMap viewParamMap;
	MTScene::MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};
	float param = 0.0f;

	//�V�[������f�t�H���g�̎��_���擾
	m_pScene->GetDefaultViewParam(&defParamMap);

	//�Z�N�V������
	_tcscat_s(section, 256, _T("Viewpoint-"));
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//�p�����[�^��ݒ�t�@�C������擾
	for (itr = defParamMap.begin(); itr != defParamMap.end(); itr++) {
		result = m_ViewConf.GetFloat((itr->first).c_str(), &param, itr->second);
		if (result != 0) goto EXIT;
		viewParamMap.insert(MTScene::MTViewParamMapPair((itr->first).c_str(), param));
	}

	//�V�[���Ɏ��_��o�^
	m_pScene->SetViewParam(&viewParamMap);

EXIT:;
	return result;
}

//******************************************************************************
// ���_�ۑ�
//******************************************************************************
int MIDITrailApp::_SaveViewpoint()
{
	int result = 0;
	MTScene::MTViewParamMap viewParamMap;
	MTScene::MTViewParamMap::iterator itr;
	TCHAR section[256] = {_T('\0')};

	//�V�[�����猻�݂̎��_���擾
	m_pScene->GetViewParam(&viewParamMap);

	//�Z�N�V������
	_tcscat_s(section, 256, _T("Viewpoint-"));
	_tcscat_s(section, 256, m_pScene->GetName());
	result = m_ViewConf.SetCurSection(section);
	if (result != 0) goto EXIT;

	//�p�����[�^��ݒ�t�@�C���ɓo�^
	for (itr = viewParamMap.begin(); itr != viewParamMap.end(); itr++) {
		result = m_ViewConf.SetFloat((itr->first).c_str(), itr->second);
		if (result != 0) goto EXIT;
	}

	//���_���؂�ւ���ꂽ���Ƃ��V�[���ɓ`�B
	m_pScene->SetViewParam(&viewParamMap);

EXIT:;
	return result;
}

//******************************************************************************
// �O���t�B�b�N�ݒ�ǂݍ���
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

	//�����l�̓A���`�G�C���A�XOFF�ɂ���
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
// �v���[���[�ݒ�ǂݍ���
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
	//�v���[���[����
	//----------------------------------
	result = confFile.SetCurSection("PlayerControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("AllowMultipleInstances", &m_AllowMultipleInstances, 0);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("AutoPlaybackAfterOpenFile", &m_AutoPlaybackAfterOpenFile, 0);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�\������
	//----------------------------------
	result = confFile.SetCurSection("ViewControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("ShowFileName", &showFileName, 0);
	if (result != 0) goto EXIT;
	m_isEnableFileName = (showFileName > 0) ? true : false;

	//----------------------------------
	//�����C���h�^�X�L�b�v����
	//----------------------------------
	result = confFile.SetCurSection("SkipControl");
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SkipBackTimeSpanInMsec", &m_SkipBackTimeSpanInMsec, 10000);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("SkipForwardTimeSpanInMsec", &m_SkipForwardTimeSpanInMsec, 10000);
	if (result != 0) goto EXIT;
	result = confFile.GetInt("MovingTimeSpanInMsec", &timeSpan, 400);
	if (result != 0) goto EXIT;

	//�V�[�P���T�Ƀ����C���h�^�X�L�b�v�ړ����Ԃ�ݒ�
	m_Sequencer.SetMovingTimeSpanInMsec(timeSpan);

	//----------------------------------
	//���t�X�s�[�h����
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
// �E�B���h�E�j��
//******************************************************************************
int MIDITrailApp::_OnDestroy()
{
	int result = 0;

	//���_�ۑ�
	if (m_isAutoSaveViewpoint) {
		result = _OnMenuSaveViewpoint();
		//if (result != 0) goto EXIT;
		//�G���[���������Ă������𑱍s����
	}

	//���t���~�߂�
	if (m_PlayStatus == Play) {
		m_Sequencer.Stop();
		//�V�[�P���T���̃X���b�h�I����҂����킹��ׂ�������𔲂�
		Sleep(100);
	}
	else if (m_PlayStatus == MonitorON) {
		m_LiveMonitor.Stop();
		//�����ɂ̓R�[���o�b�N�֐��I����҂����킹��ׂ�������𔲂�
		Sleep(100);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// �V�[���Đ���
//******************************************************************************
int MIDITrailApp::_RebuildScene()
{
	int result = 0;
	int apiresult = 0;
	bool m_isResume = false;
	bool m_isResumeMonitoring = false;
	MTScene::MTViewParamMap viewParamMap;

	//�b��΍�
	//  ���b�Z�[�W�{�b�N�X��\�����邱�Ƃɂ��
	//  ���[�U�[��OK�{�^���������܂ł̊Ԃ�
	//  �f�o�C�X�����Z�b�g�\��ԂɂȂ邱�Ƃ����҂���

	//���݂̎��_��ޔ�
	if (m_pScene != NULL) {
		m_pScene->GetViewParam(&viewParamMap);
	}

	//���t���ꎞ��~����
	//  �Ȃ����ꎞ��~���Ȃ��ƃf�o�C�X���Đ������Ă�
	//  �f�o�C�X���X�g���畜�A�ł��Ȃ�
	if (m_PlayStatus == Play) {
		m_Sequencer.Pause();
		m_isResume = true;
	}
	else if (m_PlayStatus == MonitorON) {
		//���j�^��~
		result = _OnMenuStopMonitoring();
		if (result != 0) goto EXIT;
		m_isResumeMonitoring = true;
	}

	//���b�Z�[�W�{�b�N�X�\��
	apiresult = MessageBox(
					m_hWnd,						//�I�[�i�[�E�B���h�E
					MIDITRAIL_MSG_DEVICELOST,	//���b�Z�[�W
					_T("WARNING"),				//�^�C�g��
					MB_OK | MB_ICONWARNING		//�t���O
				);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�����_���ƃV�[���I�u�W�F�N�g�̍Đ���
	result = _ChangeWindowSize();
	if (result != 0) goto EXIT;

	//�V�[���̍Đݒ�
	if (m_pScene != NULL) {
		//���_�𕜋A
		m_pScene->SetViewParam(&viewParamMap);

		//���t���̏ꍇ�̓V�[���ɉ��t�J�n��ʒm
		if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
			result = m_pScene->OnPlayStart(m_Renderer.GetDevice());
			if (result != 0) goto EXIT;
		}
		//���t�`�b�N�^�C���ʒm
		if (m_SequencerLastMsg.isRecvPlayTime) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.playTime.param1,
							m_SequencerLastMsg.playTime.param2
						);
			if (result != 0) goto EXIT;
		}
		//�e���|�ύX�ʒm
		if (m_SequencerLastMsg.isRecvTempo) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.tempo.param1,
							m_SequencerLastMsg.tempo.param2
						);
			if (result != 0) goto EXIT;
		}
		//���ߔԍ��ʒm
		if (m_SequencerLastMsg.isRecvBar) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.bar.param1,
							m_SequencerLastMsg.bar.param2
						);
			if (result != 0) goto EXIT;
		}
		//���q�L���ύX�ʒm
		if (m_SequencerLastMsg.isRecvBeat) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.beat.param1,
							m_SequencerLastMsg.beat.param2
						);
			if (result != 0) goto EXIT;
		}
		//TODO: �m�[�g���̃J�E���^�\���������ł��Ă��Ȃ�
		//TODO: �s�b�`�x���h�������ł��Ă��Ȃ�
	}

	//�ꎞ��~�����ꍇ�͉��t���ĊJ������
	if (m_isResume) {
		result = m_Sequencer.Resume();
		if (result != 0) goto EXIT;
	}
	else if (m_isResumeMonitoring) {
		//���j�^�ĊJ
		result = _OnMenuStartMonitoring();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// HowToView�\��
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
		//������@�_�C�A���O�\��
		m_HowToViewDlg.Show(m_hWnd);
	}

	count = 2;
	result = m_ViewConf.SetInt(_T("DispCount"), count);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���}�[�N�X�V
//******************************************************************************
int MIDITrailApp::_UpdateMenuCheckmark()
{
	int result = 0;

	//���s�[�g
	_CheckMenuItem(IDM_REPEAT, m_isRepeat);

	//�V�[����ʑI��
	//TAG:�V�[���ǉ�
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

	//�s�A�m�L�[�{�[�h�\��
	_CheckMenuItem(IDM_ENABLE_PIANOKEYBOARD, m_isEnablePianoKeyboard);

	//�g�����
	_CheckMenuItem(IDM_ENABLE_RIPPLE, m_isEnableRipple);

	//�s�b�`�x���h����
	_CheckMenuItem(IDM_ENABLE_PITCHBEND, m_isEnablePitchBend);

	//���\��
	_CheckMenuItem(IDM_ENABLE_STARS, m_isEnableStars);

	//�J�E���^�\��
	_CheckMenuItem(IDM_ENABLE_COUNTER, m_isEnableCounter);

	//�w�i�摜�\��
	_CheckMenuItem(IDM_ENABLE_BACKGROUNDIMAGE, m_isEnableBackgroundImage);

	//�������_�ۑ�
	_CheckMenuItem(IDM_AUTO_SAVE_VIEWPOINT, m_isAutoSaveViewpoint);

	//�t���X�N���[��
	_CheckMenuItem(IDM_FULLSCREEN, m_isFullScreen);
	
EXIT:;
	return result;
}

//******************************************************************************
// ���j���[�I���}�[�N�ݒ�
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
// �\�����ʔ��f
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
// �R�}���h���C�����
//******************************************************************************
int MIDITrailApp::_ParseCmdLine(
		LPTSTR pCmdLine
	)
{
	int result = 0;

	//�R�}���h���C�����
	result = m_CmdLineParser.Initialize(pCmdLine);
	if (result != 0) goto EXIT;

	//�R�}���h���C���Ńt�@�C�����w�肳��Ă���ꍇ
	if (m_CmdLineParser.GetSwitch(CMDSW_FILE_PATH) == CMDSW_ON) {

		//�t�@�C�����J��
		result = _LoadMIDIFile(m_CmdLineParser.GetFilePath());
		if (result != 0) goto EXIT;

		//�Đ��w�肳��Ă���ꍇ�͍Đ��J�n
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
// �^�C�}�[�J�n
//******************************************************************************
int MIDITrailApp::_StartTimer()
{
	int result = 0;
	UINT_PTR apiresult = 0;

	//�L�[��Ԋm�F�^�C�}�[
	apiresult = SetTimer(
						m_hWnd,			//�ʒm��E�B���h�E
						MIDITRAIL_TIMER_CHECK_KEY,	//�^�C�}�[ID
						200,			//�^�C���A�E�g�l�i�~���b�j
						NULL			//�^�C�}�[�֐�
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �^�C�}�[��~
//******************************************************************************
int MIDITrailApp::_StopTimer()
{
	int result = 0;

	KillTimer(m_hWnd, MIDITRAIL_TIMER_CHECK_KEY);

	return result;
}

//******************************************************************************
// �^�C�}�[�Ăяo��
//******************************************************************************
int MIDITrailApp::_OnTimer(
		WPARAM timerId
	)
{
	int result = 0;

	//�L�[��Ԋm�F�^�C�}�[
	if (timerId == MIDITRAIL_TIMER_CHECK_KEY) {
		//�Đ����x����
		if ((GetKeyState(VK_F2) & 0x8000) && (GetForegroundWindow() == m_hWnd)) {
			m_Sequencer.SetPlaybackSpeed(2);  //2�{��
		}
		else {
			m_Sequencer.SetPlaybackSpeed(1);
		}
	}

	return result;
}

//******************************************************************************
// �����_���`�F�b�N
//******************************************************************************
int MIDITrailApp::_CheckRenderer()
{
	int result = 0;
	bool isSupport = true;
	unsigned long maxVertexIndex = 0;

	result = m_Renderer.IsSupportIndexBuffer(&isSupport, &maxVertexIndex);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@���T�|�[�g���Ă��Ȃ��ꍇ�͌x�����b�Z�[�W��\��
	if (!isSupport) {
		YN_SET_WARN("This PC does not have sufficient graphics capabilities.\n"
					"Therefore, MIDITrail will not work correctly.",
					maxVertexIndex, 0);
		YN_SHOW_ERR(NULL);
		//�߂�l�ɂ͔��f���������𑱍s������
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI OUT �����ݒ�
//******************************************************************************
int MIDITrailApp::_AutoConfigMIDIOUT()
{
	int result = 0;
	int apiresult = 0;
	TCHAR devName[MAXPNAMELEN];
	TCHAR message[512];
	int autoConfigConfirm = 0;
	std::string productName;

	//�J�e�S���^�Z�N�V�����ݒ�
	result = m_MIDIConf.SetCurSection(_T("MIDIOUT"));
	if (result != 0) goto EXIT;

	//�ݒ�t�@�C������ MIDI OUT ���[�U�I���f�o�C�X�����擾
	result = m_MIDIConf.GetStr("PortA", devName, MAXPNAMELEN, _T(""));
	if (result != 0) goto EXIT;

	if (_tcslen(devName) == 0) {
		//�ݒ�Ȃ��̏ꍇ
		result = m_MIDIConf.GetInt("AutoConfigConfirm", &autoConfigConfirm, 0);
		if (result != 0) goto EXIT;

		if (autoConfigConfirm == 0) {
			//�����ݒ薢�m�F�̏ꍇ��MIDI OUT�f�o�C�X�������ݒ肷��
			result = m_MIDIConf.SetInt("AutoConfigConfirm", 1);
			if (result != 0) goto EXIT;

			//Microsoft GS Wavetable Synth������
			result = _SearchMicrosoftWavetableSynth(productName);
			if (result != 0) goto EXIT;

			//���������ꍇ��MIDI OUT�f�o�C�X�ɓo�^����
			if (productName.size() > 0) {
				result = m_MIDIConf.SetStr("PortA", productName.c_str());
				if (result != 0) goto EXIT;

				//�����ݒ�m�F�A���[�g�p�l���\��
				_stprintf_s(
						message,
						512,
						_T("MIDITrail selected %s to MIDI OUT.\n")
						_T("If you have any other MIDI device, please configure MIDI OUT."),
						productName.c_str()
					);
				apiresult = MessageBox(
								m_hWnd,						//�I�[�i�[�E�B���h�E
								message,					//���b�Z�[�W
								_T("INFORMATION"),			//�^�C�g��
								MB_OK | MB_ICONINFORMATION	//�t���O
							);
				if (apiresult == 0) {
					result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
					goto EXIT;
				}
			}
		}
		else {
			//�����ݒ�m�F�ς݂̂��߉������Ȃ�
		}
	}
	else {
		//�ݒ肠��̏ꍇ
		result = m_MIDIConf.SetInt("AutoConfigConfirm", 1);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Microsoft GS Wavetable Synth����
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

	//�����Ώ�MIDI�f�o�C�X
	//  Windows XP�ȑO    : Microsoft GS Wavetable SW Synth
	//  Windows Vista�ȍ~ : Microsoft GS Wavetable Synth

	//�����Ώە�����
	target = "Microsoft GS Wavetable";

	result = outDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	productName = "";
	for (index = 0; index < outDevCtrl.GetDevNum(); index++) {
		result = outDevCtrl.GetDevProductName(index, name);
		if (result != 0) goto EXIT;

		pos = name.find(target);
		if (pos != string::npos) {
			//��������
			productName = name;
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ��d�N���`�F�b�N
//******************************************************************************
int MIDITrailApp::_CheckMultipleInstances(
		 bool* pIsExitApp
	)
{
	int result = 0;
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES secAttribute;

	*pIsExitApp = false;

	//��d�N����������ꍇ�͉������Ȃ�
	if (m_AllowMultipleInstances > 0) {
		goto EXIT;
	}

	//�Z�L�����e�B�L�q�q������
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

	//�Z�L�����e�B�L�q�q�ɐ��ӃA�N�Z�X���䃊�X�g(DACL)��ݒ�
	SetSecurityDescriptorDacl(
			&sd,	//�Z�L�����e�B�L�q�q�̃A�h���X
			TRUE,	//DACL�̑��݃t���O
			NULL,	//DACL�̃A�h���X�F�I�u�W�F�N�g�ւ̂��ׂẴA�N�Z�X������
			FALSE	//DACL�̊���t���O
		);

	//�Z�L�����e�B����
	secAttribute.nLength = sizeof(SECURITY_ATTRIBUTES);	//�\���̃T�C�Y
	secAttribute.lpSecurityDescriptor = &sd;			//�Z�L�����e�B�L�q�q
	secAttribute.bInheritHandle = TRUE; 				//�p���t���O

	//�~���[�e�N�X�쐬
	//  �u�ʂ̃��[�U�[�Ƃ��Ď��s�v��I�������Ƃ��~���[�e�N�X�쐬�����s���邽��
	//  �Z�L�����e�B�������w�肷��
	m_hAppMutex = CreateMutex(
						&secAttribute,	//�Z�L�����e�B����
						FALSE,			//�I�u�W�F�N�g�̏��L�����擾���Ȃ�
						MIDITRAIL_MUTEX	//�I�u�W�F�N�g����
					);
	if (m_hAppMutex == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	else if (GetLastError() ==  ERROR_ALREADY_EXISTS) {
		//���łɑ��݂���ꍇ
		CloseHandle(m_hAppMutex);
		m_hAppMutex = NULL;
		*pIsExitApp = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���[���X���b�g�쐬
//******************************************************************************
int MIDITrailApp::_CreateMailSlot()
{
	int result = 0;

	//��d�N����������ꍇ�͉������Ȃ�
	if (m_AllowMultipleInstances > 0) {
		goto EXIT;
	}

	//���[���X���b�g�쐬
	m_hMailSlot = CreateMailslot(
						MIDITRAIL_MAILSLOT,	//���[���X���b�g����
						1024,				//�ő僁�b�Z�[�W�T�C�Y(byte)�F�����Ȃ�
						0,					//�ǂݎ��^�C���A�E�g�l(ms)�F���b�Z�[�W���Ȃ���Α����ɐ����Ԃ�
						NULL				//�p���I�v�V����
					);
	if (m_hMailSlot == INVALID_HANDLE_VALUE) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ��s�v���Z�X��MIDITrail�փt�@�C���p�X���|�X�g
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

	//��s��MIDITrail�̃E�B���h�E����������
	hWnd = FindWindow(
				m_WndClassName,	//�N���X��
				NULL			//�E�B���h�E��
			);
	if (hWnd == NULL) {
		//�E�B���h�E��������Ȃ��ꍇ�͉������Ȃ�
		goto EXIT;
	}

	//��s��MIDITrail�̃E�B���h�E��O�ʂɈړ�
	SetForegroundWindow(hWnd);

	//�R�}���h���C�����
	result = m_CmdLineParser.Initialize(pCmdLine);
	if (result != 0) goto EXIT;

	//�R�}���h���C���Ńt�@�C�����w�肳��Ă��Ȃ���Ή������Ȃ�
	if (m_CmdLineParser.GetSwitch(CMDSW_FILE_PATH) != CMDSW_ON) {
		goto EXIT;
	}

	//�t�@�C���p�X���t���p�X�ɕϊ�
	written = GetFullPathName(
					m_CmdLineParser.GetFilePath(),	//�t�@�C���p�X
					_MAX_PATH,		//�o�b�t�@�T�C�Y�FTCHAR�P��
					filePath,		//�o�b�t�@�ʒu
					&pFilePart		//�t�@�C�����̈ʒu
				);
	if (written == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	else if (written > _MAX_PATH) {
		result = YN_SET_ERR("File path is too long.", written, 0);
		goto EXIT;
	}

	//��s�N���v���Z�X�̃��[���X���b�g���J��
	hFile = CreateFile(
				MIDITRAIL_MAILSLOT,		//���[���X���b�g����
				GENERIC_WRITE,			//�A�N�Z�X�^�C�v
				FILE_SHARE_READ,		//���L���@
				NULL,					//�Z�L�����e�B����
				OPEN_EXISTING,			//�����w��
				FILE_ATTRIBUTE_NORMAL,	//�t�@�C�������ƃt���O
				NULL					//�e���v���[�g�t�@�C���n���h��
			);
	if (hFile == INVALID_HANDLE_VALUE) {
		//���[���X���b�g���J���Ȃ��ꍇ�͉������Ȃ�
		//��s�v���Z�X�̏�ԂɈˑ����邽�ߎ��s����\���͂���
		goto EXIT;
	}

	//���[���X���b�g�Ƀt�@�C���p�X����������
	//_tcscat_s(filePath, _MAX_PATH, m_CmdLineParser.GetFilePath());
	size = (_tcslen(filePath) + 1) * sizeof(TCHAR);
	bresult = WriteFile(
				hFile,		//�t�@�C���n���h��
				filePath,	//�f�[�^�o�b�t�@
				(DWORD)size,	//�������݃T�C�Y(byte)
				&written,	//�������񂾃T�C�Y(byte)
				NULL		//�I�[�o�[���b�v�\����
			);
	if (!bresult) {
		//�������߂Ȃ������ꍇ�͉������Ȃ�
		//��s�v���Z�X�̏�ԂɈˑ����邽�ߎ��s����\���͂���
		goto EXIT;
	}

	//��s��MIDITrail�̃E�B���h�E�Ƀt�@�C���p�X�|�X�g�ʒm
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
// �㑱�v���Z�X����̃t�@�C���p�X�|�X�g�ʒm
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

	//���[���X���b�g�����݂��Ȃ���Ή������Ȃ�
	if (m_hMailSlot == NULL) goto EXIT;

	//���[���X���b�g����t�@�C���p�X���擾
	bresult = GetMailslotInfo(
					m_hMailSlot,	//���[���X���b�g
					NULL,			//�ő僁�b�Z�[�W�T�C�Y
					&nextSize,		//�����b�Z�[�W�T�C�Y
					&count,			//���b�Z�[�W��
					NULL			//�ǂݎ��^�C���A�E�g����
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//���b�Z�[�W���Ȃ���Ή������Ȃ�
	if (nextSize == MAILSLOT_NO_MESSAGE) goto EXIT;

	//���b�Z�[�W�T�C�Y�̐������`�F�b�N
	if (nextSize > (sizeof(TCHAR)*1024)) {
		result = YN_SET_ERR("Program error.", nextSize, 0);
		goto EXIT;
	}

	//���b�Z�[�W�ǂݍ���
	bresult = ReadFile(
					m_hMailSlot,	//���[���X���b�g
					filePath,		//�o�b�t�@
					nextSize,		//�ǂݎ��T�C�Y(byte)
					&readSize,		//�ǂݎ�����T�C�Y(byte)
					NULL			//�I�[�o�[���b�v�\����
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//���t/���j�^��~�ƃt�@�C���I�[�v������
	result = _StopPlaybackAndOpenFile(filePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���t/���j�^��~��MIDI�t�@�C���I�[�v������
//******************************************************************************
int MIDITrailApp::_StopPlaybackAndOpenFile(
		TCHAR* pFilePath
	)
{
	int result = 0;

	//���t�X�e�[�^�X���Ƃ̑Ή�����
	//  �f�[�^��   �� �����Ƀt�@�C�����J��
	//  ��~       �� �����Ƀt�@�C�����J��
	//  �Đ���     �� �V�[�P���T�ɒ�~�v�����o�� �� ��~�ʒm���󂯂���Ƀt�@�C�����J��
	//  �ꎞ��~   �� �V�[�P���T�ɒ�~�v�����o�� �� ��~�ʒm���󂯂���Ƀt�@�C�����J��
	//  ���j�^��~ �� �����Ƀt�@�C�����J��
	//  ���j�^��   �� ���j�^���~���ă��j�^��~��Ԃ֑J�� �� �����Ƀt�@�C�����J��

	//���_�ۑ�
	if (m_isAutoSaveViewpoint) {
		result = _OnMenuSaveViewpoint();
		if (result != 0) goto EXIT;
	}

	//���j�^���ł���Β�~����
	if (m_PlayStatus == MonitorON) {
		result = _OnMenuStopMonitoring();
		if (result != 0) goto EXIT;
		//���̎��_�Ń��j�^��~�ɑJ�ڍς�
	}

	//��~���ł���΂����Ƀt�@�C�����J��
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//�t�@�C���ǂݍ��ݏ���
		result = _FileOpenProc(pFilePath);
		if (result != 0) goto EXIT;
	}
	//���t���̏ꍇ�͉��t��~��Ƀt�@�C�����J��
	else if ((m_PlayStatus == Play) || (m_PlayStatus == Pause)) {
		//���t��Ԓʒm���͂��܂ōĐ����Ƃ݂Ȃ�
		//�����ł͉��t��Ԃ�ύX���Ȃ�
		m_Sequencer.Stop();
		
		//��~������Ƀt�@�C�����J��
		_tcscpy_s(m_NextFilePath, _MAX_PATH, pFilePath);
		m_isOpenFileAfterStop = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�t�@�C���I�[�v������
//******************************************************************************
int MIDITrailApp::_FileOpenProc(
		TCHAR* pFilePath
	)
{
	int result = 0;

	//MIDI�t�@�C���ǂݍ���
	result = _LoadMIDIFile(pFilePath);
	if (result != 0) goto EXIT;

	//HowToView�\��
	result = _DispHowToView();
	if (result != 0) goto EXIT;

	//�Đ��w�肳��Ă���ꍇ�͍Đ��J�n
	if (m_AutoPlaybackAfterOpenFile > 0) {
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �t���X�N���[���ؑ�
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
// ���j���[�\��
//******************************************************************************
int MIDITrailApp::_ShowMenu()
{
	int result = 0;
	LONG apiresult = 0;
	
	//���j���[�o�[�\������
	if (GetMenu(m_hWnd) == NULL) {
		apiresult = SetMenu(m_hWnd, m_hMenu);
		if (apiresult == 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
	}

	//���j���[�I���}�[�N�X�V
	result = _UpdateMenuCheckmark();
	if (result != 0) goto EXIT;

	//���j���[�X�^�C���X�V
	result = _ChangeMenuStyle();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���j���[��\��
//******************************************************************************
int MIDITrailApp::_HideMenu()
{
	int result = 0;
	LONG apiresult = 0;

	//���j���[�o�[��\������
	//���łɃ��j���[�o�[��\���Ȃ牽�����Ȃ�
	if (GetMenu(m_hWnd) != NULL) {
		//GetMenu�Ŏ擾�����n���h���͔j������Ȃ�
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
// �Q�[���p�b�h���쏈��
//******************************************************************************
int MIDITrailApp::_GamePadProc()
{
	int result = 0;

	result = m_GamePadCtrl.UpdateState();
	if (result != 0) goto EXIT;
	
	//_RPTN(_CRT_WARN, "GamePad: %d %d\n", m_GamePadCtrl.DidPressNow_A(), m_GamePadCtrl.DidPressNow_B());

	//�X�^�[�g ����
	if (m_GamePadCtrl.DidPressNow_Start()) {
		//���t�J�n�^�ꎞ��~
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}

	//�{�^��A ����
	if (m_GamePadCtrl.DidPressNow_A()) {
		//���t�J�n�^�ꎞ��~
		result = _OnMenuPlay();
		if (result != 0) goto EXIT;
	}
	
	//�{�^��B ����
	if (m_GamePadCtrl.DidPressNow_B()) {
		//���t��~
		result = _OnMenuStop();
		if (result != 0) goto EXIT;
	}
	
	//���V�����_�[ ����
	if (m_GamePadCtrl.DidPressNow_LShoulder()) {
		//���_�؂�ւ�
		result = _ChangeViewPoint(-1);
		if (result != 0) goto EXIT;
	}
	
	//�E�V�����_�[ ����
	if (m_GamePadCtrl.DidPressNow_RShoulder()) {
		//���_�؂�ւ�
		result = _ChangeViewPoint(+1);
		if (result != 0) goto EXIT;
	}
	
	//���g���K�[ ����
	if (m_GamePadCtrl.DidPressNow_LTrigger()) {
		//�Đ������C���h
		result = _OnMenuSkipBack();
		if (result != 0) goto EXIT;
	}
	
	//�E�g���K�[ ����
	if (m_GamePadCtrl.DidPressNow_RTrigger()) {
		//�Đ��X�L�b�v
		result = _OnMenuSkipForward();
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ���_�؂�ւ�
//******************************************************************************
int MIDITrailApp::_ChangeViewPoint(int step)
{
	int result = 0;

	//�Q�[���p�b�h�p���_�ԍ��X�V
	m_GamePadViewPointNo += step;

	if (m_GamePadViewPointNo < 0) {
		m_GamePadViewPointNo = 2;
	}
	else if (m_GamePadViewPointNo > 2) {
		m_GamePadViewPointNo = 0;
	}

	//���_�؂�ւ�
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
