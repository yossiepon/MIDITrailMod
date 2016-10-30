//******************************************************************************
//
// MIDITrail / MTMIDIINCfgDlg
//
// MIDI IN �ݒ�_�C�A���O
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "SMIDILib.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// MIDI IN �ݒ�_�C�A���O�N���X
//******************************************************************************
class MTMIDIINCfgDlg
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTMIDIINCfgDlg(void);
	virtual ~MTMIDIINCfgDlg(void);

	//�\���F�_�C�A���O��������܂Ő����Ԃ��Ȃ�
	int Show(HWND hParentWnd);

private:

	//�E�B���h�E�v���V�[�W������p�|�C���^
	static MTMIDIINCfgDlg* m_pThis;

	//�A�v���P�[�V�����C���X�^���X
	HINSTANCE m_hInstance;

	//�ݒ�t�@�C��
	YNConfFile m_ConfFile;

	//MIDI���̓f�o�C�X����I�u�W�F�N�g
	SMInDevCtrl m_MIDIInDevCtrl;

	//�R���{�{�b�N�X�̃E�B���h�E�n���h��
	HWND m_hComboDevA;

	//MIDITHRU�`�F�b�N�{�b�N�X�̃E�B���h�E�n���h��
	HWND m_hMIDITHRU;

	//�E�B���h�E�v���V�[�W��
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//�_�C�A���O�\�����O������
	int _OnInitDlg(HWND hDlg);

	//�ݒ�t�@�C��������
	int _InitConfFile();

	//�f�o�C�X�I���R���{�{�b�N�X������
	int _InitComboDev(HWND hComboDev, TCHAR* pPortName);

	//MIDITHRU�`�F�b�N�{�^��������
	int _InitCheckBtnMIDITHRU();

	//�ۑ�����
	int _Save();
	int _SavePortCfg(HWND hComboDev, TCHAR* pPortName);
	int _SaveMIDITHRU();

};


