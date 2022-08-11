//******************************************************************************
//
// MIDITrail / MTFileList
//
// �t�@�C�����X�g�N���X
//
// Copyright (C) 2021 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTFileList.h"
#include "shlwapi.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTFileList::MTFileList(void)
{
	m_TargetDirPath[0] = _T('\0');
	m_CurFilePath[0] = _T('\0');
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTFileList::~MTFileList(void)
{
	Clear();
}

//******************************************************************************
// �N���A
//******************************************************************************
void MTFileList::Clear()
{
	m_TargetDirPath[0] = _T('\0');
	m_CurFilePath[0] = _T('\0');
	m_FileNameList.clear();
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// �f�B���N�g���z���t�@�C�����X�g�쐬
//******************************************************************************
int MTFileList::MakeFileListWithDirectory(
		const TCHAR* pTargetDirPath,
		SMRcpConv* pRcpConv
	)
{
	int result = 0;
	TCHAR findPath[_MAX_PATH] = {_T('\0')};;
	WIN32_FIND_DATA findData;
	HANDLE hFind = NULL;
	BOOL isFind = true;
	bool isMIDIDataFile = false;
	
	if (pTargetDirPath == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	if (_tcslen(pTargetDirPath) > (_MAX_PATH - 1)) {
		result = YN_SET_ERR("Directory path is too long.", _tcslen(pTargetDirPath), 0);
		goto EXIT;
	}
	
	Clear();
	
	//�f�B���N�g���p�X��ێ�����
	_tcscpy_s(m_TargetDirPath, _MAX_PATH, pTargetDirPath);
	if (pTargetDirPath[_tcslen(pTargetDirPath) - 1] != _T('\\')) {
		_tcscat_s(m_TargetDirPath, _MAX_PATH, _T("\\"));
	}
	
	//�t�@�C�������p�p�X�쐬
	findPath[0] = _T('\0');
	_tcscat_s(findPath, _MAX_PATH, m_TargetDirPath);
	_tcscat_s(findPath, _MAX_PATH, _T("*.*"));

	//�t�@�C������
	hFind = FindFirstFile(findPath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		//�t�@�C����������Ȃ�
		goto EXIT;
	}

	//�t�@�C�������X�g���쐬
	while (isFind) {
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			//�f�B���N�g���͖�������
		}
		else {
			//�t�@�C���g���q���m�F
			isMIDIDataFile = false;
			if (YNPathUtil::IsFileExtMatch(findData.cFileName, _T(".mid"))) {
				isMIDIDataFile = true;
			}
			else if (pRcpConv->IsAvailable() && pRcpConv->IsSupportFileExt(findData.cFileName)) {
				isMIDIDataFile = true;
			}
			if (isMIDIDataFile) {
				//�t�@�C���������X�g�ɒǉ�
				m_FileNameList.push_back(findData.cFileName);
			}
		}
		//���̃t�@�C��������
		isFind = FindNextFile(hFind, &findData);
	}

	//�t�@�C�����\�[�g
	m_FileNameList.sort();

EXIT:;
	if (hFind != NULL) FindClose(hFind);
	return result;
}

//******************************************************************************
// �t�@�C�����擾
//******************************************************************************
size_t MTFileList::GetFileCount()
{
	return m_FileNameList.size();
}

//******************************************************************************
// �t�@�C���p�X�擾
//******************************************************************************
const TCHAR* MTFileList::GetFilePath(unsigned long index)
{
	TCHAR* pFilePath = NULL;
	MTFileNameList::iterator itr;

	if (m_FileNameList.size() <= index) {
		pFilePath = NULL;
	}
	else {
		itr = m_FileNameList.begin();
		advance(itr, index);
		m_CurFilePath[0] = _T('\0');
		_tcscat_s(m_CurFilePath, _MAX_PATH, m_TargetDirPath);
		_tcscat_s(m_CurFilePath, _MAX_PATH, (*itr).c_str());
		pFilePath = &(m_CurFilePath[0]);
	}

	return pFilePath;
}

//******************************************************************************
// �t�@�C�����擾
//******************************************************************************
const TCHAR* MTFileList::GetFileName(unsigned long index)
{
	const TCHAR* pFilePath = NULL;
	const TCHAR* pFileName = NULL;

	pFilePath = GetFilePath(index);
	if (pFilePath != NULL) {
		pFileName = PathFindFileName(pFilePath);
	}

	return pFileName;
}

//******************************************************************************
// �I���t�@�C���o�^
//******************************************************************************
int MTFileList::SetSelectedFileName(const TCHAR* pFileName)
{
	int result = 0;
	unsigned long index = 0;
	MTFileNameList::iterator itr;

	if (pFileName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_SelectedFileIndex = 0;

	//�t�@�C�������X�g���猟���i�啶������������ʂ��Ȃ��j
	for (itr = m_FileNameList.begin(); itr != m_FileNameList.end(); itr++) {
		if (_tcsicmp((*itr).c_str(), pFileName) == 0) {
			m_SelectedFileIndex = index;
			break;
		}
		index++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �擪�t�@�C���I��
//******************************************************************************
void MTFileList::SelectFirstFile()
{
	m_SelectedFileIndex = 0;
}

//******************************************************************************
// �O�t�@�C���I��
//******************************************************************************
void MTFileList::SelectPreviousFile(bool* pIsExist)
{
	bool isExist = false;

	//�t�@�C�������݂��Ȃ��ꍇ
	if (m_FileNameList.size() == 0) {
		//�O�t�@�C���Ȃ��ŏI��
	}
	//�t�@�C�����X�g�擪��I�𒆂̏ꍇ
	else if (m_SelectedFileIndex == 0) {
		//�O�t�@�C���Ȃ��ŏI��
	}
	else {
		//�O�t�@�C����I��
		m_SelectedFileIndex -= 1;
		isExist = true;
	}

	if (pIsExist != NULL) {
		*pIsExist = isExist;
	}

	return;
}

//******************************************************************************
// ���t�@�C���I��
//******************************************************************************
void MTFileList::SelectNextFile(bool* pIsExist)
{
	bool isExist = false;

	//�t�@�C�������݂��Ȃ��ꍇ
	if (m_FileNameList.size() == 0) {
		//���t�@�C���Ȃ��ŏI��
	}
	//�t�@�C�����X�g������I�𒆂̏ꍇ
	else if (m_SelectedFileIndex >= (m_FileNameList.size() - 1)) {
		//���t�@�C���Ȃ��ŏI��
	}
	else {
		//���t�@�C����I��
		m_SelectedFileIndex += 1;
		isExist = true;
	}
	
	if (pIsExist != NULL) {
		*pIsExist = isExist;
	}

	return;
}

//******************************************************************************
// �擪�t�@�C���I��
//******************************************************************************
unsigned long MTFileList::GetSelectedFileIndex()
{
	return m_SelectedFileIndex;
}


