//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail �A�v���P�[�V�����N���X
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
// �p�����[�^��`
//******************************************************************************
#define MAX_LOADSTRING  (100)

//�E�B���h�E�X�^�C��
//  WS_OVERLAPPEDWINDOW ���玟�̃X�^�C�������������
//    WS_THICKFRAME   �T�C�Y�ύX��
#define MIDITRAIL_WINDOW_STYLE  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

//�㑱�N���v���Z�X�̃t�@�C���p�X�|�X�g�ʒm
#define WM_FILEPATH_POSTED  (WM_USER + 100)

//���j���[�X�^�C������
#define MT_MENU_NUM        (32)
#define MT_PLAYSTATUS_NUM  (6)

//�f�o�C�X���X�g�x�����b�Z�[�W
#define MIDITRAIL_MSG_DEVICELOST  _T("Direct3D device is lost.")

//�^�C�}�[ID
#define MIDITRAIL_TIMER_CHECK_KEY  (1)

//��d�N���}�~�p�~���[�e�N�X����
#define MIDITRAIL_MUTEX     _T("yknk.MIDITrail")

//���[���X���b�g����
#define MIDITRAIL_MAILSLOT  _T("\\\\.\\mailslot\\yknk\\MIDITrail")


//******************************************************************************
// MIDITrail �A�v���P�[�V�����N���X
//******************************************************************************
class MIDITrailApp
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MIDITrailApp(void);
	virtual ~MIDITrailApp(void);

	//������
	int Initialize(HINSTANCE hInstance, LPTSTR pCmdLine, int nCmdShow);

	//���s
	int Run();

	//��~
	int Terminate();

private:

	//----------------------------------------------------------------
	//�p�����[�^��`
	//----------------------------------------------------------------
	//���t���
	enum PlayStatus {
		NoData,			//�f�[�^�Ȃ�
		Stop,			//��~���
		Play,			//�Đ���
		Pause,			//�ꎞ��~
		MonitorOFF,		//���j�^��~
		MonitorON		//���j�^��
	};

	//�V�[�����
	//TAG:�V�[���ǉ�
	enum SceneType {
		Title,			//�^�C�g��
		PianoRoll3D,	//�s�A�m���[��3D
		PianoRoll2D,	//�s�A�m���[��2D
		PianoRollRain,	//�s�A�m���[�����C��
		PianoRollRain2D	//�s�A�m���[�����C��2D
	};

	//�V�[�P���T���b�Z�[�W
	typedef struct {
		unsigned long param1;
		unsigned long param2;
	} MTSequencerMsg;

	//�ŐV�V�[�P���T���b�Z�[�W
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
	//�����o��`
	//----------------------------------------------------------------
	//�E�B���h�E�v���V�[�W������p�|�C���^
	static MIDITrailApp* m_pThis;

	//�A�v���P�[�V�����C���X�^���X
	HINSTANCE m_hInstance;

	//�A�v���P�[�V������d�N���}�~����
	HANDLE m_hAppMutex;
	HANDLE m_hMailSlot;
	bool m_isExitApp;

	//�R�}���h���C���p�[�T
	MTCmdLineParser m_CmdLineParser;

	//�E�B���h�E�n
	HWND m_hWnd;
	HACCEL m_Accel;
	TCHAR m_Title[MAX_LOADSTRING];
	TCHAR m_WndClassName[MAX_LOADSTRING];
	bool m_isFullScreen;
	HMENU m_hMenu;

	//�����_�����O�n
	DXRenderer m_Renderer;
	MTScene* m_pScene;
	unsigned long m_MultiSampleType;

	//FPS�\���n
	DWORD m_PrevTime;
	DWORD m_FPSCount;

	//MIDI����n
	SMSeqData m_SeqData;
	SMSequencer m_Sequencer;
	SMRcpConv m_RcpConv;
	SMMsgQueue m_MsgQueue;
	SMLiveMonitor m_LiveMonitor;

	//���t���
	PlayStatus m_PlayStatus;
	bool m_isRepeat;
	bool m_isRewind;
	bool m_isOpenFileAfterStop;
	MTSequencerLastMsg m_SequencerLastMsg;
	unsigned long m_PlaySpeedRatio;

	//�\������
	bool m_isEnablePianoKeyboard;
	bool m_isEnableRipple;
	bool m_isEnablePitchBend;
	bool m_isEnableStars;
	bool m_isEnableCounter;
	bool m_isEnableFileName;
	bool m_isEnableBackgroundImage;

	//�V�[�����
	SceneType m_SceneType;
	SceneType m_SelectedSceneType;

	//�E�B���h�E�T�C�Y�ݒ�_�C�A���O
	MTWindowSizeCfgDlg m_WindowSizeCfgDlg;

	//MIDI OUT�ݒ�_�C�A���O
	MTMIDIOUTCfgDlg m_MIDIOUTCfgDlg;

	//MIDI IN�ݒ�_�C�A���O
	MTMIDIINCfgDlg m_MIDIINCfgDlg;

	//�O���t�B�b�N�ݒ�_�C�A���O
	MTGraphicCfgDlg m_GraphicCfgDlg;

	//������@�_�C�A���O
	MTHowToViewDlg m_HowToViewDlg;

	//�o�[�W�������_�C�A���O
	MTAboutDlg m_AboutDlg;

	//�ݒ�t�@�C��
	YNConfFile m_MIDIConf;
	YNConfFile m_ViewConf;
	YNConfFile m_GraphicConf;

	//�v���[���[����
	int m_AllowMultipleInstances;
	int m_AutoPlaybackAfterOpenFile;

	//�X�L�b�v����
	int m_SkipBackTimeSpanInMsec;
	int m_SkipForwardTimeSpanInMsec;

	//���t�X�s�[�h����
	unsigned long m_SpeedStepInPercent;
	unsigned long m_MaxSpeedInPercent;

	//�������_�ۑ�
	bool m_isAutoSaveViewpoint;

	//����I�[�v���Ώۃt�@�C���p�X
	TCHAR m_NextFilePath[_MAX_PATH];

	//�Q�[���p�b�h����
	MTGamePadCtrl m_GamePadCtrl;

	//�Q�[���p�b�h�p���_�ԍ�
	int m_GamePadViewPointNo;

	//----------------------------------------------------------------
	//���\�b�h��`
	//----------------------------------------------------------------
	//�E�B���h�E����
	int _RegisterClass(HINSTANCE hInstance);
	int _CreateWindow(HINSTANCE hInstance, int nCmdShow);
	int _SetWindowSize();
	int _SetWindowSizeFullScreen();

	//�ݒ�t�@�C��������
	int _InitConfFile();

	//�E�B���h�E�v���V�[�W��
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//���j���[�C�x���g����
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

	//���̑��C�x���g����
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

