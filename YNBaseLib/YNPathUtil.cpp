//******************************************************************************
//
// Simple Base Library / YNPathUtil
//
// �p�X���[�e�B���e�B�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNErrCtrl.h"
#include "YNPathUtil.h"
#include <stdlib.h>
#include <shlobj.h>
#include <stdio.h>

namespace YNBaseLib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
YNPathUtil::YNPathUtil(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
YNPathUtil::~YNPathUtil(void)
{
}

//******************************************************************************
// �v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
//******************************************************************************
int YNPathUtil::GetModuleDirPath(
		TCHAR* pBuf,
		unsigned long bufSize
	)
{
	int result = 0;
	DWORD apiresult = 0;
	errno_t eresult = 0;
	TCHAR path[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	//�v���Z�X���s�t�@�C���p�X�擾
	apiresult = GetModuleFileName(GetModuleHandle(NULL), path, _MAX_PATH);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�p�X�v�f�̕���
	eresult = _tsplitpath_s(
					path,		//�p�X
					drive,		//�h���C�u������o�b�t�@
					_MAX_DRIVE,	//�o�b�t�@�T�C�Y
					dir,		//�f�B���N�g��������o�b�t�@
					_MAX_DIR,	//�o�b�t�@�T�C�Y
					fname,		//�t�@�C����������o�b�t�@
					_MAX_FNAME,	//�o�b�t�@�T�C�Y
					ext,		//�g���q������o�b�t�@
					_MAX_EXT	//�o�b�t�@�T�C�Y
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�p�X�쐬
	eresult = _tmakepath_s(
					pBuf,		//�p�X�i�[��o�b�t�@
					bufSize,	//�o�b�t�@�T�C�Y
					drive,		//�h���C�u������
					dir,		//�f�B���N�g��������
					NULL,		//�t�@�C����������
					NULL		//�g���q������
				);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �A�v���P�[�V�����f�[�^�f�B���N�g���p�X�擾
//******************************************************************************
int YNPathUtil::GetAppDataDirPath(
		TCHAR* pBuf,
		unsigned long bufSize
	)
{
	int result = 0;
	HRESULT hresult = 0;
	errno_t eresult = 0;
	TCHAR path[MAX_PATH];

	hresult = SHGetFolderPath(
					NULL,				//�I�[�i�[�E�B���h�E
					CSIDL_APPDATA,		//�t�H���_�w��
					NULL,				//�A�N�Z�X�g�[�N��
					SHGFP_TYPE_CURRENT,	//�t���O�F���݂̃t�H���_�p�X
										//  ���[�U���ύX���Ă���\��������
					path				//�p�X�i�[��o�b�t�@
				);
	if (hresult != S_OK) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	eresult = _tcscpy_s(pBuf, bufSize, path);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	eresult = _tcscat_s(pBuf, bufSize, _T("\\"));
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �g���q����
//******************************************************************************
bool YNPathUtil::IsFileExtMatch(
		const TCHAR* pPath,
		const TCHAR* pExt
	)
{
	bool isMatch = false;
	errno_t eresult = 0;
	TCHAR ext[_MAX_EXT] = {_T('\0')};

	//�p�X�v�f�𕪊����Ċg���q���擾
	eresult = _tsplitpath_s(
					pPath,			//�p�X
					NULL, 0,		//�h���C�u������o�b�t�@�ƃT�C�Y
					NULL, 0,		//�f�B���N�g��������o�b�t�@�ƃT�C�Y
					NULL, 0,		//�t�@�C����������o�b�t�@�ƃT�C�Y
					ext, _MAX_EXT	//�g���q������o�b�t�@�ƃT�C�Y
				);
	if (eresult != 0) {
		//result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�啶���Ə���������ʂ����Ɋg���q���r����
	if (_tcsicmp(ext, pExt) == 0) {
		isMatch = true;
	}

EXIT:;
	return isMatch;
}

//******************************************************************************
// �e���|�����t�@�C���p�X�擾
//******************************************************************************
int YNPathUtil::GetTempFilePath(
		TCHAR* pPathBuf,
		unsigned long bufSize,
		const TCHAR* pPrefix
	)
{
	int result = 0;
	DWORD apiresult = 0;
	TCHAR tempDir[_MAX_PATH] = {_T('\0')};

	//�e���|�����f�B���N�g���p�X���擾
	apiresult = GetTempPath(_MAX_PATH, tempDir);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//GetTmpFileName�̓o�b�t�@�T�C�Y���w��ł��Ȃ����API�ł���
	//�u�o�b�t�@�T�C�Y��MAX_PATH�ȏ�ɂ���v�ƒ�`����Ă���̂�
	//�T�C�Y�`�F�b�N���s��
	if (bufSize < MAX_PATH) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�e���|�����t�@�C���p�X���擾
	//  �t�@�C�����FPREuuuu.TMP
	//    PRE �F�v���t�B�b�N�X
	//    uuuu�F�V�X�e�������Ɋ�Â��Đ������ꂽ16�i������
	apiresult = GetTempFileName(
						tempDir,	//�f�B���N�g���p�X
						pPrefix,	//�v���t�B�b�N�X�i3�����j
						0,			//��Ӑ��F�L��
						pPathBuf	//�������ꂽ�t�@�C���p�X
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

} // end of namespace

