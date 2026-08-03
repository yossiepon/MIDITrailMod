//******************************************************************************
//
// MIDITrail / MTConfigManager11
//
// 設定ファイル(conf/*.ini)を GUI(ImGui) で編集する Mod Mod 独自機能。
//
// Copyright (C) 2026 Ced (MIDITrail Mod Mod). All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfigManager11.h"
#include "imgui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ／デストラクタ
//******************************************************************************
MTConfigManager11::MTConfigManager11()
{
	m_Visible = false;
	m_ApplyRequested = false;
	m_ConfDir[0] = '\0';
	m_SelectedFile = -1;
	m_HasBOM = true;
}

MTConfigManager11::~MTConfigManager11()
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTConfigManager11::Initialize()
{
	int result = 0;
	TCHAR dir[MAX_PATH] = {_T('\0')};

	//conf ディレクトリパス：実行ファイルディレクトリ + conf サブフォルダ
	result = YNPathUtil::GetModuleDirPath(dir, MAX_PATH);
	if (result != 0) goto EXIT;
	_tcscat_s(dir, MAX_PATH, MT_CONFFILE_DIR);
	strncpy_s(m_ConfDir, MAX_PATH, dir, _TRUNCATE);

	_RefreshFileList();

EXIT:;
	return result;
}

//******************************************************************************
// 表示切替
//******************************************************************************
void MTConfigManager11::Toggle()
{
	m_Visible = !m_Visible;
	if (m_Visible) {
		_RefreshFileList();
		_ReloadCurrent();
	}
}

//******************************************************************************
// 再構築要求の取得（1回限り）
//******************************************************************************
bool MTConfigManager11::ConsumeApplyRequest()
{
	bool r = m_ApplyRequested;
	m_ApplyRequested = false;
	return r;
}

//******************************************************************************
// conf 内 .ini ファイル一覧の作成
//******************************************************************************
void MTConfigManager11::_RefreshFileList()
{
	WIN32_FIND_DATAA findData;
	HANDLE hFind = NULL;
	char findPath[MAX_PATH] = {'\0'};

	m_FileList.clear();

	strncpy_s(findPath, MAX_PATH, m_ConfDir, _TRUNCATE);
	strncat_s(findPath, MAX_PATH, "*.ini", _TRUNCATE);

	hFind = FindFirstFileA(findPath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) return;
	//編集対象は以下の PianoRoll 系シーン設定のみ（Player.ini / Video.ini 等は除外）
	static const char* const ALLOWED[] = {
		"PianoRoll2D.ini",     "PianoRoll2DLive.ini",
		"PianoRoll3D.ini",     "PianoRoll3DLive.ini",
		"PianoRollRain.ini",   "PianoRollRain2D.ini",
		"PianoRollRain2DLive.ini", "PianoRollRainLive.ini",
		"PianoRollRing.ini",   "PianoRollRingLive.ini",
	};
	const int ALLOWED_NUM = (int)(sizeof(ALLOWED) / sizeof(ALLOWED[0]));

	do {
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			for (int a = 0; a < ALLOWED_NUM; a++) {
				if (_stricmp(findData.cFileName, ALLOWED[a]) == 0) {
					m_FileList.push_back(findData.cFileName);
					break;
				}
			}
		}
	} while (FindNextFileA(hFind, &findData));
	FindClose(hFind);

	if (m_FileList.empty()) {
		m_SelectedFile = -1;
	}
	else if ((m_SelectedFile < 0) || (m_SelectedFile >= (int)m_FileList.size())) {
		m_SelectedFile = 0;
	}
}

//******************************************************************************
// 選択中ファイルの再読み込み
//******************************************************************************
void MTConfigManager11::_ReloadCurrent()
{
	m_Lines.clear();
	if ((m_SelectedFile < 0) || (m_SelectedFile >= (int)m_FileList.size())) return;
	_LoadFile(m_FileList[m_SelectedFile].c_str());
}

//******************************************************************************
// ファイル読み込み（行単位に解析。コメント・順序を保持）
//******************************************************************************
int MTConfigManager11::_LoadFile(const char* pFileName)
{
	int result = 0;
	char path[MAX_PATH] = {'\0'};
	FILE* fp = NULL;
	errno_t e = 0;
	std::string buf;

	m_Lines.clear();
	m_HasBOM = false;

	strncpy_s(path, MAX_PATH, m_ConfDir, _TRUNCATE);
	strncat_s(path, MAX_PATH, pFileName, _TRUNCATE);

	e = fopen_s(&fp, path, "rb");
	if ((e != 0) || (fp == NULL)) {
		m_Status = "Open failed.";
		result = -1;
		goto EXIT;
	}
	{
		char chunk[4096];
		size_t n = 0;
		while ((n = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
			buf.append(chunk, n);
		}
	}
	fclose(fp);

	//UTF-8 BOM 判定（保存時に復元するため）
	if ((buf.size() >= 3) &&
		((unsigned char)buf[0] == 0xEF) && ((unsigned char)buf[1] == 0xBB) && ((unsigned char)buf[2] == 0xBF)) {
		m_HasBOM = true;
		buf.erase(0, 3);
	}

	//行に分割（\r\n / \n）
	{
		size_t start = 0;
		while (start <= buf.size()) {
			size_t nl = buf.find('\n', start);
			std::string raw;
			if (nl == std::string::npos) {
				if (start >= buf.size()) break;   //末尾改行後は空行を足さない
				raw = buf.substr(start);
				start = buf.size() + 1;
			}
			else {
				raw = buf.substr(start, nl - start);
				start = nl + 1;
			}
			//末尾 \r 除去
			if (!raw.empty() && raw[raw.size() - 1] == '\r') raw.erase(raw.size() - 1);

			Line line;
			line.type = LT_BLANK;
			line.value[0] = '\0';

			//先頭の非空白文字を調べる
			size_t p = raw.find_first_not_of(" \t");
			if (p == std::string::npos) {
				line.type = LT_BLANK;
				line.text = "";
			}
			else if (raw[p] == ';' || raw[p] == '#') {
				line.type = LT_COMMENT;
				line.text = raw;
			}
			else if (raw[p] == '[') {
				size_t rb = raw.find(']', p);
				if (rb != std::string::npos) {
					line.type = LT_SECTION;
					line.text = raw.substr(p + 1, rb - p - 1);   //セクション名
				}
				else {
					line.type = LT_COMMENT;   //不正な行はコメント扱いで保持
					line.text = raw;
				}
			}
			else {
				size_t eq = raw.find('=');
				if (eq != std::string::npos) {
					line.type = LT_KEYVAL;
					line.key = raw.substr(0, eq);
					std::string v = raw.substr(eq + 1);
					strncpy_s(line.value, sizeof(line.value), v.c_str(), _TRUNCATE);
				}
				else {
					line.type = LT_COMMENT;   //'=' の無い行はそのまま保持
					line.text = raw;
				}
			}
			m_Lines.push_back(line);
		}
	}

	m_Status = "Loaded.";

EXIT:;
	return result;
}

//******************************************************************************
// ファイル保存（行構成を再構成して書き戻す）
//******************************************************************************
int MTConfigManager11::_SaveFile()
{
	int result = 0;
	char path[MAX_PATH] = {'\0'};
	FILE* fp = NULL;
	errno_t e = 0;
	std::string out;
	size_t i = 0;

	if ((m_SelectedFile < 0) || (m_SelectedFile >= (int)m_FileList.size())) return -1;

	strncpy_s(path, MAX_PATH, m_ConfDir, _TRUNCATE);
	strncat_s(path, MAX_PATH, m_FileList[m_SelectedFile].c_str(), _TRUNCATE);

	if (m_HasBOM) {
		out.append("\xEF\xBB\xBF");
	}
	for (i = 0; i < m_Lines.size(); i++) {
		const Line& line = m_Lines[i];
		switch (line.type) {
			case LT_SECTION:
				out.append("[");
				out.append(line.text);
				out.append("]\r\n");
				break;
			case LT_KEYVAL:
				out.append(line.key);
				out.append("=");
				out.append(line.value);
				out.append("\r\n");
				break;
			case LT_COMMENT:
			case LT_BLANK:
			default:
				out.append(line.text);
				out.append("\r\n");
				break;
		}
	}

	e = fopen_s(&fp, path, "wb");
	if ((e != 0) || (fp == NULL)) {
		m_Status = "Save failed.";
		result = -1;
		goto EXIT;
	}
	fwrite(out.data(), 1, out.size(), fp);
	fclose(fp);

	m_Status = "Saved. (applied to the current scene)";
	m_ApplyRequested = true;   //現シーンを再構築して反映する

EXIT:;
	return result;
}

//******************************************************************************
// 選択肢（enum）テーブル：固定値のキーはコンボで選択させる
//******************************************************************************
struct MTCfgEnumOpt { const char* value; const char* label; };
struct MTCfgEnumKey { const char* key; const MTCfgEnumOpt* opts; int num; };

static const MTCfgEnumOpt ENUM_NOTECOLORTYPE[] = {
	{ "CHANNEL",      "CHANNEL (per MIDI channel)" },
	{ "SCALE",        "SCALE (per pitch class)" },
	{ "CHANNELTRACK", "CHANNELTRACK (channel x track)" },
};
static const MTCfgEnumOpt ENUM_ACTIVEKEYCOLORTYPE[] = {
	{ "STANDARD", "STANDARD (fixed pressed-key color)" },
	{ "NOTE",     "NOTE (use the played note's color)" },
};
//D3dBlendType: ZERO=1, ONE=2, SRCALPHA=5, INVSRCALPHA=6, DESTALPHA=7, INVDESTALPHA=8
static const MTCfgEnumOpt ENUM_BLEND[] = {
	{ "1", "ZERO (1)" },        { "2", "ONE (2)" },
	{ "5", "SRCALPHA (5)" },    { "6", "INVSRCALPHA (6)" },
	{ "7", "DESTALPHA (7)" },   { "8", "INVDESTALPHA (8)" },
};

static const MTCfgEnumKey ENUM_KEYS[] = {
	{ "NoteColorType",      ENUM_NOTECOLORTYPE,      (int)(sizeof(ENUM_NOTECOLORTYPE)/sizeof(MTCfgEnumOpt)) },
	{ "ActiveKeyColorType", ENUM_ACTIVEKEYCOLORTYPE, (int)(sizeof(ENUM_ACTIVEKEYCOLORTYPE)/sizeof(MTCfgEnumOpt)) },
	{ "SrcBlend",           ENUM_BLEND,              (int)(sizeof(ENUM_BLEND)/sizeof(MTCfgEnumOpt)) },
	{ "DestBlend",          ENUM_BLEND,              (int)(sizeof(ENUM_BLEND)/sizeof(MTCfgEnumOpt)) },
};

static const MTCfgEnumKey* _FindEnum(const char* key)
{
	int n = (int)(sizeof(ENUM_KEYS) / sizeof(MTCfgEnumKey));
	for (int i = 0; i < n; i++) {
		if (_stricmp(key, ENUM_KEYS[i].key) == 0) return &ENUM_KEYS[i];
	}
	return NULL;
}

//******************************************************************************
// 色値ヘルパ
//  色とみなす条件 = キー名が color / rgb / rgba で「終わる」 かつ 値が 6桁(RGB) または
//  8桁(RGBA) の16進。
//  ・キー名を「含む」でなく「末尾一致」にしているのは、ActiveKeyColorDuration /
//    ActiveKeyColorTailRate / NoteColorType のように Color を含むが色でないキーを
//    確実に除外するため。
//  ・値の桁で誤判定（NumberOfStars=10000000 等）しないよう、キー名でも絞り込む。
//  ・6桁(RGB,アルファ無し)の例: BackGroundRGB=000000 ／ 8桁(RGBA)の例: NoteRGBA=EF7272EE
//******************************************************************************
static bool _EndsWithCI(const char* s, const char* suffix)
{
	if ((s == NULL) || (suffix == NULL)) return false;
	size_t ls = strlen(s), lf = strlen(suffix);
	if (lf > ls) return false;
	return (_stricmp(s + (ls - lf), suffix) == 0);
}

static bool _KeyLooksLikeColor(const char* key)
{
	//"rgba" は "rgb" で終わる判定でも拾えるが、明示しておく
	return _EndsWithCI(key, "color") || _EndsWithCI(key, "rgb") || _EndsWithCI(key, "rgba");
}

//色値なら hex の桁数(6 or 8)を返す。色でなければ 0。
static int _ColorHexDigits(const char* key, const char* v)
{
	int n = 0;
	if (v == NULL) return 0;
	if (!_KeyLooksLikeColor(key)) return 0;     //キー名が色っぽくなければ色扱いしない
	for (n = 0; v[n] != '\0'; n++) {
		if (n >= 8) return 0;                   //8桁超は色扱いしない
		if (!isxdigit((unsigned char)v[n])) return 0;
	}
	return ((n == 6) || (n == 8)) ? n : 0;      //6桁(RGB) か 8桁(RGBA) のみ
}

//hex を col[4] へ。6桁(RGB)はアルファを 1.0 とする。
static void _HexToColor(const char* hex, float col[4])
{
	char b[3] = { 0, 0, 0 };
	int comps = (int)(strlen(hex) / 2);         //3(RGB) or 4(RGBA)
	int i = 0;
	col[3] = 1.0f;                              //RGB のときの既定アルファ
	for (i = 0; i < comps; i++) {
		b[0] = hex[i * 2];
		b[1] = hex[i * 2 + 1];
		col[i] = (float)strtol(b, NULL, 16) / 255.0f;
	}
}

//col[4] を hex へ。digits(6 or 8) に合わせて RGB / RGBA で書き戻す（元の形式を保持）。
static void _ColorToHex(const float col[4], int digits, char* out, size_t outSize)
{
	int r = (int)(col[0] * 255.0f + 0.5f);
	int g = (int)(col[1] * 255.0f + 0.5f);
	int b = (int)(col[2] * 255.0f + 0.5f);
	int a = (int)(col[3] * 255.0f + 0.5f);
	if (r < 0) r = 0; if (r > 255) r = 255;
	if (g < 0) g = 0; if (g > 255) g = 255;
	if (b < 0) b = 0; if (b > 255) b = 255;
	if (a < 0) a = 0; if (a > 255) a = 255;
	if (digits == 6) sprintf_s(out, outSize, "%02X%02X%02X", r, g, b);
	else             sprintf_s(out, outSize, "%02X%02X%02X%02X", r, g, b, a);
}

//******************************************************************************
// ImGui ウィンドウ描画
//******************************************************************************
void MTConfigManager11::RenderImGui()
{
	if (!m_Visible) return;

	ImGui::SetNextWindowSize(ImVec2(560, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Config Manager", &m_Visible)) {
		ImGui::End();
		return;
	}

	//ファイル選択コンボ
	const char* curName = ((m_SelectedFile >= 0) && (m_SelectedFile < (int)m_FileList.size()))
							? m_FileList[m_SelectedFile].c_str() : "(no file)";
	ImGui::SetNextItemWidth(280);
	if (ImGui::BeginCombo("File", curName)) {
		for (int n = 0; n < (int)m_FileList.size(); n++) {
			bool sel = (n == m_SelectedFile);
			if (ImGui::Selectable(m_FileList[n].c_str(), sel)) {
				if (n != m_SelectedFile) {
					m_SelectedFile = n;
					_ReloadCurrent();
				}
			}
			if (sel) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload")) {
		_ReloadCurrent();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		_SaveFile();
	}
	if (!m_Status.empty()) {
		ImGui::SameLine();
		ImGui::TextDisabled("%s", m_Status.c_str());
	}

	ImGui::Separator();

	//本体（スクロール領域）
	ImGui::BeginChild("body", ImVec2(0, 0), true);
	bool sectionOpen = true;   //最初のセクション前の行は表示
	for (size_t i = 0; i < m_Lines.size(); i++) {
		Line& line = m_Lines[i];
		ImGui::PushID((int)i);
		switch (line.type) {
			case LT_SECTION:
				sectionOpen = ImGui::CollapsingHeader(line.text.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
				break;
			case LT_KEYVAL:
				if (sectionOpen) {
					const MTCfgEnumKey* en = _FindEnum(line.key.c_str());
					if (en != NULL) {
						//選択肢（enum）はコンボで選択（現在値が候補外でもプレビュー表示）
						ImGui::SetNextItemWidth(220);
						if (ImGui::BeginCombo(line.key.c_str(), line.value)) {
							for (int o = 0; o < en->num; o++) {
								bool sel = (_stricmp(line.value, en->opts[o].value) == 0);
								if (ImGui::Selectable(en->opts[o].label, sel)) {
									strncpy_s(line.value, sizeof(line.value), en->opts[o].value, _TRUNCATE);
								}
								if (sel) ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
					}
					else if (int colorDigits = _ColorHexDigits(line.key.c_str(), line.value)) {
						//色はカラーピッカーで編集。8桁(RGBA)はアルファ付き、6桁(RGB)はアルファ無し。
						float col[4];
						_HexToColor(line.value, col);
						ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_Uint8;
						if (colorDigits == 8) flags |= ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf;
						else                  flags |= ImGuiColorEditFlags_NoAlpha;
						ImGui::SetNextItemWidth(220);
						if (ImGui::ColorEdit4(line.key.c_str(), col, flags)) {
							_ColorToHex(col, colorDigits, line.value, sizeof(line.value));
						}
					}
					else {
						ImGui::SetNextItemWidth(220);
						ImGui::InputText(line.key.c_str(), line.value, sizeof(line.value));
					}
				}
				break;
			case LT_COMMENT:
				if (sectionOpen) {
					ImGui::TextDisabled("%s", line.text.c_str());
				}
				break;
			case LT_BLANK:
			default:
				if (sectionOpen) ImGui::Spacing();
				break;
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::End();
}
