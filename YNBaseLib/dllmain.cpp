//******************************************************************************
//
// Simple Base Library / DllMain
//
// DLL�G���g���[�|�C���g
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNErrCtrl.h"

using namespace YNBaseLib;

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
			YNErrCtrl::Initialize();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			YNErrCtrl::Terminate();
			break;
	}
	return TRUE;
}

