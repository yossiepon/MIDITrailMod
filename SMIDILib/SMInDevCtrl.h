//******************************************************************************
//
// Simple MIDI Library / SMInDevCtrl
//
// MIDI���̓f�o�C�X����N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <list>
#include "mmsystem.h"
#include "SMEvent.h"

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//MIDI�C�x���g�ǂݍ��݃R�[���o�b�N�֐�
typedef int (*SMInReadCallBack)(SMEvent* pEvent, void* pUserParam);

//�V�X�e���G�N�X�N���[�V�u�p�o�b�t�@�T�C�Y
//  �T�C�Y�̍����͓��ɂȂ�
#define SM_MIDIIN_BUF_SIZE  (1024 * 10)


//******************************************************************************
// MIDI���̓f�o�C�X����N���X
//******************************************************************************
class SMIDILIB_API SMInDevCtrl
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	SMInDevCtrl(void);
	virtual ~SMInDevCtrl(void);
	
	//������
	int Initialize();
	
	//�f�o�C�X���擾
	unsigned long GetDevNum();
	
	//�f�o�C�X�v���_�N�g���̎擾
	int GetDevProductName(unsigned long index, std::string& name);
	
	//�|�[�g�Ή��f�o�C�X�o�^
	int SetPortDev(const char* pProductName);
	
	//MIDI�C�x���g�ǂݍ��݃R�[���o�b�N�֐��o�^
	void SetInReadCallBack(SMInReadCallBack pCallBack, void* pUserParam);
	
	//�S�f�o�C�X�̃I�[�v���^�N���[�Y
	int OpenPortDev();
	int ClosePortDev();
	
	//�|�[�g���N���A
	int ClearPortInfo();
	
private:
	
	//�|�[�g���
	typedef struct {
		bool isExist;
		unsigned long devId;
		HMIDIIN hMidiIn;
		MIDIHDR midiHdr;
	} SMPortInfo;
	
	//�f�o�C�X���
	typedef struct {
		unsigned long devId;
		char productName[MAXPNAMELEN];
	} SMInDevInfo;
	
	//���̓f�o�C�X���X�g
	typedef std::list<SMInDevInfo> SMInDevList;
	typedef std::list<SMInDevInfo>::iterator SMInDevListItr;
	SMInDevList m_InDevList;
	
	//�|�[�g���
	SMPortInfo m_PortInfo;
	
	//�R�[���o�b�N�֐�
	SMInReadCallBack m_pInReadCallBack;
	void* m_pCallBackUserParam;
	
	//�p�P�b�g��͌n
	bool m_isContinueSysEx;
	
	int _InitDevList();
	static void CALLBACK _InReadCallBack(
			HMIDIIN hMidiIn,
			UINT wMsg,
			DWORD dwInstance,
			DWORD dwParam1,
			DWORD dwParam2
		);
	void _InReadProc(
			HMIDIIN hMidiIn,
			UINT wMsg,
			DWORD dwParam1,
			DWORD dwParam2
		);
	int _InReadProcMIDI(
			DWORD midiMessage,
			DWORD timestamp,
			SMEvent* pEvent
		);
	int _InReadProcSysEx(
			MIDIHDR* pMIDIHDR,
			DWORD timestamp,
			bool* pIsContinueSysEx,
			SMEvent* pEvent
		);
	unsigned long _GetMIDIMsgSize(unsigned char status);
	unsigned long _GetSysMsgSize(unsigned char status);
	
};

} // end of namespace

#pragma warning(default:4251)


