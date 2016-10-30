//******************************************************************************
//
// Simple MIDI Library / SMEvent
//
// �C�x���g�N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEvent.h"
#include <new>

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMEvent::SMEvent(void)
{
	m_pExData = NULL;
	Clear();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMEvent::~SMEvent(void)
{
	delete [] m_pExData;
}

//******************************************************************************
// �f�[�^�o�^
//******************************************************************************
int SMEvent::SetData(
		EventType type,
		unsigned char status,
		unsigned char meta,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	m_Type = type;
	m_Status = status;
	m_MetaType = meta;

	delete [] m_pExData;
	m_pExData = NULL;
	m_DataSize = 0;
	ZeroMemory(m_Data, SMEVENT_INTERNAL_DATA_SIZE);

	if (size == 0) {
		//�������Ȃ�
	}
	else if (size <= SMEVENT_INTERNAL_DATA_SIZE) {
		memcpy(m_Data, pData, size);
	}
	else {
		try {
			m_pExData = new unsigned char[size];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", size, 0);
			goto EXIT;
		}
		memcpy(m_pExData, pData, size);
	}

	m_DataSize = size;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�C�x���g�f�[�^�o�^
//******************************************************************************
int SMEvent::SetMIDIData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	result = SetData(EventMIDI, status, 0, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// SysEx�C�x���g�f�[�^�o�^
//******************************************************************************
int SMEvent::SetSysExData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	unsigned char* pExData = NULL;

	//�X�e�[�^�X 0xF0 �̏ꍇ�͐擪�p�P�b�g
	// �� �擪�� 0xF0 �����đ��M����
	if (status == 0xF0) {
		try {
			pExData = new unsigned char[size + 1];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", size + 1, 0);
			goto EXIT;
		}
		pExData[0] = status;
		memcpy(&(pExData[1]), pData, size);
		result = SetData(EventSysEx, status, 0, pExData, size + 1);
		if (result != 0) goto EXIT;
	}
	//�X�e�[�^�X 0xF7 �̏ꍇ�͌㑱�p�P�b�g
	// �� �擪�� 0xF7 �����đ��M���Ȃ�
	else if (status == 0xF7) {
		result = SetData(EventSysEx, status, 0, pData, size);
		if (result != 0) goto EXIT;
	}
	//����ȊO�̓G���[
	else {
		result = YN_SET_ERR("Program error.", status, 0);
		goto EXIT;
	}

EXIT:;
	delete [] pExData;
	return result;
}

//******************************************************************************
// SysMsg�C�x���g�f�[�^�o�^
//******************************************************************************
int SMEvent::SetSysMsgData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	
	result = SetData(EventSysMsg, status, 0, pData, size);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// ���^�C�x���g�f�[�^�o�^
//******************************************************************************
int SMEvent::SetMetaData(
		unsigned char status,
		unsigned char type,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	result = SetData(EventMeta, status, type, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �C�x���g��ʎ擾
//******************************************************************************
SMEvent::EventType SMEvent::GetType()
{
	return m_Type;
}

//******************************************************************************
// �X�e�[�^�X�擾
//******************************************************************************
unsigned char SMEvent::GetStatus()
{
	return m_Status;
}

//******************************************************************************
// ���^�C�x���g��ʎ擾
//******************************************************************************
unsigned char SMEvent::GetMetaType()
{
	return m_MetaType;
}

//******************************************************************************
// �f�[�^�T�C�Y�擾
//******************************************************************************
unsigned long SMEvent::GetDataSize()
{
	return m_DataSize;
}

//******************************************************************************
// �f�[�^�ʒu�擾
//******************************************************************************
unsigned char* SMEvent::GetDataPtr()
{
	unsigned char* pData = NULL;

	if (m_DataSize <= SMEVENT_INTERNAL_DATA_SIZE) {
		pData = m_Data;
	}
	else {
		pData = m_pExData;
	}

	return pData;
}

//******************************************************************************
// �N���A
//******************************************************************************
void SMEvent::Clear()
{
	delete [] m_pExData;
	
	m_Type = EventNone;
	m_Status = 0;
	m_MetaType = 0;
	m_DataSize = 0;
	memset(m_Data, 0, SMEVENT_INTERNAL_DATA_SIZE);
	m_pExData = NULL;
}

} // end of namespace


