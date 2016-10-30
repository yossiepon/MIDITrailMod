//******************************************************************************
//
// MIDITrail / MTAboutDlg
//
// �o�[�W�������_�C�A���O�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "resource.h"


//******************************************************************************
// �o�[�W�������_�C�A���O�N���X
//******************************************************************************
class MTAboutDlg
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTAboutDlg(void);
	virtual ~MTAboutDlg(void);

	//�\��
	int Show(HWND hParentWnd);

private:

	//�E�B���h�E�v���V�[�W������p�|�C���^
	static MTAboutDlg* m_pThis;

	//�A�v���P�[�V�����C���X�^���X
	HINSTANCE m_hInstance;

	//�E�B���h�E�v���V�[�W��
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

};


