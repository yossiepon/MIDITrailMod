//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// �E�B���h�E�T�C�Y�ݒ�_�C�A���O�N���X
//
// Copyright (C) 2010-2016 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "resource.h"
#include "YNBaseLib.h"
#include <list>

using namespace YNBaseLib;


//******************************************************************************
//�E�B���h�E�T�C�Y�ݒ�_�C�A���O�N���X �p�����^��`
//******************************************************************************

//�E�B���h�E�T�C�Y�ݒ�ŏ��l
#define MT_WINDOW_SIZE_MIN			200

//�E�B���h�E�T�C�Y�ݒ�ő�l
#define MT_WINDOW_SIZE_MAX			99999

//�E�B���h�E�T�C�Y�������ő�l�F99999��5��
#define MT_WINDOW_SIZE_CHAR_MAX		5


//******************************************************************************
//�E�B���h�E�T�C�Y�ݒ�_�C�A���O�N���X
//******************************************************************************
class MTWindowSizeCfgDlg
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTWindowSizeCfgDlg(void);
	virtual ~MTWindowSizeCfgDlg(void);

	//�\���F�_�C�A���O��������܂Ő����Ԃ��Ȃ�
	int Show(HWND hParentWnd);

	//�ύX�m�F
	bool IsChanged();

private:

	typedef struct {
		unsigned long width;
		unsigned long height;
	} MTWindowSizeItem;

	typedef std::list<MTWindowSizeItem> MTWindowSizeList;

private:

	//�E�B���h�E�v���V�[�W������p�|�C���^
	static MTWindowSizeCfgDlg* m_pThis;

	//�A�v���P�[�V�����C���X�^���X
	HINSTANCE m_hInstance;

	//�E�B���h�E�T�C�Y�I�����X�g�{�b�N�X�E�B���h�E�n���h��
	HWND m_hSizeList;

	//�E�B���h�E�T�C�Y���X�g
	MTWindowSizeList m_SizeList;

	//�E�B���h�E�T�C�Y�G�f�B�b�g�{�b�N�X�E�B���h�E�n���h��
	HWND m_hEditWidth;
	HWND m_hEditHeight;

	//�r���[�̈�K�p�`�F�b�N�{�b�N�X�E�B���h�E�n���h��
	HWND m_hCheckApplyToView;

	//�ݒ�t�@�C��
	YNConfFile m_ConfFile;

	//�ۑ����{�t���O
	bool m_isSaved;

	//�E�B���h�E�v���V�[�W��
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//�_�C�A���O�\�����O������
	int _OnInitDlg(HWND hDlg);

	//�ݒ�t�@�C��������
	int _InitConfFile();

	//�E�B���h�E�T�C�Y�I�����X�g�{�b�N�X������
	int _InitSizeList();

	//�E�B���h�E�T�C�Y�G�f�B�b�g�{�b�N�X������
	int _InitSizeEditbox();

	//�E�B���h�E�T�C�Y�擾
	int _GetConfWindowSize(int* pWidth, int* pHeight);

	//�ۑ�����
	int _Save();

	//�E�B���h�E�T�C�Y���X�g�{�b�N�X�I����ԕω�
	int _OnSizeListChanged();

	//�E�B���h�E�T�C�Y�G�f�B�b�g�{�b�N�X�X�V
	int _UpdateSizeEditBox(int width, int height);

};

