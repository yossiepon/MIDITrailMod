//******************************************************************************
//
// Simple MIDI Library / SMEvent
//
// �C�x���g�N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

namespace SMIDILib {

//******************************************************************************
// �p�����[�^��`
//******************************************************************************
#define SMEVENT_INTERNAL_DATA_SIZE  (16)


//******************************************************************************
// �C�x���g�N���X
//******************************************************************************
class SMIDILIB_API SMEvent
{
public:

	//�C�x���g���
	enum EventType {
		EventNone,
		EventMIDI,
		EventSysEx,
		EventSysMsg,
		EventMeta
	};

	//�R���X�g���N�^�^�f�X�g���N�^
	SMEvent(void);
	virtual ~SMEvent(void);

	//�f�[�^�o�^
	int SetData(EventType type, unsigned char status, unsigned char meta, unsigned char* pData, unsigned long size);

	//MIDI�C�x���g�o�^
	int SetMIDIData(unsigned char status, unsigned char* pData, unsigned long size);

	//SysEx�C�x���g�o�^
	int SetSysExData(unsigned char status, unsigned char* pData, unsigned long size);

	//SysMsg�C�x���g�o�^
	int SetSysMsgData(unsigned char status, unsigned char* pData, unsigned long size);

	//���^�C�x���g�o�^
	int SetMetaData(unsigned char status, unsigned char type, unsigned char* pData, unsigned long size);

	//�C�x���g��ʎ擾
	EventType GetType();

	//�X�e�[�^�X�擾
	unsigned char GetStatus();

	//�X�e�[�^�X�ݒ�
	void SetStatus(unsigned char status);

	//���^��ʎ擾
	unsigned char GetMetaType();

	//�f�[�^�T�C�Y�擾
	unsigned long GetDataSize();

	//�f�[�^�|�C���^�擾
	unsigned char* GetDataPtr();

	//�N���A
	void Clear();

private:

	EventType m_Type;
	unsigned char m_Status;
	unsigned char m_MetaType;
	unsigned long m_DataSize;
	unsigned char m_Data[SMEVENT_INTERNAL_DATA_SIZE];
	unsigned char* m_pExData;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMEvent&);
	SMEvent(const SMEvent&);

};

} // end of namespace

