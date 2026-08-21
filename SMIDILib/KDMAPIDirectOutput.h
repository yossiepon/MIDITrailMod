//******************************************************************************
//
// Simple MIDI Library / KDMAPIDirectOutput
//
// IPortOutput implementation for OmniMIDI Mod (KDMAPI 128ch direct output).
// Loads OmniMIDI.dll at runtime and calls SendDirectDataMultiPort for
// extended channel routing (port * 16 + channel).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "IPortOutput.h"
#include <windows.h>

namespace SMIDILib {

//******************************************************************************
// KDMAPIDirectOutput
//******************************************************************************
class KDMAPIDirectOutput : public IPortOutput
{
public:

	KDMAPIDirectOutput(unsigned char portIndex);
	virtual ~KDMAPIDirectOutput();

	static bool LoadDLL();
	static void UnloadDLL();
	static bool IsForkDetected();
	static bool InitializeStream();
	static void TerminateStream();
	static unsigned long GetPortCount();

	int SendShort(unsigned long msg) override;
	int SendLong(unsigned char* pMsg, unsigned long size) override;
	int NoteOffAll() override;
	int SoundOffAll() override;

private:

	unsigned char m_portIndex;

	// Function pointer types
	typedef BOOL(WINAPI* IsKDMAPIAvailable_t)();
	typedef BOOL(WINAPI* InitializeKDMAPIStream_t)();
	typedef VOID(WINAPI* TerminateKDMAPIStream_t)();
	typedef VOID(WINAPI* SendDirectDataMultiPort_t)(DWORD dwMsg, BYTE port);
	typedef UINT(WINAPI* SendDirectLongDataMultiPort_t)(LPSTR data, DWORD len, BYTE port);
	typedef VOID(WINAPI* ResetKDMAPIStreamMultiPort_t)(BYTE port);

	// Shared state (all instances share the same DLL)
	static HMODULE s_hDLL;
	static int s_streamRefCount;
	static bool s_forkDetected;

	static IsKDMAPIAvailable_t s_pfnIsKDMAPIAvailable;
	static InitializeKDMAPIStream_t s_pfnInitializeKDMAPIStream;
	static TerminateKDMAPIStream_t s_pfnTerminateKDMAPIStream;
	static SendDirectDataMultiPort_t s_pfnSendDirectDataMultiPort;
	static SendDirectLongDataMultiPort_t s_pfnSendDirectLongDataMultiPort;
	static ResetKDMAPIStreamMultiPort_t s_pfnResetKDMAPIStreamMultiPort;

	KDMAPIDirectOutput(const KDMAPIDirectOutput&);
	void operator=(const KDMAPIDirectOutput&);
};

} // end of namespace
