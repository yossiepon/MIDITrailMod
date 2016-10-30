//******************************************************************************
//
// MIDITrail / MTCmdLineParser
//
// コマンドライン解析クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

//******************************************************************************
// パラメータ定義
//******************************************************************************
//スイッチ状態
#define CMDSW_NONE		(0)	//未定義
#define CMDSW_ON		(1)	//ON

//スイッチ種別
#define CMDSW_FILE_PATH	(0)	//ファイルパス
#define CMDSW_PLAY		(1)	//再生
#define CMDSW_QUIET		(2)	//終了
#define CMDSW_DEBUG		(3)	//デバッグモード
#define CMDSW_MAX		(4)	//終端フラグ：必ず末尾に定義する


//******************************************************************************
// コマンドライン解析クラス
//******************************************************************************
class MTCmdLineParser
{
public:

	//コンストラクタ／デストラクタ
	MTCmdLineParser(void);
	virtual ~MTCmdLineParser(void);

	//初期化
	int Initialize(LPTSTR pCmdLine);

	//スイッチ状態取得
	int GetSwitch(unsigned long switchType);

	//ファイルパス取得
	const TCHAR* GetFilePath();

private:

	unsigned char m_CmdSwitchStatus[CMDSW_MAX];
	TCHAR* m_pFilePath;

	int _AnalyzeCmdLine(LPTSTR pCmdLine);

};


