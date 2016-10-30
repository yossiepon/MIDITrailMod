//******************************************************************************
//
// Simple MIDI Library / SMMsgQueue
//
// ���b�Z�[�W�L���[�N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "SMMsgQueue.h"

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMMsgQueue::SMMsgQueue(void)
 : m_List(sizeof(unsigned long)*2, 10000)
{
	InitializeCriticalSection(&m_CriticalSection);
	m_MaxMsgNum = 0;
	m_NextPostIndex = 0;
	m_NextReadIndex = 0;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMMsgQueue::~SMMsgQueue(void)
{
	DeleteCriticalSection(&m_CriticalSection);
}

//******************************************************************************
// �o�b�t�@�쐬
//******************************************************************************
int SMMsgQueue::Initialize(
		unsigned long maxMsgNum
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long dummy[2] = {0, 0};
	
	//�쐬�ς݂Ȃ牽�����Ȃ�
	if (m_List.GetSize() > 0) goto EXIT;
	
	for (index = 0; index < maxMsgNum; index++) {
		result = m_List.AddItem(dummy);
		if (result != 0) goto EXIT;
	}
	m_MaxMsgNum = maxMsgNum;
	
EXIT:;
	return result;
}

//******************************************************************************
// ���b�Z�[�W�o�^
//******************************************************************************
int SMMsgQueue::PostMessage(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	unsigned long params[2] = {0, 0};
	
	EnterCriticalSection(&m_CriticalSection);
	
	params[0] = param1;
	params[1] = param2;
	
	//�p�����[�^�o�^
	result = m_List.SetItem(m_NextPostIndex, params);
	if (result != 0) goto EXIT;
	
	//����ǂݍ��݈ʒu���X�V
	m_NextPostIndex++;
	if (m_NextPostIndex == m_MaxMsgNum) {
		m_NextPostIndex = 0;
	}
	
	//�ǂݍ��݂��Ă��Ȃ��ł��Â��f�[�^���㏑���ɂ���Ď̂Ă�ꂽ�ꍇ
	if (m_NextPostIndex == m_NextReadIndex) {
		//�ǂݍ��݈ʒu���J��グ��i�̂Ă�ꂽ�f�[�^�͖�������j
		m_NextReadIndex++;
		if (m_NextReadIndex == m_MaxMsgNum) {
			m_NextReadIndex = 0;
		}
	}
	
EXIT:;
	LeaveCriticalSection(&m_CriticalSection);
	return result;
}

//******************************************************************************
// ���b�Z�[�W�擾
//******************************************************************************
int SMMsgQueue::GetMessage(
		bool* pIsExist,
		unsigned long* pParam1,
		unsigned long* pParam2
	)
{
	int result = 0;
	unsigned long params[2] = {0, 0};
	
	EnterCriticalSection(&m_CriticalSection);
	
	*pIsExist = false;
	
	//���b�Z�[�W����̏ꍇ
	if (m_NextReadIndex == m_NextPostIndex) goto EXIT;
	
	//�p�����[�^�擾
	result = m_List.GetItem(m_NextReadIndex, params);
	if (result != 0) goto EXIT;
	
	*pParam1 = params[0];
	*pParam2 = params[1];
	
	//����ǂݎ��ʒu���X�V
	m_NextReadIndex++;
	if (m_NextReadIndex == m_MaxMsgNum) {
		m_NextReadIndex = 0;
	}
	
	*pIsExist = true;
	
EXIT:;
	LeaveCriticalSection(&m_CriticalSection);
	return result;
}

} // end of namespace

