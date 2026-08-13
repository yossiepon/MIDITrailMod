//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// Command-line argument parser.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMRcpConv.h"
#include "MTCmdLineParser.h"
#include <tchar.h>
#include <stdlib.h>
#include <shellapi.h>

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// Constructor
//******************************************************************************
MTCmdLineParser::MTCmdLineParser(void)
{
	m_pFilePath = L"";
	ZeroMemory(m_CmdSwitchStatus, sizeof(unsigned char)*CMDSW_MAX);
}

//******************************************************************************
// Destructor
//******************************************************************************
MTCmdLineParser::~MTCmdLineParser(void)
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTCmdLineParser::Initialize()
{
	int result = 0;

	//Parse command line
	result = _AnalyzeCmdLine();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Parse command line
//******************************************************************************
int MTCmdLineParser::_AnalyzeCmdLine()
{
	int result = 0;
	int i = 0;
	int argc = 0;
	LPWSTR* pArgList = NULL;
	WCHAR* pArg = NULL;
	SMRcpConv rcpConv;

	//Prepare an RCP file conversion object to check whether RCP loading is available
	result = rcpConv.Initialize();
	if (result != 0) goto EXIT;

	//Get the argument list
	pArgList = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (pArgList == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//Parse the arguments
	for (i = 1; i < argc; i++) {
		pArg = pArgList[i];

		//File path
		//  If multiple file paths are specified, only the first one is used
		if ((wcslen(m_pFilePath) == 0) && (wcslen(pArg) > 4)) {
			if (YNPathUtil::IsFileExtMatch(pArg, L".mid")) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
			//If rcpcv.dll is available, additionally check whether the file is a supported type
			else if (rcpConv.IsAvailable() && rcpConv.IsSupportFileExt(pArg)) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
		}
		//Start playback after launch
		if (wcscmp(pArg, L"-p") == 0) {
			m_CmdSwitchStatus[CMDSW_PLAY] = CMDSW_ON;
		}
		//Exit the app when playback ends
		if (wcscmp(pArg, L"-q") == 0) {
			m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_ON;
		}
		//Debug mode
		if (wcscmp(pArg, L"-d") == 0) {
			m_CmdSwitchStatus[CMDSW_DEBUG] = CMDSW_ON;
		}
	}

	//When no file path is specified
	if (m_CmdSwitchStatus[CMDSW_FILE_PATH] != CMDSW_ON) {
		//Both the play and quit flags are disabled
		m_CmdSwitchStatus[CMDSW_PLAY] = CMDSW_NONE;
		m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_NONE;
	}

	//If the play flag is not ON, the quit flag is disabled
	if (m_CmdSwitchStatus[CMDSW_PLAY] != CMDSW_ON) {
		m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_NONE;
	}

EXIT:;
	if (pArgList != NULL) {
		LocalFree(pArgList);
	}
	return result;
}

//******************************************************************************
// Get switch state
//******************************************************************************
int MTCmdLineParser::GetSwitch(
		unsigned long switchType
	)
{
	int switchStatus = CMDSW_NONE;

	if (switchType < CMDSW_MAX) {
		switchStatus = m_CmdSwitchStatus[switchType];
	}

	return switchStatus;
}

//******************************************************************************
// Get file path
//******************************************************************************
const WCHAR* MTCmdLineParser::GetFilePath()
{
	return m_pFilePath;
}


