//******************************************************************************
//
// Simple MIDI Library / IPortOutput
//
// Port output interface for MIDI device abstraction.
// Implementations: LibremidiPortOutput (libremidi backend),
//                  KDMAPIDirectOutput (OmniMIDI KDMAPI direct backend).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

namespace SMIDILib {

//******************************************************************************
// IPortOutput
//******************************************************************************
class IPortOutput
{
public:

	virtual ~IPortOutput() {}

	// Send short MIDI message (packed DWORD: status | data1<<8 | data2<<16)
	virtual int SendShort(unsigned long msg) = 0;

	// Send long MIDI message (SysEx)
	virtual int SendLong(unsigned char* pMsg, unsigned long size) = 0;

	// Send Note Off (CC#123) on all 16 channels
	virtual int NoteOffAll() = 0;

	// Send All Sound Off (CC#120) on all 16 channels
	virtual int SoundOffAll() = 0;
};

} // end of namespace
