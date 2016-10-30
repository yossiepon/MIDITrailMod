//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// �E�B���h�E�T�C�Y�ݒ�_�C�A���O�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "resource.h"
#include "YNBaseLib.h"
#include <list>

using namespace YNBaseLib;


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
	bool IsCahnged();

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

	//�E�B���h�E�T�C�Y�I���R���{�{�b�N�X������
	int _InitSizeList();

	//�ۑ�����
	int _Save();

};

