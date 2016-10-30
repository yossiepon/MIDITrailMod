//******************************************************************************
//
// MIDITrail / MTHowToViewDlg
//
// ������@�_�C�A���O
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// ������@�_�C�A���O
//******************************************************************************
class MTHowToViewDlg
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTHowToViewDlg(void);
	virtual ~MTHowToViewDlg(void);

	//�\���F�_�C�A���O��������܂Ő����Ԃ��Ȃ�
	int Show(HWND hParentWnd);

private:

	BITMAPFILEHEADER m_BmpHead;
	BITMAPINFOHEADER m_BmpInfo;
	HANDLE m_hMemBmpPixel;
	BYTE* m_pBmpPixcel;
	unsigned long m_PageNo;
	HWND m_hWnd;

	//�E�B���h�E�v���V�[�W������p�|�C���^
	static MTHowToViewDlg* m_pThis;

	//�A�v���P�[�V�����C���X�^���X
	HINSTANCE m_hInstance;

	//�E�B���h�E�v���V�[�W��
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//�_�C�A���O�\�����O������
	int _OnInitDlg(HWND hDlg);

	//HowTo�r�b�g�}�b�v�ǂݍ���
	int _LoadHowToBmp();

	//HowTo�r�b�g�}�b�v�`��
	int _DrawHowToBmp();

	//�N���A
	void _Clear();

	//Previous�{�^��
	int _OnPreviousButton();

	//Next�{�^��
	int _OnNextButton();

	//�摜�\��
	int _DrawHowToImage();

	//�{�^����ԍX�V
	void _UpdateButtonStatus();

};


