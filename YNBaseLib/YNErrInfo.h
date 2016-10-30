//******************************************************************************
//
// Simple Base Library / YNErrInfo
//
// �G���[���N���X
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

#include <string>
using namespace std;

namespace YNBaseLib {

//******************************************************************************
// �G���[���N���X
//******************************************************************************
class YNBASELIB_API YNErrInfo
{
public:

	//�G���[���x��
	enum ErrLevel {
		LVL_ERR,
		LVL_WARN,
		LVL_INFO
	};

	//�R���X�g���N�^�^�f�X�g���N�^
	YNErrInfo(
			ErrLevel errLevel,
			unsigned long lineNo,
			const TCHAR* pFileName,
			const TCHAR* pMessage,
			unsigned long long errInfo1,
			unsigned long long errInfo2
		);
	virtual ~YNErrInfo(void);

	//�G���[���x���擾
	ErrLevel GetErrLevel();

	//�s�ԍ��擾
	unsigned long GetLineNo();

	//�֐����擾
	const TCHAR* GetFuncName();

	//���b�Z�[�W�擾
	const TCHAR* GetMessage();

	//�G���[���擾
	unsigned long long GetErrInfo1();
	unsigned long long GetErrInfo2();

private:

	ErrLevel m_ErrLevel;
	unsigned long m_LineNo;
	unsigned long long m_ErrInfo1;
	unsigned long long m_ErrInfo2;

//CRT���X�^�e�B�b�N�����N(/MT)����ƌx�����o��
//#pragma warning(disable:4251)
#ifdef _UNICODE
	wstring m_FuncName;
	wstring m_Message;
#else
	string m_FuncName;
	string m_Message;
#endif
//#pragma warning(default:4251)

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const YNErrInfo&);
	YNErrInfo(const YNErrInfo&);

};

} // end of namespace

