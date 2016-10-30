//******************************************************************************
//
// MIDITrail / MTConfFile
//
// �ݒ�t�@�C���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTParam.h"
#include "MTConfFile.h"


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTConfFile::MTConfFile(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTConfFile::~MTConfFile(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int MTConfFile::Initialize(
		const TCHAR* pCategory
	)
{
	int result = 0;
	TCHAR confFilePath[_MAX_PATH] = {_T('\0')};
	YNConfFile confFile;

	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	result = YNPathUtil::GetModuleDirPath(confFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//�ݒ�t�@�C���p�X�o�^
	_tcscat_s(confFilePath, _MAX_PATH, MT_CONFFILE_DIR);
	_tcscat_s(confFilePath, _MAX_PATH, pCategory);
	_tcscat_s(confFilePath, _MAX_PATH, _T(".ini"));

	//������
	result = YNConfFile::Initialize(confFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


