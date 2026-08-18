//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// Command-line argument parser.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <string>

//******************************************************************************
// Parameter definitions
//******************************************************************************
//Switch state
#define CMDSW_NONE		(0)	//Undefined
#define CMDSW_ON		(1)	//ON

//Switch type
#define CMDSW_FILE_PATH		(0)	//File path
#define CMDSW_PLAY			(1)	//Play
#define CMDSW_QUIET			(2)	//Quit
#define CMDSW_DUMP_MIDI		(3)	//MIDI file dump (--dump-midi / -d)
#define CMDSW_LOG_LEVEL		(4)	//Log level override (--log-level <level>)
#define CMDSW_MAX			(5)	//Terminator flag: must always be defined last


//******************************************************************************
// Command-line parser class
//******************************************************************************
class MTCmdLineParser
{
public:

	//Constructor / Destructor
	MTCmdLineParser(void);
	virtual ~MTCmdLineParser(void);

	//Initialize
	int Initialize();

	//Get switch state
	int GetSwitch(unsigned long switchType);

	//Get file path
	const WCHAR* GetFilePath();

	//Get log level override value
	const WCHAR* GetLogLevel();

private:

	unsigned char m_CmdSwitchStatus[CMDSW_MAX];
	const WCHAR* m_pFilePath;
	std::wstring m_LogLevel;

	int _AnalyzeCmdLine();

};
