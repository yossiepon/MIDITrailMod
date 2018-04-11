//******************************************************************************
//
// Simple MIDI Library / SMMsgQueue
//
// ���b�Z�[�W�L���[�N���X�w�b�_
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMSimpleList.h"

namespace SMIDILib {


//******************************************************************************
// ���b�Z�[�W�L���[�N���X
//******************************************************************************
class SMIDILIB_API SMMsgQueue
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	SMMsgQueue(void);
	virtual ~SMMsgQueue(void);
	
	//������
	int Initialize(unsigned long maxMsgNum);
	
	//���b�Z�[�W�o�^
	int PostMessage(unsigned long param1, unsigned long param2);
	
	//���b�Z�[�W�擾
	int GetMessage(bool* pIsExist, unsigned long* pParam1, unsigned long* pParam2);
	
private:
	
	CRITICAL_SECTION m_CriticalSection;
	
	SMSimpleList m_List;
	unsigned long m_MaxMsgNum;
	unsigned long m_NextPostIndex;
	unsigned long m_NextReadIndex;

};

} // end of namespace

