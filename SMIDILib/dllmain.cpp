//******************************************************************************
//
// Simple MIDI Library / DllMain
//
// DLL�G���g���|�C���g
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"


//******************************************************************************
// �G���g���[�|�C���g
//******************************************************************************
BOOL APIENTRY DllMain(
		HMODULE hModule,
		DWORD  ul_reason_for_call,
		LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

