//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrl
//
// MIDI output device control class.
//
// Copyright (C) 2010-2021 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "mmsystem.h"
#include "ISMOutDevCtrl.h"
#include <string>
#include <list>

#pragma warning(disable:4251)

namespace SMIDILib {

//******************************************************************************
// Parameter definitions
//******************************************************************************
//Max port count: A,B,C,D,E,F
#define SM_MIDIOUT_PORT_NUM_MAX   (6)

//******************************************************************************
// MIDI output device control class
//******************************************************************************
class SMIDILIB_API SMOutDevCtrl : public ISMOutDevCtrl
{
public:

	//Constructor / Destructor
	SMOutDevCtrl(void);
	virtual ~SMOutDevCtrl(void);

	//Initialize
	int Initialize();

	//Get device count
	unsigned long GetDevNum();

	//Get device product name
	int GetDevProductName(unsigned long index, std::string& name);

	//Register device for port
	int SetPortDev(unsigned char portNo, const char* pProductName);

	//Get device ID for port
	int GetPortDevId(unsigned char portNo, unsigned long* pDevId);

	//Open/close all devices
	int OpenPortDevAll();
	int ClosePortDevAll();

	//Clear port info
	int ClearPortInfo();

	//Send MIDI output message
	int SendShortMsg(unsigned char portNo, unsigned long msg);
	int SendLongMsg(unsigned char portNo, unsigned char* pMsg, unsigned long size);
	int NoteOffAll();
	int SoundOffAll();

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

	//Prohibit assignment and copy constructor
	void operator=(const SMOutDevCtrl&);
	SMOutDevCtrl(const SMOutDevCtrl&);

};

} // end of namespace

#pragma warning(default:4251)

