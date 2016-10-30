//******************************************************************************
//
// Simple Base Library / YNConfFile
//
// �ݒ�t�@�C���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// INI�t�@�C���ւ̃A�N�Z�X�����b�v����N���X�B

#pragma once

#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

#include <stdlib.h>

namespace YNBaseLib {

//******************************************************************************
// �ݒ�t�@�C���N���X
//******************************************************************************
class YNBASELIB_API YNConfFile
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	YNConfFile(void);
	virtual ~YNConfFile(void);

	//������
	int Initialize(const TCHAR* pConfFilePath);

	//�J�����g�Z�N�V�����ݒ�
	int SetCurSection(const TCHAR* pSection);

	//�����l�擾�^�o�^
	int GetInt(const TCHAR* pKey, int* pVal, int defaultVal);
	int SetInt(const TCHAR* pKey, int val);

	//���������l�擾�^�o�^
	int GetFloat(const TCHAR* pKey, float* pVal, float defaultVal);
	int SetFloat(const TCHAR* pKey, float val);

	//������擾�^�o�^
	int GetStr(const TCHAR* pKey, TCHAR* pBuf, unsigned long bufSize, const TCHAR* pDefaultVal);
	int SetStr(const TCHAR* pKey, const TCHAR* pStr);

private:

	TCHAR m_FilePath[_MAX_PATH];
	TCHAR m_Section[_MAX_PATH];

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const YNConfFile&);
	YNConfFile(const YNConfFile&);

};


} // end of namespace

