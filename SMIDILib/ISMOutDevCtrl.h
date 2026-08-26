//******************************************************************************
//
// Simple MIDI Library / ISMOutDevCtrl
//
// MIDI output device control interface.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <string>

namespace SMIDILib {

//******************************************************************************
// MIDI output transport type
//******************************************************************************
enum class SMTransportType : int
{
	None = 0,
	WinMM = 1,
	KDMAPI = 2,
	KDMAPIMod = 3,
};

//******************************************************************************
// MIDI output device control interface
//******************************************************************************
class SMIDILIB_API ISMOutDevCtrl
{
public:

	virtual ~ISMOutDevCtrl() = default;

	virtual int Initialize() = 0;
	virtual unsigned long GetDevNum() = 0;
	virtual int GetDevProductName(unsigned long index, std::string& name) = 0;
	virtual int SetPortDev(unsigned char portNo, const char* pProductName) = 0;
	virtual int GetPortDevId(unsigned char portNo, unsigned long* pDevId) = 0;
	virtual int OpenPortDevAll() = 0;
	virtual int ClosePortDevAll() = 0;
	virtual int ClearPortInfo() = 0;
	virtual int SendShortMsg(unsigned char portNo, unsigned long msg) = 0;
	virtual int SendLongMsg(unsigned char portNo, unsigned char* pMsg, unsigned long size) = 0;
	virtual int NoteOffAll() = 0;
	virtual int SoundOffAll() = 0;
	virtual SMTransportType GetTransportType() const = 0;

};

} // end of namespace
