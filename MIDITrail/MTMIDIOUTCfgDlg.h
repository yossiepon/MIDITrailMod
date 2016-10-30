//******************************************************************************
//
// MIDITrail / MTMIDIOUTCfgDlg
//
// MIDI OUT �ݒ�_�C�A���O�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "SMIDILib.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// MIDI OUT �ݒ�_�C�A���O�N���X
//******************************************************************************
class MTMIDIOUTCfgDlg
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTMIDIOUTCfgDlg(void);
	virtual ~MTMIDIOUTCfgDlg(void);

	//�\���F�_�C�A���O��������܂Ő����Ԃ��Ȃ�
	int Show(HWND hParentWnd);

private:

	//�E�B���h�E�v���V�[�W������p�|�C���^
	static MTMIDIOUTCfgDlg* m_pThis;

	//�A�v���P�[�V�����C���X�^���X
	HINSTANCE m_hInstance;

	//�ݒ�t�@�C��
	YNConfFile m_ConfFile;

	//MIDI�o�̓f�o�C�X����I�u�W�F�N�g
	SMOutDevCtrl m_MIDIOutDevCtrl;

	//�R���{�{�b�N�X�̃E�B���h�E�n���h��
	HWND m_hComboDevA;
	HWND m_hComboDevB;
	HWND m_hComboDevC;
	HWND m_hComboDevD;
	HWND m_hComboDevE;
	HWND m_hComboDevF;

	//�E�B���h�E�v���V�[�W��
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//�_�C�A���O�\�����O������
	int _OnInitDlg(HWND hDlg);

	//�ݒ�t�@�C��������
	int _InitConfFile();

	//�f�o�C�X�I���R���{�{�b�N�X������
	int _InitComboDev(HWND hComboDev, TCHAR* pPortName);

	//�ۑ�����
	int _Save();
	int _SavePortCfg(HWND hComboDev, TCHAR* pPortName);

};


