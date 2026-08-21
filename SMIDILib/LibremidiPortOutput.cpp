//******************************************************************************
//
// Simple MIDI Library / LibremidiPortOutput
//
// IPortOutput implementation wrapping libremidi::midi_out.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "LibremidiPortOutput.h"

using namespace YNBaseLib;

namespace SMIDILib {

//******************************************************************************
// Constructor
//******************************************************************************
LibremidiPortOutput::LibremidiPortOutput(std::unique_ptr<libremidi::midi_out> pMidiOut)
	: m_pMidiOut(std::move(pMidiOut))
{
}

//******************************************************************************
// Destructor
//******************************************************************************
LibremidiPortOutput::~LibremidiPortOutput()
{
}

//******************************************************************************
// Send short message
//******************************************************************************
int LibremidiPortOutput::SendShort(unsigned long msg)
{
	if (m_pMidiOut == nullptr) return -1;

	// Unpack DWORD into individual MIDI bytes
	unsigned char b0 = static_cast<unsigned char>(msg & 0xFF);
	unsigned char b1 = static_cast<unsigned char>((msg >> 8) & 0xFF);
	unsigned char b2 = static_cast<unsigned char>((msg >> 16) & 0xFF);

	auto err = m_pMidiOut->send_message(b0, b1, b2);
	if (err.is_set()) {
		return -1;
	}
	return 0;
}

//******************************************************************************
// Send long message (SysEx)
//******************************************************************************
int LibremidiPortOutput::SendLong(unsigned char* pMsg, unsigned long size)
{
	if (m_pMidiOut == nullptr) return -1;

	auto err = m_pMidiOut->send_message(pMsg, static_cast<size_t>(size));
	if (err.is_set()) {
		return -1;
	}
	return 0;
}

//******************************************************************************
// Note off all channels
//******************************************************************************
int LibremidiPortOutput::NoteOffAll()
{
	if (m_pMidiOut == nullptr) return -1;

	// CC#123 (All Notes Off) on all 16 channels
	for (int ch = 0; ch < 16; ch++) {
		auto err = m_pMidiOut->send_message(
			static_cast<unsigned char>(0xB0 | ch),
			static_cast<unsigned char>(0x7B),
			static_cast<unsigned char>(0x00)
		);
		if (err.is_set()) {
			return -1;
		}
	}
	return 0;
}

//******************************************************************************
// Sound off all channels
//******************************************************************************
int LibremidiPortOutput::SoundOffAll()
{
	if (m_pMidiOut == nullptr) return -1;

	// CC#120 (All Sound Off) on all 16 channels
	for (int ch = 0; ch < 16; ch++) {
		auto err = m_pMidiOut->send_message(
			static_cast<unsigned char>(0xB0 | ch),
			static_cast<unsigned char>(0x78),
			static_cast<unsigned char>(0x00)
		);
		if (err.is_set()) {
			return -1;
		}
	}
	return 0;
}

} // end of namespace
