//******************************************************************************
//
// MIDITrail / MTConfigManager11
//
// 設定ファイル(conf/*.ini)を GUI(ImGui) で編集する Mod Mod 独自機能。
//   ・conf フォルダの .ini を一覧から選択
//   ・[section] は折りたたみ、key=value は入力欄で編集、コメント(;)は保持表示
//   ・保存時は行構成を再構成して書き戻す（コメント・順序を保持）
//
// Copyright (C) 2026 Ced (MIDITrail Mod Mod). All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <windows.h>
#include <vector>
#include <string>


//******************************************************************************
// 設定マネージャ（ImGui）
//******************************************************************************
class MTConfigManager11
{
public:

	MTConfigManager11();
	virtual ~MTConfigManager11();

	//初期化（conf ディレクトリパス解決＋ファイル一覧作成）
	int Initialize();

	//ImGui ウィンドウ描画（ImGui フレーム内から呼ぶ）
	void RenderImGui();

	//表示状態
	bool IsVisible() const { return m_Visible; }
	void SetVisible(bool visible) { m_Visible = visible; }
	void Toggle();

	//保存後の「現シーン再構築要求」を1回だけ取得する
	bool ConsumeApplyRequest();

private:

	enum LineType { LT_SECTION, LT_KEYVAL, LT_COMMENT, LT_BLANK };

	struct Line {
		LineType    type;
		std::string text;        //section名 / コメント・空行の生テキスト
		std::string key;         //key=value の key
		char        value[256];  //key=value の value（編集バッファ：UTF-8）
	};

	typedef std::vector<std::string> StrList;

	bool                     m_Visible;
	bool                     m_ApplyRequested;
	char                     m_ConfDir[MAX_PATH];   // module dir + conf subdir
	StrList                  m_FileList;            // .ini file names in conf
	int                      m_SelectedFile;
	std::vector<Line>        m_Lines;               //選択中ファイルの行
	bool                     m_HasBOM;              //選択中ファイルが UTF-8 BOM か
	std::string              m_Status;              //ステータス表示

	void _RefreshFileList();
	void _ReloadCurrent();
	int  _LoadFile(const char* pFileName);
	int  _SaveFile();

	//代入とコピーコンストラクタの禁止
	void operator=(const MTConfigManager11&);
	MTConfigManager11(const MTConfigManager11&);
};
