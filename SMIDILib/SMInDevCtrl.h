//******************************************************************************
//
// Simple MIDI Library / SMInDevCtrl
//
// MIDI input device control class.
//
// Copyright (C) 2012-2014 WADA Masashi. All Rights Reserved.
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
// Parameter definitions
//******************************************************************************
//MIDI event read callback function
typedef int (*SMInReadCallBack)(SMEvent* pEvent, void* pUserParam);

//Buffer size for system exclusive messages
//  No particular basis for this size
#define SM_MIDIIN_BUF_SIZE  (1024 * 10)


//******************************************************************************
// MIDI input device control class
//******************************************************************************
class SMIDILIB_API SMInDevCtrl
{
public:
	
	//Constructor / Destructor
	SMInDevCtrl(void);
	virtual ~SMInDevCtrl(void);
	
	//Initialize
	int Initialize();
	
	//Get device count
	unsigned long GetDevNum();
	
	//Get device product name
	int GetDevProductName(unsigned long index, std::string& name);
	
	//Register port-associated device
	int SetPortDev(const char* pProductName);
	
	//Register MIDI event read callback function
	void SetInReadCallBack(SMInReadCallBack pCallBack, void* pUserParam);
	
	//Open/close all devices
	int OpenPortDev();
	int ClosePortDev();
	
	//Clear port info
	int ClearPortInfo();
	
private:
	
	//Port info
	typedef struct {
		bool isExist;
		unsigned long devId;
		HMIDIIN hMidiIn;
		MIDIHDR midiHdr;
	} SMPortInfo;
	
	//Device info
	typedef struct {
		unsigned long devId;
		char productName[MAXPNAMELEN];
	} SMInDevInfo;
	
	//Input device list
	typedef std::list<SMInDevInfo> SMInDevList;
	typedef std::list<SMInDevInfo>::iterator SMInDevListItr;
	SMInDevList m_InDevList;
	
	//Port info
	SMPortInfo m_PortInfo;
	
	//Callback function
	SMInReadCallBack m_pInReadCallBack;
	void* m_pCallBackUserParam;
	
	//Packet parsing
	bool m_isContinueSysEx;
	
	int _InitDevList();
	static void CALLBACK _InReadCallBack(
			HMIDIIN hMidiIn,
			UINT wMsg,
			DWORD_PTR dwInstance,
			DWORD_PTR dwParam1,
			DWORD_PTR dwParam2
		);
	void _InReadProc(
			HMIDIIN hMidiIn,
			UINT wMsg,
			DWORD_PTR dwParam1,
			DWORD_PTR dwParam2
		);
	int _InReadProcMIDI(
			DWORD_PTR midiMessage,
			DWORD_PTR timestamp,
			SMEvent* pEvent
		);
	int _InReadProcSysEx(
			MIDIHDR* pMIDIHDR,
			DWORD_PTR timestamp,
			bool* pIsContinueSysEx,
			SMEvent* pEvent
		);
	unsigned long _GetMIDIMsgSize(unsigned char status);
	unsigned long _GetSysMsgSize(unsigned char status);
	
};

} // end of namespace

#pragma warning(default:4251)


