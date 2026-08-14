//******************************************************************************
//
// MIDITrail / MIDITrailMain
//
// Application entry point.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MIDITrailApp.h"
#include "MIDITrailMain.h"

using namespace YNBaseLib;


//******************************************************************************
// Entry point
//******************************************************************************
int APIENTRY _tWinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPTSTR lpCmdLine,
		int nCmdShow
	)
{
	int result = 0;
	int winMainResult = 0;
	MIDITrailApp app;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	result = app.Initialize(hInstance, lpCmdLine, nCmdShow);
	if (result != 0) {
		YN_SHOW_ERR(NULL);
		winMainResult = 0;
		goto EXIT;
	}

	winMainResult = app.Run();

EXIT:;
	app.Terminate();
	return winMainResult;
}
