//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// �R�}���h���C����̓N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�X�C�b�`���
#define CMDSW_NONE		(0)	//����`
#define CMDSW_ON		(1)	//ON

//�X�C�b�`���
#define CMDSW_FILE_PATH	(0)	//�t�@�C���p�X
#define CMDSW_PLAY		(1)	//�Đ�
#define CMDSW_QUIET		(2)	//�I��
#define CMDSW_DEBUG		(3)	//�f�o�b�O���[�h
#define CMDSW_MAX		(4)	//�I�[�t���O�F�K�������ɒ�`����


//******************************************************************************
// �R�}���h���C����̓N���X
//******************************************************************************
class MTCmdLineParser
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTCmdLineParser(void);
	virtual ~MTCmdLineParser(void);

	//������
	int Initialize(LPTSTR pCmdLine);

	//�X�C�b�`��Ԏ擾
	int GetSwitch(unsigned long switchType);

	//�t�@�C���p�X�擾
	const TCHAR* GetFilePath();

private:

	unsigned char m_CmdSwitchStatus[CMDSW_MAX];
	TCHAR* m_pFilePath;

	int _AnalyzeCmdLine(LPTSTR pCmdLine);

};


