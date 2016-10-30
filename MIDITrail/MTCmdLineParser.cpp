//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// �R�}���h���C����̓N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMRcpConv.h"
#include "MTCmdLineParser.h"
#include <tchar.h>
#include <stdlib.h>

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTCmdLineParser::MTCmdLineParser(void)
{
	m_pFilePath = _T("");
	ZeroMemory(m_CmdSwitchStatus, sizeof(unsigned char)*CMDSW_MAX);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTCmdLineParser::~MTCmdLineParser(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int MTCmdLineParser::Initialize(
		LPTSTR pCmdLine
	)
{
	int result = 0;

	//�R�}���h���C�����
	result = _AnalyzeCmdLine(pCmdLine);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �R�}���h���C�����
//******************************************************************************
int MTCmdLineParser::_AnalyzeCmdLine(
		LPTSTR pCmdLine
	)
{
	int result = 0;
	int i = 0;
	TCHAR* pArg = NULL;
	SMRcpConv rcpConv;

	//CommandLineToArgvW �͑��݂��邪 CommandLineToArgvA �͑��݂��Ȃ�
	//���̂���API�ł̉�͂͂�����߂� __argc, __targv �𗘗p����
	//�c�O�Ȃ���pCmdLine�͎Q�Ƃ��Ȃ�

	//RCP�ǂݍ��݉ۊm�F�̂���RCP�t�@�C���ϊ��I�u�W�F�N�g��p�ӂ���
	result = rcpConv.Initialize();
	if (result != 0) goto EXIT;

	//�����̉��
	for (i = 1; i < __argc; i++) {
		pArg = __targv[i];

		//MessageBox(NULL, pArg, _T(""), MB_OK);

		//�t�@�C���p�X
		//  �t�@�C���p�X�������w�肳�ꂽ�ꍇ�͐擪�݂̂��̗p����
		if ((_tcslen(m_pFilePath) == 0) && (_tcslen(pArg) > 4)) {
			if (YNPathUtil::IsFileExtMatch(pArg, ".mid")) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
			else if (rcpConv.IsAvailable() && rcpConv.IsSupportFileExt(pArg)) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
		}
		//�N����ɍĐ��J�n
		if (_tcscmp(pArg, _T("-p")) == 0) {
			m_CmdSwitchStatus[CMDSW_PLAY] = CMDSW_ON;
		}
		//�Đ��I�����ɃA�v���I��
		if (_tcscmp(pArg, _T("-q")) == 0) {
			m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_ON;
		}
		//�f�o�b�O���[�h
		if (_tcscmp(pArg, _T("-d")) == 0) {
			m_CmdSwitchStatus[CMDSW_DEBUG] = CMDSW_ON;
		}
	}

	//�t�@�C���p�X�����w��̏ꍇ
	if (m_CmdSwitchStatus[CMDSW_FILE_PATH] != CMDSW_ON) {
		//�Đ��^�I���t���O�͋��ɖ���
		m_CmdSwitchStatus[CMDSW_PLAY] = CMDSW_NONE;
		m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_NONE;
	}

	//�Đ��t���OON�łȂ���ΏI���t���O�͖���
	if (m_CmdSwitchStatus[CMDSW_PLAY] != CMDSW_ON) {
		m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_NONE;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �X�C�b�`��Ԏ擾
//******************************************************************************
int MTCmdLineParser::GetSwitch(
		unsigned long switchType
	)
{
	int switchStatus = CMDSW_NONE;

	if (switchType < CMDSW_MAX) {
		switchStatus = m_CmdSwitchStatus[switchType];
	}

	return switchStatus;
}

//******************************************************************************
// �t�@�C���p�X�擾
//******************************************************************************
const TCHAR* MTCmdLineParser::GetFilePath()
{
	return m_pFilePath;
}


