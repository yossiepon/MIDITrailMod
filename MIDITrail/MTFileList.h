//******************************************************************************
//
// MIDITrail / MTFileList
//
// �t�@�C�����X�g�N���X
//
// Copyright (C) 2021 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "SMRcpConv.h"
#include <list>
#include <string>

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// �t�@�C�����X�g�N���X
//******************************************************************************
class MTFileList
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTFileList(void);
	virtual ~MTFileList(void);

	//�f�B���N�g���z���t�@�C�����X�g�쐬
	int MakeFileListWithDirectory(const TCHAR* pTargetDirPath, SMRcpConv* pRcpConv);

	//�t�@�C����
	size_t GetFileCount();

	//�t�@�C���p�X�擾
	const TCHAR* GetFilePath(unsigned long index);

	//�t�@�C�����擾
	const TCHAR* GetFileName(unsigned long index);

	//�N���A
	void Clear();

	//�I���t�@�C���o�^
	int SetSelectedFileName(const TCHAR* pFileName);

	//�擪�t�@�C���I��
	void SelectFirstFile();

	//�O�t�@�C���I��
	void SelectPreviousFile(bool* pExist);

	//���t�@�C���I��
	void SelectNextFile(bool* pExist);

	//�I���t�@�C���C���f�b�N�X�擾
	unsigned long GetSelectedFileIndex();

private:

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const MTFileList&);
	MTFileList(const MTFileList&);

	TCHAR m_TargetDirPath[_MAX_PATH];
	TCHAR m_CurFilePath[_MAX_PATH];

#ifdef _UNICODE
	typedef std::list<wsting> MTFileNameList;
#else
	typedef std::list<string> MTFileNameList;
#endif

	MTFileNameList m_FileNameList;

	unsigned long m_SelectedFileIndex;


};

