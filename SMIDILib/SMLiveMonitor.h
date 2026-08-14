//******************************************************************************
//
// Simple MIDI Library / SMLiveMonitor
//
// Live (real-time input) monitor class.
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
// Parameter definitions
//******************************************************************************


//******************************************************************************
// Live monitor class
//******************************************************************************
class SMIDILIB_API SMLiveMonitor
{
public:
	
	//Playback state
	enum Status {
		StatusMonitorOFF,
		StatusMonitorON
	};
	
	//Constructor / Destructor
	SMLiveMonitor(void);
	virtual ~SMLiveMonitor(void);
	
	//Initialize
	int Initialize(SMMsgQueue* pMsgQueue);
	
	//Register device for port
	int SetInPortDev(const char* pProductName, bool isMIDITHRU);
	int SetOutPortDev(const char* pProductName);
	
	//Get input port device display name
	//NSString* GetInPortDevDisplayName(NSString* pIdName);
	int GetInPortDevDisplayName(std::string& name);
	
	//Start monitor
	int Start();
	
	//Stop monitor
	int Stop();
	
private:
	
	//Playback state
	Status m_Status;
	SMMsgTransmitter m_MsgTrans;
	SMMsgQueue* m_pMsgQue;
	SMEventWatcher m_EventWatcher;
	
	//MIDI device related
	char m_InPortDevName[MAXPNAMELEN];
	char m_OutPortDevName[MAXPNAMELEN];
	bool m_isMIDITHRU;
	SMInDevCtrl m_InDevCtrl;
	SMOutDevCtrl m_OutDevCtrl;
	
	//Port control
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


