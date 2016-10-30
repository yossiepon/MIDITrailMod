//******************************************************************************
//
// MIDITrail / MIDITrailMain
//
// �A�v���P�[�V�����G���g���|�C���g
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MIDITrailApp.h"
#include "MIDITrailMain.h"

using namespace YNBaseLib;


//******************************************************************************
// �G���g���|�C���g
//******************************************************************************
int APIENTRY _tWinMain(
		HINSTANCE hInstance,		//�C���X�^���X�n���h��
		HINSTANCE hPrevInstance,	//�ȑO�̃C���X�^���X�n���h���F���NULL
		LPTSTR lpCmdLine,			//�R�}���h���C��
		int nCmdShow				//�E�B���h�E�\����Ԏw��
	)
{
	int result = 0;
	int winMainResult = 0;
	MIDITrailApp app;

	//���Q�ƌx�����
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//�A�v���P�[�V����������
	//���b�Z�[�W���[�v�ɓ���O�ɏI������ꍇ�͖߂�l��0�Ƃ���
	result = app.Initialize(hInstance, lpCmdLine, nCmdShow);
	if (result != 0) {
		YN_SHOW_ERR(NULL);
		winMainResult = 0;
		goto EXIT;
	}

	//�A�v���P�[�V�������s
	//Run��WinMain�̖߂�l�ƂȂ�l��Ԃ�
	winMainResult = app.Run();

EXIT:;
	app.Terminate();
	return winMainResult;
}


