//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail �A�v���P�[�V�����N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
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
// >>> modify 20120729 yossiepon begin
#include "MTScenePianoRoll3DMod.h"
#include "MTScenePianoRoll2DMod.h"
// <<< modify 20120729 yossiepon end
#include "MTScenePianoRollRain.h"
#include "MTScenePianoRoll3DLive.h"
#include "MTScenePianoRoll2DLive.h"
#include "MTScenePianoRollRainLive.h"

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

	//�E�B���h�E�n
	m_hWnd = NULL;
	m_Accel = NULL;
	m_Title[0] = _T('\0');
	m_WndClassName[0] = _T('\0');

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
	ZeroMemory(&m_SequencerLastMsg, sizeof(MTSequencerLastMsg));
	m_PlaySpeedRatio = 100;

	//�\�����
	m_isEnablePianoKeyboard = true;
	m_isEnableRipple = true;
	m_isEnablePitchBend = true;
	m_isEnableStars = true;
	m_isEnableCounter = true;

	//�V�[�����
	m_SceneType = Title;
	m_SelectedSceneType = PianoRoll3D;

	//�����C���h�^�X�L�b�v����
	m_SkipBackTimeSpanInMsec = 10000;
	m_SkipForwardTimeSpanInMsec = 10000;

	//���t�X�s�[�h����
	m_SpeedStepInPercent = 1;
	m_MaxSpeedInPercent = 400;
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

	//�E�B���h�E�N���X�o�^
	result = _RegisterClass(hInstance);
	if (result != 0) goto EXIT;

	//���C���E�B���h�E����
	result = _CreateWindow(hInstance, nCmdShow);
	if (result != 0) goto EXIT;

	//�A�N�Z�����[�^�e�[�u���ǂݍ���
	m_Accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIDITRAIL));
	if (m_Accel == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)hInstance);
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
				quitCode = msg.wParam;
				break;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (m_pScene != NULL) {
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
// >>> modify 20120728 yossiepon begin
	// COLOR+WINDOW+1���Ƃ�������ɔ���F�ɂȂ��Ėڗ��̂ŁA�w�i�F�����ɂ��Ă������}����
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH); //(COLOR_WINDOW+1);			//�w�i�p�u���V�n���h��
// <<< modify 20120728 yossiepon end
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

	//���[�U�I���E�B���h�E�T�C�Y�擾
	result = m_ViewConf.SetCurSection(_T("WindowSize"));
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Width"), &width, 0);
	if (result != 0) goto EXIT;
	result = m_ViewConf.GetInt(_T("Height"), &height, 0);
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

	//�t���X�N���[�����[�h�łȂ���Θg���܂߂��T�C�Y�Ƃ���
	//TODO: �t���X�N���[�����[�h�Ή�
	framew = 0;
	frameh = 0;

	//�E�B���h�E�T�C�Y�ύX
	bresult = SetWindowPos(
					m_hWnd,			//�E�B���h�E�n���h��
					HWND_TOP,		//�z�u�����FZ�I�[�_�[�擪
					0,				//�������̈ʒu
					0,				//�c�����̈ʒu
					width + framew,	//��
					height + frameh,//����
					SWP_NOMOVE		//�E�B���h�E�ʒu�w��
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)m_hWnd);
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
// >>> add 20120728 yossiepon begin
				case IDM_ADD_FILE:
					//�t�@�C���ǉ�
					result = _OnMenuFileAdd();
					if (result != 0) goto EXIT;
					break;
// <<< add 20120728 yossiepon end
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
				case IDM_RESET_VIEWPOINT:
					//���_���Z�b�g
					result = _OnMenuResetViewpoint();
					if (result != 0) goto EXIT;
					break;
				case IDM_SAVE_VIEWPOINT:
					//���_�ۑ�
					result = _OnMenuSaveViewpoint();
					if (result != 0) goto EXIT;
					break;
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
				case IDM_WINDOWSIZE:
					//�E�B���h�E�T�C�Y�ݒ�
					result = _OnMenuWindowSize();
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
		case WM_SEQUENCER_MESSAGE:
			//�V�[�P���T���b�Z�[�W
			result = _OnRecvSequencerMsg(wParam, lParam);
			if (result != 0) goto EXIT;
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

	//���t���̓t�@�C���I�[�v�������Ȃ�
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//�t�@�C���I�[�v��OK
	}
	else {
		//�t�@�C���I�[�v��NG
		goto EXIT;
	}

	//�t�@�C���I���_�C�A���O�\��
	result = _SelectMIDIFile(filePath, MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//�t�@�C���I�����̏���
	if (isSelected) {
		//MIDI�t�@�C���ǂݍ��ݏ���
		result = _LoadMIDIFile(filePath);
		if (result != 0) goto EXIT;

		//HowToView�\��
		result = _DispHowToView();
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

// >>> add 20120728 yossiepon begin

//******************************************************************************
// �t�@�C���ǉ�
//******************************************************************************
int MIDITrailApp::_OnMenuFileAdd()
{
	int result = 0;
	TCHAR filePath[MAX_PATH] = {_T('\0')};
	bool isSelected = false;

	//���t���̓t�@�C���I�[�v�������Ȃ�
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//�t�@�C���I�[�v��OK
	}
	else {
		//�t�@�C���I�[�v��NG
		goto EXIT;
	}

	//�t�@�C���I���_�C�A���O�\��
	result = _SelectMIDIFile(filePath, MAX_PATH, &isSelected);
	if (result != 0) goto EXIT;

	//�t�@�C���I�����̏���
	if (isSelected) {
		//MIDI�t�@�C���ǂݍ��ݏ���
		result = _AddMIDIFile(filePath);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

// <<< add 20120728 yossiepon end

//******************************************************************************
// ���j���[�I���F�Đ��^�ꎞ��~�^�ĊJ
//******************************************************************************
int MIDITrailApp::_OnMenuPlay()
{
	int result = 0;

	if (m_PlayStatus == Stop) {
		//�V�[�P���T������
		result = m_Sequencer.Initialize(m_hWnd, WM_SEQUENCER_MESSAGE);
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
	result = m_Sequencer.Initialize(m_hWnd, WM_SEQUENCER_MESSAGE);
	if (result != 0) goto EXIT;
	
	//���C�u���j�^�p�V�[������
	if (m_PlayStatus != MonitorOFF) {
		//�V�[�����
		m_SceneType = m_SelectedSceneType;
		
		//�V�[������
		result = _CreateScene(m_SceneType, NULL);
		if (result != 0) goto EXIT;
	}
	
	//���C�u���j�^������
	result = m_LiveMonitor.Initialize(m_hWnd, WM_SEQUENCER_MESSAGE);
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
// ���j���[�I���F
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
	if (m_WindowSizeCfgDlg.IsCahnged()) {
		result = _ChangeWindowSize();
		if (result != 0) goto EXIT;
	}

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
	if (m_GraphicCfgDlg.IsCahnged()) {
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
		result = YN_SET_ERR("File open error.", (unsigned long)hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �V�[�P���T���b�Z�[�W��M
//******************************************************************************
int MIDITrailApp::_OnRecvSequencerMsg(
		unsigned long wParam,
		unsigned long lParam
	)
{
	int result = 0;
	SMMsgParser parser;

	//�V�[���ɃV�[�P���T���b�Z�[�W��n��
	if (m_pScene != NULL) {
		result = m_pScene->OnRecvSequencerMsg(wParam, lParam);
		if (result != 0) goto EXIT;
	}

	//���t��ԕύX�ʒm�ւ̑Ή�
	parser.Parse(wParam, lParam);
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

			//���[�U�[�̗v���ɂ���Ē�~�����ꍇ�͊����߂�
			if ((m_isRewind) && (m_pScene != NULL)) {
				m_isRewind = false;
				result = m_pScene->Rewind();
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
		m_SequencerLastMsg.playTime.wParam = wParam;
		m_SequencerLastMsg.playTime.lParam = lParam;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		//�e���|�ύX�ʒm
		m_SequencerLastMsg.isRecvTempo = true;
		m_SequencerLastMsg.tempo.wParam = wParam;
		m_SequencerLastMsg.tempo.lParam = lParam;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		//���ߔԍ��ʒm
		m_SequencerLastMsg.isRecvBar = true;
		m_SequencerLastMsg.bar.wParam = wParam;
		m_SequencerLastMsg.bar.lParam = lParam;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		//���q�L���ύX�ʒm
		m_SequencerLastMsg.isRecvBeat = true;
		m_SequencerLastMsg.beat.wParam = wParam;
		m_SequencerLastMsg.beat.lParam = lParam;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �E�B���h�E�N���b�N�C�x���g
//******************************************************************************
int MIDITrailApp::_OnMouseButtonDown(
		unsigned long button,
		unsigned long wParam,
		unsigned long lParam
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
// �L�[���̓C�x���g
//******************************************************************************
int MIDITrailApp::_OnKeyDown(
		unsigned long wParam,
		unsigned long lParam
	)
{
	int result = 0;
	unsigned short keycode = 0;

	keycode = LOWORD(wParam);

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
			if ((HIWORD(lParam) & KF_EXTENDED) && (GetKeyState(VK_NUMLOCK) & 0x01)) {
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
		case 'O':
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				//�t�@�C���I�[�v��
				result = _OnMenuFileOpen();
				if (result != 0) goto EXIT;
			}
			break;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �t�@�C���h���b�v�C�x���g
//******************************************************************************
int MIDITrailApp::_OnDropFiles(
		unsigned long wParam,
		unsigned long lParam
	)
{
	int result = 0;
	UINT fileNum = 0;
	UINT charNum = 0;
	HDROP hDrop = NULL;
	TCHAR path[_MAX_PATH] = {_T('\0')};
	bool isMIDIDataFile = false;

	//��~���łȂ���΃t�@�C���h���b�v�͖�������
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		//�t�@�C���h���b�vOK
	}
	else {
		//�t�@�C���h���b�vNG
		goto EXIT;
	}

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

	if (isMIDIDataFile) {
		//MIDI�t�@�C���ǂݍ���
		result = _LoadMIDIFile(path);
		if (result != 0) goto EXIT;

		//HowToView�\��
		result = _DispHowToView();
		if (result != 0) goto EXIT;
	}

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

// >>> add 20120728 yossiepon begin

//******************************************************************************
// MIDI�t�@�C���ǉ��ǂݍ���
// �t�@�C�����ɁuportX�v���܂܂��ꍇ�AX���|�[�g�ԍ��Ƃ݂Ȃ��ia-Z:�召���ꎋ�j
// �t�@�C�����ɁuchXX�v���܂܂��ꍇ�AXX���`�����l���ԍ��ƌ��Ȃ��i00-99)
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

	//�t�@�C�����ꎞ�V�[�P���X�ɓǂݍ���
	result = smfReader.Load(pPath, &tmpSeqData);
	if (result != 0) goto EXIT;

	//�t�@�C�����Ƀ|�[�g�ԍ����܂܂�Ă���Β��o
	char *pPortNo = strstr(pPath, "port");
	if(pPortNo != NULL) {
		portNo = tolower(*(pPortNo + 4)) - 'a';
	}

	//�t�@�C�����Ƀ`�����l���ԍ����܂܂�Ă���Β��o
	char *pChNo = strstr(pPath, "ch");
	if(pChNo != NULL) {
		char bufChNo[3];
		strncpy_s(bufChNo, 3, pChNo + 2, 2);
		bufChNo[2] = '\0';
		chNo = atoi(bufChNo) - 1;
	}

	//�ꎞ�V�[�P���X���}�[�W
	m_SeqData.AddSequence(tmpSeqData, portNo, chNo);

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

// <<< add 20120728 yossiepon end

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

	//�t�@�C���h���b�N����
	if ((m_PlayStatus == NoData) || (m_PlayStatus == Stop) || (m_PlayStatus == MonitorOFF)) {
		DragAcceptFiles(m_hWnd, TRUE);
	}
	else {
		DragAcceptFiles(m_hWnd, FALSE);
	}

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
		IDM_RESET_VIEWPOINT,
		IDM_SAVE_VIEWPOINT,
		IDM_ENABLE_PIANOKEYBOARD,
		IDM_ENABLE_RIPPLE,
		IDM_ENABLE_PITCHBEND,
		IDM_ENABLE_STARS,
		IDM_ENABLE_COUNTER,
		IDM_WINDOWSIZE,
		IDM_OPTION_MIDIOUT,
		IDM_OPTION_MIDIIN,
		IDM_OPTION_GRAPHIC,
		IDM_HOWTOVIEW,
		IDM_MANUAL,
		IDM_ABOUT
	};

	//���j���[�X�^�C���ꗗ
	unsigned long menuStyle[MT_MENU_NUM][MT_PLAYSTATUS_NUM] = {
		//�f�[�^��, ��~, �Đ���, �ꎞ��~, ���j���[ID, ���j�^��~, ���j�^��
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_OPEN_FILE
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
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_3DPIAMF_GRAYEDROLL
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_2DPIAMF_GRAYEDROLL
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_VIEW_PIAMF_GRAYEDROLLRAIN
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_RESET_VIEWPOINT
		{	MF_GRAYED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_SAVE_VIEWPOINT
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_PIAMF_GRAYEDKEYBOAR
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_RIPPLE
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_PITCHBEND
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_STARS
		{	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED,	MF_ENABLED	},	//IDM_ENABLE_COUNTER
		{	MF_ENABLED,	MF_ENABLED,	MF_GRAYED,	MF_GRAYED,	MF_ENABLED,	MF_GRAYED	},	//IDM_WINDOWSIZE
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
	try {
		if (type == Title) {
			m_pScene = new MTSceneTitle();
		}
		else {
			//�v���C���p�V�[������
			if (pSeqData != NULL) {
				if (type == PianoRoll3D) {
// >>> modify 20120729 yossiepon begin
					m_pScene = new MTScenePianoRoll3DMod();
// <<< modify 20120729 yossiepon end
				}
				else if (type == PianoRoll2D) {
// >>> modify 20120729 yossiepon begin
					m_pScene = new MTScenePianoRoll2DMod();
// <<< modify 20120729 yossiepon end
				}
				else if (type == PianoRollRain) {
					m_pScene = new MTScenePianoRollRain();
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

	if (_tcscmp(type, _T("PianoRoll3D")) == 0) {
		m_SelectedSceneType = PianoRoll3D;
	}
	else if (_tcscmp(type, _T("PianoRoll2D")) == 0) {
		m_SelectedSceneType = PianoRoll2D;
	}
	else if (_tcscmp(type, _T("PianoRollRain")) == 0) {
		m_SelectedSceneType = PianoRollRain;
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

	result = confFile.Initialize("Player");
	if (result != 0) goto EXIT;

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
							m_SequencerLastMsg.playTime.wParam,
							m_SequencerLastMsg.playTime.lParam
						);
			if (result != 0) goto EXIT;
		}
		//�e���|�ύX�ʒm
		if (m_SequencerLastMsg.isRecvTempo) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.tempo.wParam,
							m_SequencerLastMsg.tempo.lParam
						);
			if (result != 0) goto EXIT;
		}
		//���ߔԍ��ʒm
		if (m_SequencerLastMsg.isRecvBar) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.bar.wParam,
							m_SequencerLastMsg.bar.lParam
						);
			if (result != 0) goto EXIT;
		}
		//���q�L���ύX�ʒm
		if (m_SequencerLastMsg.isRecvBeat) {
			result = m_pScene->OnRecvSequencerMsg(
							m_SequencerLastMsg.beat.wParam,
							m_SequencerLastMsg.beat.lParam
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
	_CheckMenuItem(IDM_VIEW_3DPIANOROLL, false);
	_CheckMenuItem(IDM_VIEW_2DPIANOROLL, false);
	_CheckMenuItem(IDM_VIEW_PIANOROLLRAIN, false);
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
		if (m_CmdLineParser.GetSwitch(CMDSW_PLAY) == CMDSW_ON) {
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
	UINT apiresult = 0;

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


