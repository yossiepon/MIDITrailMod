//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// コマンドライン解析クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMRcpConv.h"
#include "MTCmdLineParser.h"
#include <tchar.h>
#include <stdlib.h>

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTCmdLineParser::MTCmdLineParser(void)
{
	m_pFilePath = _T("");
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
int MTCmdLineParser::Initialize(
		LPTSTR pCmdLine
	)
{
	int result = 0;

	//コマンドライン解析
	result = _AnalyzeCmdLine(pCmdLine);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// コマンドライン解析
//******************************************************************************
int MTCmdLineParser::_AnalyzeCmdLine(
		LPTSTR pCmdLine
	)
{
	int result = 0;
	int i = 0;
	TCHAR* pArg = NULL;
	SMRcpConv rcpConv;

	//CommandLineToArgvW は存在するが CommandLineToArgvA は存在しない
	//このためAPIでの解析はあきらめて __argc, __targv を利用する
	//残念ながらpCmdLineは参照しない

	//RCP読み込み可否確認のためRCPファイル変換オブジェクトを用意する
	result = rcpConv.Initialize();
	if (result != 0) goto EXIT;

	//引数の解析
	for (i = 1; i < __argc; i++) {
		pArg = __targv[i];

		//MessageBox(NULL, pArg, _T(""), MB_OK);

		//ファイルパス
		//  ファイルパスが複数指定された場合は先頭のみを採用する
		if ((_tcslen(m_pFilePath) == 0) && (_tcslen(pArg) > 4)) {
			if (YNPathUtil::IsFileExtMatch(pArg, ".mid")) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
			else if (rcpConv.IsAvailable() && rcpConv.IsSupportFileExt(pArg)) {
				m_pFilePath = pArg;
				m_CmdSwitchStatus[CMDSW_FILE_PATH] = CMDSW_ON;
			}
		}
		//起動後に再生開始
		if (_tcscmp(pArg, _T("-p")) == 0) {
			m_CmdSwitchStatus[CMDSW_PLAY] = CMDSW_ON;
		}
		//再生終了時にアプリ終了
		if (_tcscmp(pArg, _T("-q")) == 0) {
			m_CmdSwitchStatus[CMDSW_QUIET] = CMDSW_ON;
		}
		//デバッグモード
		if (_tcscmp(pArg, _T("-d")) == 0) {
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
const TCHAR* MTCmdLineParser::GetFilePath()
{
	return m_pFilePath;
}


