//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// コマンドライン解析クラス
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
// コンストラクタ
//******************************************************************************
MTCmdLineParser::MTCmdLineParser(void)
{
	m_pFilePath = L"";
	ZeroMemory(m_CmdSwitchStatus, sizeof(unsigned char)*CMDSW_MAX);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTCmdLineParser::~MTCmdLineParser(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTCmdLineParser::Initialize()
{
	int result = 0;

	//コマンドライン解析
	result = _AnalyzeCmdLine();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// コマンドライン解析
//******************************************************************************
int MTCmdLineParser::_AnalyzeCmdLine()
{
	int result = 0;
	int i = 0;
	int argc = 0;
	LPWSTR* pArgList = NULL;
	WCHAR* pArg = NULL;
	SMRcpConv rcpConv;

	//RCP読み込み可否確認のためRCPファイル変換オブジェクトを用意する
	result = rcpConv.Initialize();
	if (result != 0) goto EXIT;

	//引数リスト取得
	pArgList = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (pArgList == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//引数の解析
	for (i = 1; i < argc; i++) {
		pArg = pArgList[i];

		//ファイルパス
		//  ファイルパスが複数指定された場合は先頭のみを採用する
		if ((wcslen(m_pFilePath) == 0) && (wcslen(pArg) > 4)) {
			if (YNPathUtil::IsFileExtMatch(pArg, L".mid")) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
			//rcpcv.dllが有効ならサポート対象ファイルであるか追加確認する
			else if (rcpConv.IsAvailable() && rcpConv.IsSupportFileExt(pArg)) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
		}
		//起動後に再生開始
		if (wcscmp(pArg, L"-p") == 0) {
			m_CmdSwitchStatus[CMDSW_PLAY] = CMDSW_ON;
		}
		//再生終了時にアプリ終了
		if (wcscmp(pArg, L"-q") == 0) {
			m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_ON;
		}
		//デバッグモード
		if (wcscmp(pArg, L"-d") == 0) {
			m_CmdSwitchStatus[CMDSW_DEBUG] = CMDSW_ON;
		}
	}

	//ファイルパスが未指定の場合
	if (m_CmdSwitchStatus[CMDSW_FILE_PATH] != CMDSW_ON) {
		//再生／終了フラグは共に無効
		m_CmdSwitchStatus[CMDSW_PLAY] = CMDSW_NONE;
		m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_NONE;
	}

	//再生フラグONでなければ終了フラグは無効
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
// スイッチ状態取得
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
// ファイルパス取得
//******************************************************************************
const WCHAR* MTCmdLineParser::GetFilePath()
{
	return m_pFilePath;
}


