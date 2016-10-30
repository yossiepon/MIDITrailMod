//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrl
//
// MIDI�o�̓f�o�C�X����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "mmsystem.h"
#include <string>
#include <list>

#pragma warning(disable:4251)

namespace SMIDILib {

//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő�|�[�g���FA,B,C,D,E,F
#define SM_MIDIOUT_PORT_NUM_MAX   (6)

//******************************************************************************
// MIDI�o�̓f�o�C�X����N���X
//******************************************************************************
class SMIDILIB_API SMOutDevCtrl
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMOutDevCtrl(void);
	virtual ~SMOutDevCtrl(void);

	//������
	int Initialize();

	//�f�o�C�X���擾
	unsigned long GetDevNum();

	//�f�o�C�X�v���_�N�g���̎擾
	int GetDevProductName(unsigned long index, std::string& name);

	//�|�[�g�Ή��f�o�C�X�o�^
	int SetPortDev(unsigned char portNo, const char* pProductName);

	//�|�[�g�Ή��f�o�C�XID�擾
	int GetPortDevId(unsigned char portNo, unsigned long* pDevId);

	//�S�f�o�C�X�̃I�[�v���^�N���[�Y
	int OpenPortDevAll();
	int ClosePortDevAll();

	//�|�[�g���N���A
	int ClearPortInfo();

	//MIDI�o�̓��b�Z�[�W���M
	int SendShortMsg(unsigned char portNo, unsigned long msg);
	int SendLongMsg(unsigned char portNo, unsigned char* pMsg, unsigned long size);
	int NoteOffAll();

private:

	typedef struct {
		bool isExist;
		unsigned long devId;
		HMIDIOUT hMIDIOut;
	} SMPortInfo;

	SMPortInfo m_PortInfo[SM_MIDIOUT_PORT_NUM_MAX];

	typedef struct {
		unsigned long devId;
		char productName[MAXPNAMELEN];
	} SMOutDevInfo;

	typedef std::list<SMOutDevInfo> SMOutDevList;
	typedef std::list<SMOutDevInfo>::iterator SMOutDevListItr;

	SMOutDevList m_OutDevList;

	int _InitDevList();

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMOutDevCtrl&);
	SMOutDevCtrl(const SMOutDevCtrl&);

};

} // end of namespace

#pragma warning(default:4251)

