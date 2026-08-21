//******************************************************************************
//
// Simple MIDI Library / LibremidiPortOutput
//
// IPortOutput implementation wrapping libremidi::midi_out.
// Extracts the existing send logic from SMOutDevCtrl into a reusable class.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "IPortOutput.h"
#include <libremidi/libremidi.hpp>
#include <memory>

namespace SMIDILib {

//******************************************************************************
// LibremidiPortOutput
//******************************************************************************
class LibremidiPortOutput : public IPortOutput
{
public:

	// Takes ownership of an opened midi_out instance
	explicit LibremidiPortOutput(std::unique_ptr<libremidi::midi_out> pMidiOut);
	virtual ~LibremidiPortOutput();

	int SendShort(unsigned long msg) override;
	int SendLong(unsigned char* pMsg, unsigned long size) override;
	int NoteOffAll() override;
	int SoundOffAll() override;

private:

	std::unique_ptr<libremidi::midi_out> m_pMidiOut;

	LibremidiPortOutput(const LibremidiPortOutput&);
	void operator=(const LibremidiPortOutput&);
};

} // end of namespace
