//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// Command-line argument parser.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

//******************************************************************************
// Parameter definitions
//******************************************************************************
//Switch state
#define CMDSW_NONE		(0)	//Undefined
#define CMDSW_ON		(1)	//ON

//Switch type
#define CMDSW_FILE_PATH	(0)	//File path
#define CMDSW_PLAY		(1)	//Play
#define CMDSW_QUIET		(2)	//Quit
#define CMDSW_DEBUG		(3)	//Debug mode
#define CMDSW_MAX		(4)	//Terminator flag: must always be defined last


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

private:

	unsigned char m_CmdSwitchStatus[CMDSW_MAX];
	const WCHAR* m_pFilePath;

	int _AnalyzeCmdLine();

};


