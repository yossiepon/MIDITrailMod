//******************************************************************************
//
// Simple MIDI Library / KDMAPIDirectOutput
//
// IPortOutput implementation for OmniMIDI Mod (KDMAPI 128ch direct output).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "KDMAPIDirectOutput.h"

using namespace YNBaseLib;

namespace SMIDILib {

//******************************************************************************
// Static member initialization
//******************************************************************************
HMODULE KDMAPIDirectOutput::s_hDLL = NULL;
int KDMAPIDirectOutput::s_streamRefCount = 0;
bool KDMAPIDirectOutput::s_forkDetected = false;

KDMAPIDirectOutput::IsKDMAPIAvailable_t KDMAPIDirectOutput::s_pfnIsKDMAPIAvailable = nullptr;
KDMAPIDirectOutput::InitializeKDMAPIStream_t KDMAPIDirectOutput::s_pfnInitializeKDMAPIStream = nullptr;
KDMAPIDirectOutput::TerminateKDMAPIStream_t KDMAPIDirectOutput::s_pfnTerminateKDMAPIStream = nullptr;
KDMAPIDirectOutput::SendDirectDataMultiPort_t KDMAPIDirectOutput::s_pfnSendDirectDataMultiPort = nullptr;
KDMAPIDirectOutput::SendDirectLongDataMultiPort_t KDMAPIDirectOutput::s_pfnSendDirectLongDataMultiPort = nullptr;
KDMAPIDirectOutput::ResetKDMAPIStreamMultiPort_t KDMAPIDirectOutput::s_pfnResetKDMAPIStreamMultiPort = nullptr;

//******************************************************************************
// Constructor
//******************************************************************************
KDMAPIDirectOutput::KDMAPIDirectOutput(unsigned char portIndex)
	: m_portIndex(portIndex)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
KDMAPIDirectOutput::~KDMAPIDirectOutput()
{
}

//******************************************************************************
// Load OmniMIDI DLL and resolve function pointers
//******************************************************************************
bool KDMAPIDirectOutput::LoadDLL()
{
	if (s_hDLL != NULL) return true;

	s_hDLL = LoadLibraryA("OmniMIDI.dll");
	if (s_hDLL == NULL) return false;

	s_pfnIsKDMAPIAvailable = (IsKDMAPIAvailable_t)
		GetProcAddress(s_hDLL, "IsKDMAPIAvailable");
	s_pfnInitializeKDMAPIStream = (InitializeKDMAPIStream_t)
		GetProcAddress(s_hDLL, "InitializeKDMAPIStream");
	s_pfnTerminateKDMAPIStream = (TerminateKDMAPIStream_t)
		GetProcAddress(s_hDLL, "TerminateKDMAPIStream");

	if (!s_pfnIsKDMAPIAvailable || !s_pfnInitializeKDMAPIStream || !s_pfnTerminateKDMAPIStream) {
		FreeLibrary(s_hDLL);
		s_hDLL = NULL;
		return false;
	}

	// Fork detection: SendDirectDataMultiPort only exists in our fork
	s_pfnSendDirectDataMultiPort = (SendDirectDataMultiPort_t)
		GetProcAddress(s_hDLL, "SendDirectDataMultiPort");
	s_pfnSendDirectLongDataMultiPort = (SendDirectLongDataMultiPort_t)
		GetProcAddress(s_hDLL, "SendDirectLongDataMultiPort");
	s_pfnResetKDMAPIStreamMultiPort = (ResetKDMAPIStreamMultiPort_t)
		GetProcAddress(s_hDLL, "ResetKDMAPIStreamMultiPort");

	s_forkDetected = (s_pfnSendDirectDataMultiPort != nullptr);

	return true;
}

//******************************************************************************
// Unload OmniMIDI DLL
//******************************************************************************
void KDMAPIDirectOutput::UnloadDLL()
{
	if (s_hDLL == NULL) return;

	s_pfnIsKDMAPIAvailable = nullptr;
	s_pfnInitializeKDMAPIStream = nullptr;
	s_pfnTerminateKDMAPIStream = nullptr;
	s_pfnSendDirectDataMultiPort = nullptr;
	s_pfnSendDirectLongDataMultiPort = nullptr;
	s_pfnResetKDMAPIStreamMultiPort = nullptr;
	s_forkDetected = false;

	FreeLibrary(s_hDLL);
	s_hDLL = NULL;
}

//******************************************************************************
// Fork detection
//******************************************************************************
bool KDMAPIDirectOutput::IsForkDetected()
{
	return s_forkDetected;
}

//******************************************************************************
// Initialize KDMAPI stream (reference counted)
//******************************************************************************
bool KDMAPIDirectOutput::InitializeStream()
{
	if (s_pfnInitializeKDMAPIStream == nullptr) return false;

	if (s_streamRefCount == 0) {
		if (s_pfnIsKDMAPIAvailable && !s_pfnIsKDMAPIAvailable()) {
			return false;
		}
		s_pfnInitializeKDMAPIStream();
	}
	s_streamRefCount++;
	return true;
}

//******************************************************************************
// Terminate KDMAPI stream (reference counted)
//******************************************************************************
void KDMAPIDirectOutput::TerminateStream()
{
	if (s_streamRefCount <= 0) return;

	s_streamRefCount--;
	if (s_streamRefCount == 0 && s_pfnTerminateKDMAPIStream != nullptr) {
		s_pfnTerminateKDMAPIStream();
	}
}

//******************************************************************************
// Get virtual port count
//******************************************************************************
unsigned long KDMAPIDirectOutput::GetPortCount()
{
	return 8;
}

//******************************************************************************
// Send short message
//******************************************************************************
int KDMAPIDirectOutput::SendShort(unsigned long msg)
{
	if (s_pfnSendDirectDataMultiPort == nullptr) return -1;
	s_pfnSendDirectDataMultiPort(static_cast<DWORD>(msg), m_portIndex);
	return 0;
}

//******************************************************************************
// Send long message (SysEx)
//******************************************************************************
int KDMAPIDirectOutput::SendLong(unsigned char* pMsg, unsigned long size)
{
	if (s_pfnSendDirectLongDataMultiPort == nullptr) return -1;
	UINT result = s_pfnSendDirectLongDataMultiPort(
		reinterpret_cast<LPSTR>(pMsg),
		static_cast<DWORD>(size),
		m_portIndex
	);
	return (result == 0) ? 0 : -1;
}

//******************************************************************************
// Note off all channels for this port
//******************************************************************************
int KDMAPIDirectOutput::NoteOffAll()
{
	if (s_pfnResetKDMAPIStreamMultiPort == nullptr) return -1;
	s_pfnResetKDMAPIStreamMultiPort(m_portIndex);
	return 0;
}

//******************************************************************************
// Sound off all channels for this port
//******************************************************************************
int KDMAPIDirectOutput::SoundOffAll()
{
	if (s_pfnResetKDMAPIStreamMultiPort == nullptr) return -1;
	s_pfnResetKDMAPIStreamMultiPort(m_portIndex);
	return 0;
}

} // end of namespace
