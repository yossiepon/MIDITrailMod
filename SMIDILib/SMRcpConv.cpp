//******************************************************************************
//
// Simple MIDI Library / SMRcpConv
//
// RCP�t�@�C���ϊ��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "shlwapi.h"
#include "YNBaseLib.h"
#include "SMRcpConv.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMRcpConv::SMRcpConv(void)
{
	m_hModule = NULL;
	m_pFuncConvertFile = NULL;
	m_pFuncSaveSMF = NULL;
	m_pFuncDeleteObject = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMRcpConv::~SMRcpConv(void)
{
	_Release();
}

//******************************************************************************
// ������
//******************************************************************************
int SMRcpConv::Initialize()
{
	int result = 0;
	TCHAR dllFilePath[_MAX_PATH] = {_T('\0')};

	_Release();

	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	result = YNPathUtil::GetModuleDirPath(dllFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//DLL�t�@�C���p�X
	_tcscat_s(dllFilePath, _MAX_PATH, _T("RCPCV.DLL"));

	//DLL���݊m�F
	if (!PathFileExists(dllFilePath)) {
		//DLL�����݂��Ȃ��̂ŉ�����������I��
		goto EXIT;
	}

	//DLL�ǂݍ���
	//  LoadLibrary / FreeLibrary ��API���ŎQ�ƃJ�E���g���Ǘ����邽��
	//  �{�N���X�̃C���X�^���X���������݂��Ă����Ȃ�
	m_hModule = LoadLibrary(dllFilePath);
	if (m_hModule == NULL) {
		result = YN_SET_ERR("LoadLibrary Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//�֐��|�C���^�擾�FrcpcvConvertFile
	m_pFuncConvertFile = (RCPCV_ConvertFile)GetProcAddress(m_hModule, "rcpcvConvertFile");
	if (m_pFuncConvertFile == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//�֐��|�C���^�擾�FrcpcvSaveSMF
	m_pFuncSaveSMF = (RCPCV_SaveSMF)GetProcAddress(m_hModule, "rcpcvSaveSMF");
	if (m_pFuncSaveSMF == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

	//�֐��|�C���^�擾�FrcpcvDeleteObject
	m_pFuncDeleteObject = (RCPCV_DeleteObject)GetProcAddress(m_hModule, "rcpcvDeleteObject");
	if (m_pFuncDeleteObject == NULL) {
		result = YN_SET_ERR("GetProcAddress Error. (rcpcv.dll)", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	if (result != 0) {
		_Release();
	}
	return result;
}

//******************************************************************************
// �g�p�۔���
//******************************************************************************
bool SMRcpConv::IsAvailable()
{
	bool isAvailable = false;

	if (m_hModule != NULL) {
		isAvailable = true;
	}

	return isAvailable;
}

//******************************************************************************
// �t�@�C���ϊ�
//******************************************************************************
int SMRcpConv::Convert(
		const TCHAR* pRCPPath,
		const TCHAR* pSMFPath
	)
{
	int result = 0;
	int apiresult = 0;
	DWORD hRCPCV = 0;

	if (!IsAvailable()) {
		result = YN_SET_ERR("Program Error.", 0, 0);
		goto EXIT;
	}

	try {

		//RCP�t�@�C���ǂݍ���
		hRCPCV = (*m_pFuncConvertFile)(
						pRCPPath,	//�t�@�C���p�X
						0,			//�R�[���o�b�N��ʁF�Ȃ�
						NULL,		//�R�[���o�b�N�֐��^�E�B���h�E�n���h���F�Ȃ�
						0,			//�E�B���h�E���b�Z�[�W�F�Ȃ�
						0			//�C���X�^���X���ʗpID�F�Ȃ�
					);
		if (hRCPCV == 0) {
			result = YN_SET_ERR("File read error.", 0, 0);
			goto EXIT;
		}

		//SMF�o��
		apiresult = (*m_pFuncSaveSMF)(hRCPCV, pSMFPath);
		if (apiresult != 1) {
			result = YN_SET_ERR("File save error.", apiresult, 0);
			goto EXIT;
		}

	}
	catch (...) {
		result = YN_SET_ERR("Exception occurred. (rcpcv.dll)", 0, 0);
		goto EXIT;
	}

EXIT:;
	if (hRCPCV != 0) {
		(*m_pFuncDeleteObject)(hRCPCV);
	}
	return result;
}

//******************************************************************************
// �����[�X
//******************************************************************************
void SMRcpConv::_Release()
{
	if (m_hModule != NULL) {
		FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

//******************************************************************************
// �g���q�ɂ��T�|�[�g�Ώۃt�@�C������
//******************************************************************************
bool SMRcpConv::IsSupportFileExt(
		const TCHAR* pFilePath
	)
{
	bool isSupport = false;

	if (YNPathUtil::IsFileExtMatch(pFilePath, ".rcp")
	 || YNPathUtil::IsFileExtMatch(pFilePath, ".r36")
	 || YNPathUtil::IsFileExtMatch(pFilePath, ".g36")) {
		isSupport = true;
	}

	return isSupport;
}

//******************************************************************************
// GetOpenFileName�p�t�@�C���t�B���^�擾
//******************************************************************************
const TCHAR* SMRcpConv::GetOpenFileNameFilter()
{
	TCHAR* pFilter =
		_T("MIDI File (*.mid *.rcp *.r36 *.g36)\0*.mid;*.rcp;*.r36;*.g36\0")
		_T("Standard MIDI File (*.mid)\0*.mid\0")
		_T("Recomposer Data File (*.rcp *.r36 *.g36)\0*.rcp;*.r36;*.g36\0")
		_T("\0");
	return pFilter;
}

} // end of namespace

