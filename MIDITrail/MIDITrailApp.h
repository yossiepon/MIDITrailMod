//******************************************************************************
//
// MIDITrail / MIDITrailApp
//
// MIDITrail �A�v���P�[�V�����N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
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

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
#define MAX_LOADSTRING  (100)

//�E�B���h�E�X�^�C��
//  WS_OVERLAPPEDWINDOW ���玟�̃X�^�C�������������
//    WS_THICKFRAME   �T�C�Y�ύX��
//    WS_MAXIMIZEBOX  �ő剻�{�^��
#define MIDITRAIL_WINDOW_STYLE  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

//�V�[�P���T���b�Z�[�WID
#define WM_SEQUENCER_MESSAGE (WM_USER + 1)

//���j���[�X�^�C������
#define MT_MENU_NUM        (29)
#define MT_PLAYSTATUS_NUM  (6)

//�f�o�C�X���X�g�x�����b�Z�[�W
#define MIDITRAIL_MSG_DEVICELOST  _T("Direct3D device is lost.")

//�^�C�}�[ID
#define MIDITRAIL_TIMER_CHECK_KEY  (1)


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
	enum SceneType {
		Title,			//�^�C�g��
		PianoRoll3D,	//�s�A�m���[��3D
		PianoRoll2D,	//�s�A�m���[��2D
		PianoRollRain	//�s�A�m���[�����C��
	};

	//�V�[�P���T���b�Z�[�W
	typedef struct {
		unsigned long wParam;
		unsigned long lParam;
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

	//�R�}���h���C���p�[�T
	MTCmdLineParser m_CmdLineParser;

	//�E�B���h�E�n
	HWND m_hWnd;
	HACCEL m_Accel;
	TCHAR m_Title[MAX_LOADSTRING];
	TCHAR m_WndClassName[MAX_LOADSTRING];

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
	SMLiveMonitor m_LiveMonitor;

	//���t���
	PlayStatus m_PlayStatus;
	bool m_isRepeat;
	bool m_isRewind;
	MTSequencerLastMsg m_SequencerLastMsg;
	unsigned long m_PlaySpeedRatio;

	//�\������
	bool m_isEnablePianoKeyboard;
	bool m_isEnableRipple;
	bool m_isEnablePitchBend;
	bool m_isEnableStars;
	bool m_isEnableCounter;

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

	//�X�L�b�v����
	int m_SkipBackTimeSpanInMsec;
	int m_SkipForwardTimeSpanInMsec;

	//���t�X�s�[�h����
	unsigned long m_SpeedStepInPercent;
	unsigned long m_MaxSpeedInPercent;

	//----------------------------------------------------------------
	//���\�b�h��`
	//----------------------------------------------------------------
	//�E�B���h�E����
	int _RegisterClass(HINSTANCE hInstance);
	int _CreateWindow(HINSTANCE hInstance, int nCmdShow);
	int _SetWindowSize();

	//�ݒ�t�@�C��������
	int _InitConfFile();

	//�E�B���h�E�v���V�[�W��
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//���j���[�C�x���g����
	int _OnMenuFileOpen();
	int _OnMenuFileAdd();
	int _OnMenuPlay();
	int _OnMenuStop();
	int _OnMenuRepeat();
	int _OnMenuSkipBack();
	int _OnMenuSkipForward();
	int _OnMenuPlaySpeedDown();
	int _OnMenuPlaySpeedUp();
	int _OnMenuStartMonitoring();
	int _OnMenuStopMonitoring();
	int _OnMenuResetViewpoint();
	int _OnMenuSaveViewpoint();
	int _OnMenuEnableEffect(MTScene::EffectType type);
	int _OnMenuWindowSize();
	int _OnMenuOptionMIDIOUT();
	int _OnMenuOptionMIDIIN();
	int _OnMenuOptionGraphic();
	int _OnMenuManual();
	int _OnMenuSelectSceneType(SceneType type);

	//���̑��C�x���g����
	int _OnRecvSequencerMsg(unsigned long wParam, unsigned long lParam);
	int _OnMouseButtonDown(unsigned long button, unsigned long wParam, unsigned long lParam);
	int _OnKeyDown(unsigned long wParam, unsigned long lParam);
	int _OnDropFiles(unsigned long wParam, unsigned long lParam);

	int _SelectMIDIFile(TCHAR* pFilePath,  unsigned long bufSize, bool* pIsSelected);
	int _LoadMIDIFile(const TCHAR* pFilePath);
	int _AddMIDIFile(const TCHAR* pFilePath);
	void _UpdateFPS();
	int _SetPortDev(SMSequencer* pSequencer);
	int _SetMonitorPortDev(SMLiveMonitor* pLiveMonitor, MTScene* pScene);
	int _ChangeWindowSize();
	int _ChangePlayStatus(PlayStatus status);
	int _ChangeMenuStyle();
	int _CreateScene(SceneType type, SMSeqData* pSeqData);
	int _LoadSceneType();
	int _SaveSceneType();
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
	
};

