//******************************************************************************
//
// Simple Base Library / YNErrCtrl
//
// �G���[����N���X
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

#include "YNErrInfo.h"

namespace YNBaseLib {

//******************************************************************************
//�G���[����}�N��
//******************************************************************************
#define YN_SET_ERR(msg,info1,info2)   YNErrCtrl::SetErr(YNErrInfo::LVL_ERR,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SET_WARN(msg,info1,info2)  YNErrCtrl::SetErr(YNErrInfo::LVL_WARN,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SET_INFO(msg,info1,info2)  YNErrCtrl::SetErr(YNErrInfo::LVL_INFO,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SHOW_ERR(howner)   YNErrCtrl::ShowErr(howner)


//******************************************************************************
// �G���[����N���X
//******************************************************************************
class YNBASELIB_API YNErrCtrl
{
private:

	//�R���X�g���N�^�^�f�X�g���N�^
	//�C���X�^���X�����������Ȃ�
	YNErrCtrl();
	virtual ~YNErrCtrl();

public:

	//������
	//  �v���Z�X�A�^�b�`���Ɏ��s����
	//  ��ʗ��p�҂͗��p���Ȃ�����
	static int Initialize();

	//�I��
	//  �v���Z�X�I�����Ɏ��s����
	//  ��ʗ��p�҂͗��p���Ȃ�����
	static int Terminate();

	//�G���[���o�^
	static int SetErr(
			YNErrInfo::ErrLevel errLevel,
			unsigned long lineNo,
			const TCHAR* pFuncName,
			const TCHAR* pMessage,
			unsigned long long errInfo1,
			unsigned long long errInfo2
		);

	//�G���[���擾
	static YNErrInfo* GetErr();

	//�G���[���_�C�A���O�\��
	static int ShowErr(HWND hOwner);

private:

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const YNErrCtrl&);
	YNErrCtrl(const YNErrCtrl&);

};

} // end of namespace

