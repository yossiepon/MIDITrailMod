//******************************************************************************
//
// Simple Base Library / YNConfFile
//
// �ݒ�t�@�C���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNConfFile.h"
#include <stdio.h>
#include <stdlib.h>

namespace YNBaseLib {

//******************************************************************************
// �p�����[�^��`
//******************************************************************************
#define YNCONFFILE_NO_DATA  _T("*** NO DATA ***")

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
YNConfFile::YNConfFile(void)
{
	m_FilePath[0] = _T('\0');
	m_Section[0] = _T('\0');
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
YNConfFile::~YNConfFile(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int YNConfFile::Initialize(
		const TCHAR* pConfFilePath
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = _tcscpy_s(m_FilePath, _MAX_PATH, pConfFilePath);
	if (eresult != 0) {
		result = -1;
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �Z�N�V�����ݒ�
//******************************************************************************
int YNConfFile::SetCurSection(
		const TCHAR* pSection
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = _tcscpy_s(m_Section, _MAX_PATH, pSection);
	if (eresult != 0) {
		result = -1;
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �����l�擾
//******************************************************************************
int YNConfFile::GetInt(
		const TCHAR* pKey,
		int* pVal,
		int defaultVal
	)
{
	int result = 0;
	DWORD apiresult = 0;
	TCHAR buf[20];

	apiresult = GetPrivateProfileString(
					m_Section,			//�Z�N�V������
					pKey,				//�L�[��
					YNCONFFILE_NO_DATA, //�f�t�H���g������
					buf,				//�o�b�t�@�ʒu
					20,					//�o�b�t�@�T�C�Y�iTCHAR�P�ʁj
					m_FilePath			//�t�@�C���p�X
				);
	//�߂�l�̃`�F�b�N�͂�����߂�

	if (_tcscmp(buf, YNCONFFILE_NO_DATA) == 0) {
		*pVal = defaultVal;
	}
	else {
		*pVal = _tstoi(buf);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// �����l�o�^
//******************************************************************************
int YNConfFile::SetInt(
		const TCHAR* pKey,
		int val
	)
{
	int result = 0;
	BOOL bresult = TRUE;
	TCHAR buf[20];

	_stprintf_s(buf, 20, _T("%d"), val);

	bresult = WritePrivateProfileString(
					m_Section,		//�Z�N�V������
					pKey,			//�L�[��
					buf,			//�o�^���镶����
					m_FilePath		//�t�@�C���p�X
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ���������l�擾
//******************************************************************************
int YNConfFile::GetFloat(
		const TCHAR* pKey,
		float* pVal,
		float defaultVal
	)
{
	int result = 0;
	DWORD apiresult = 0;
	TCHAR buf[20];

	apiresult = GetPrivateProfileString(
					m_Section,			//�Z�N�V������
					pKey,				//�L�[��
					YNCONFFILE_NO_DATA, //�f�t�H���g������
					buf,				//�o�b�t�@�ʒu
					20,					//�o�b�t�@�T�C�Y�iTCHAR�P�ʁj
					m_FilePath			//�t�@�C���p�X
				);
	//�߂�l�̃`�F�b�N�͂�����߂�

	if (_tcscmp(buf, YNCONFFILE_NO_DATA) == 0) {
		*pVal = defaultVal;
	}
	else {
		//_tstof��double��Ԃ�
		*pVal = (float)_tstof(buf);
	}

//EXIT:;
	return result;
}

//******************************************************************************
// ���������l�o�^
//******************************************************************************
int YNConfFile::SetFloat(
		const TCHAR* pKey,
		float val
	)
{
	int result = 0;
	BOOL bresult = TRUE;
	TCHAR buf[20];

	_stprintf_s(buf, 20, _T("%f"), val);

	bresult = WritePrivateProfileString(
					m_Section,		//�Z�N�V������
					pKey,			//�L�[��
					buf,			//�o�^���镶����
					m_FilePath		//�t�@�C���p�X
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ������擾
//******************************************************************************
int YNConfFile::GetStr(
		const TCHAR* pKey,
		TCHAR* pBuf,
		unsigned long bufSize, 
		const TCHAR* pDefaultVal
	)
{
	int result = 0;
	DWORD apiresult = 0;

	apiresult = GetPrivateProfileString(
					m_Section,			//�Z�N�V������
					pKey,				//�L�[��
					pDefaultVal,		//�f�t�H���g������
					pBuf,				//�o�b�t�@�ʒu
					bufSize,			//�o�b�t�@�T�C�Y�iTCHAR�P�ʁj
					m_FilePath			//�t�@�C���p�X
				);
	//�߂�l�̃`�F�b�N�͂�����߂�

//EXIT:;
	return result;
}

//******************************************************************************
// ������o�^
//******************************************************************************
int YNConfFile::SetStr(
		const TCHAR* pKey,
		const TCHAR* pStr
	)
{
	int result = 0;
	BOOL bresult = TRUE;

	bresult = WritePrivateProfileString(
					m_Section,		//�Z�N�V������
					pKey,			//�L�[��
					pStr,			//�o�^���镶����
					m_FilePath		//�t�@�C���p�X
				);
	if (!bresult) {
		result = -1;  //GetLastError
		goto EXIT;
	}

EXIT:;
	return result;
}

} // end of namespace

