//******************************************************************************
//
// Simple Base Library / YNPathUtil
//
// �p�X���[�e�B���e�B�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

namespace YNBaseLib {

//******************************************************************************
// �p�X���[�e�B���e�B�N���X
//******************************************************************************
class YNBASELIB_API YNPathUtil
{
public:

	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	//  ������"\"��t�^����
	//  �擾�p�X�̗�F"C:\Program Files\AppName\"
	static int GetModuleDirPath(TCHAR* pBuf, unsigned long bufSize);

	//�A�v���P�[�V�����f�[�^�f�B���N�g���p�X�擾
	//  ������"\"��t�^����
	//  �擾�p�X�̗�FWidows7�̏ꍇ "C:\Users\UserName\AppData\Roaming\"
	static int GetAppDataDirPath(TCHAR* pBuf, unsigned long bufSize);

	//�g���q����
	//  �t�@�C���̊g���q���w�肳�ꂽ���̂ł��邩���肷��
	//  �w�肷��g���q�̗�F".txt"
	static bool IsFileExtMatch(const TCHAR* pPath, const TCHAR* pExt);

	//�e���|�����t�@�C���p�X�擾
	//  ���ϐ�(TMP or TEMP)�Œ�`���ꂽ�e���|�����f�B���N�g����
	//  ��ӂ̃e���|�����t�@�C�����쐬���ăp�X��ԋp����
	//  �w��ł���v���t�B�b�N�X��3����
	//  �쐬�����t�@�C���̖��̂� PREuuuu.TMP
	static int GetTempFilePath(TCHAR* pPathBuf, unsigned long bufSize, const TCHAR* pPrefix);

private:

	//�R���X�g���N�^�^�f�X�g���N�^
	YNPathUtil(void);
	virtual ~YNPathUtil(void);

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const YNPathUtil&);
	YNPathUtil(const YNPathUtil&);

};

} // end of namespace

