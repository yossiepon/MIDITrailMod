//******************************************************************************
//
// Simple MIDI Library / SMLiveMonitor
//
// ���C�u���j�^�N���X
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include "SMMsgQueue.h"
#include "SMMsgTransmitter.h"
#include "SMInDevCtrl.h"
#include "SMOutDevCtrl.h"
#include "SMEventWatcher.h"

namespace SMIDILib {


//******************************************************************************
// �p�����[�^��`
//******************************************************************************


//******************************************************************************
// ���C�u���j�^�N���X
//******************************************************************************
class SMIDILIB_API SMLiveMonitor
{
public:
	
	//���t���
	enum Status {
		StatusMonitorOFF,
		StatusMonitorON
	};
	
	//�R���X�g���N�^�^�f�X�g���N�^
	SMLiveMonitor(void);
	virtual ~SMLiveMonitor(void);
	
	//������
	int Initialize(SMMsgQueue* pMsgQueue);
	
	//�|�[�g�Ή��f�o�C�X�o�^
	int SetInPortDev(const char* pProductName, bool isMIDITHRU);
	int SetOutPortDev(const char* pProductName);
	
	//���̓|�[�g�f�o�C�X�\�����擾
	//NSString* GetInPortDevDisplayName(NSString* pIdName);
	int GetInPortDevDisplayName(std::string& name);
	
	//���j�^�J�n
	int Start();
	
	//���j�^��~
	int Stop();
	
private:
	
	//���t���
	Status m_Status;
	SMMsgTransmitter m_MsgTrans;
	SMMsgQueue* m_pMsgQue;
	SMEventWatcher m_EventWatcher;
	
	//MIDI�f�o�C�X�n
	char m_InPortDevName[MAXPNAMELEN];
	char m_OutPortDevName[MAXPNAMELEN];
	bool m_isMIDITHRU;
	SMInDevCtrl m_InDevCtrl;
	SMOutDevCtrl m_OutDevCtrl;
	
	//�|�[�g����
	void _ClearPortInfo();
	int _OpenMIDIDev();
	int _CloseMIDIDev();
	
	static int _InReadCallBack(SMEvent* pEvent, void* pUserParam);
	int _InReadProc(SMEvent* pEvent);
	int _InReadProcParseEvent(SMEvent* pEvent);
	int _InReadProcMIDITHRU(SMEvent* pEvent);
	int _InReadProcSendMIDIEvent(unsigned char portNo, SMEvent* pEvent);
	int _InReadProcSendSysExEvent(unsigned char portNo, SMEvent* pEvent);
	int _InReadProcSendSysMsgEvent(unsigned char portNo, SMEvent* pEvent);
	
};

} // end of namespace


