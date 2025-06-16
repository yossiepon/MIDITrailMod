//******************************************************************************
//
// MIDITrail / MTColorConf
//
// カラー設定クラス
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "DXColorUtil.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTColorConf::MTColorConf(void)
{
	unsigned long i = 0;

	m_SelectedColorPaletteNo = 0;
	
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		m_pColorPalette[i] = NULL;
	}

	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTColorConf::~MTColorConf(void)
{
	unsigned long i = 0;
	
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		if (m_pColorPalette[i] != NULL) {
			delete m_pColorPalette[i];
		}
		m_pColorPalette[i] = NULL;
	}
	
	return;
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTColorConf::Initialize(const TCHAR* pDefaultSceneName)
{
	int result = 0;
	int i = 0;
		
	//色パレット生成と初期化
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		try {
			m_pColorPalette[i] = new MTColorPalette();
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", i, 0);
			goto EXIT;
		}
		result = m_pColorPalette[i]->Initialize();
		if (result != 0) goto EXIT;
	}
	
	//設定ファイル初期化
	result = _InitConfFile();
	if (result != 0) goto EXIT;
	
	//ユーザ設定読み込み
	result = _LoadColorConf(pDefaultSceneName);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 選択カラーパレット番号取得：0 デフォルト、1-6 パレット番号
//******************************************************************************
unsigned long MTColorConf::GetSelectedColorPaletteNo()
{
	return m_SelectedColorPaletteNo;
}

//******************************************************************************
// 選択カラーパレット番号登録：0 デフォルト、1-6 パレット番号
//******************************************************************************
int MTColorConf::SetSelectedColorPaletteNo(unsigned long paletteNo)
{
	int result = 0;
	
	if (paletteNo >= MT_COLOR_PALETTE_NUM_MAX) {
		result = YN_SET_ERR("Program error.", paletteNo, 0);
		goto EXIT;
	}
	
	m_SelectedColorPaletteNo = paletteNo;
	
EXIT:;
	return result;
}

//******************************************************************************
// カラーパレット取得：0 デフォルト、1-6 パレット番号
//******************************************************************************
int MTColorConf::GetColorPalette(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	
	if (paletteNo >= MT_COLOR_PALETTE_NUM_MAX) {
		result = YN_SET_ERR("Program error.", paletteNo, 0);
		goto EXIT;
	}
	
	pColorPalette->CopyFrom(m_pColorPalette[paletteNo]);
	
EXIT:;
	return result;
}

//******************************************************************************
// 選択カラーパレット取得
//******************************************************************************
void MTColorConf::GetSelectedColorPalette(MTColorPalette* pColorPalette)
{
	pColorPalette->CopyFrom(m_pColorPalette[m_SelectedColorPaletteNo]);
}

//******************************************************************************
// カラーパレット登録：1-6 パレット番号、0 デフォルトは登録不可
//******************************************************************************
int MTColorConf::SetColorPalette(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	
	//デフォルト0のパレットは書き換え不可
	if ((paletteNo == 0) || (paletteNo >= MT_COLOR_PALETTE_NUM_MAX)) {
		result = YN_SET_ERR("Program error.", paletteNo, 0);
		goto EXIT;
	}
	
	m_pColorPalette[paletteNo]->CopyFrom(pColorPalette);
	
EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル初期化
//******************************************************************************
int MTColorConf::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_COLOR);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ユーザ設定読み込み
//******************************************************************************
int MTColorConf::_LoadColorConf(const TCHAR* pDefaultSceneName)
{
	int result = 0;
	unsigned long paletteNo = 0;
	
	//セクション設定
	result = m_ConfFile.SetCurSection(_T("ColorSelect"));
	if (result != 0) goto EXIT;
	
	//ユーザ設定値取得：選択カラーパレット番号
	result = m_ConfFile.GetInt(_T("SelectedColorPaletteNo"), &m_SelectedColorPaletteNo, 0);
	if (result != 0) goto EXIT;
	if ((m_SelectedColorPaletteNo < 0) || (m_SelectedColorPaletteNo >= MT_COLOR_PALETTE_NUM_MAX)) {
		m_SelectedColorPaletteNo = 0;
	}
	
	//デフォルトカラーパレット読み込み
	result = _LoadColorPaletteDefault(pDefaultSceneName, m_pColorPalette[0]);
	if (result != 0) goto EXIT;

	//カラーパレット設定読み込み
	for (paletteNo = 1; paletteNo < MT_COLOR_PALETTE_NUM_MAX; paletteNo++) {
		result = _LoadColorPalettes(paletteNo, m_pColorPalette[paletteNo]);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// デフォルトカラーパレット読み込み
//******************************************************************************
int MTColorConf::_LoadColorPaletteDefault(
		const TCHAR* pDefaultSceneName,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	MTConfFile confFile;
	
	//設定ファイル読み込み
	result = confFile.Initialize(pDefaultSceneName);
	if (result != 0) goto EXIT;
	
	//セクション指定
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;
	
	//チャンネル色取得
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02d-NoteRGBA"), chNo+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;
		pColorPalette->SetChColor(chNo, DXColorUtil::MakeColorFromHexRGBA(hexColor));
	}
	//背景色取得
	result = confFile.GetStr(_T("BackGroundRGB"), hexColor, 16, _T("000000FF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetBackgroundColor(DXColorUtil::MakeColorFromHexRGB(hexColor));
	
	//グリッドライン色取得
	result = confFile.GetStr(_T("GridLineRGBA"), hexColor, 16, "444444FF");
	if (result != 0) goto EXIT;
	pColorPalette->SetGridLineColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//カウンター色取得
	result = confFile.GetStr(_T("CaptionRGBA"), hexColor, 16, "FFFFFFFF");
	if (result != 0) goto EXIT;
	pColorPalette->SetCounterColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));

EXIT:;
	return result;
}

//******************************************************************************
// カラーパレット読み込み
//******************************************************************************
int MTColorConf::_LoadColorPalettes(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR section[32] = {_T('\0')};
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	
	//セクション設定
	_stprintf_s(section, 32, _T("ColorPalette-%u"), paletteNo);
	result = m_ConfFile.SetCurSection(section);
	if (result != 0) goto EXIT;
	
	//チャンネル色取得
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo+1);
		result = m_ConfFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;
		pColorPalette->SetChColor(chNo, DXColorUtil::MakeColorFromHexRGBA(hexColor));
	}
	
	//背景色取得
	result = m_ConfFile.GetStr(_T("BackGroundRGBA"), hexColor, 16, _T("000000FF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetBackgroundColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//グリッドライン色取得
	result = m_ConfFile.GetStr(_T("GridLineRGBA"), hexColor, 16, _T("444444FF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetGridLineColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//カウンター色取得
	result = m_ConfFile.GetStr(_T("CaptionRGBA"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetCounterColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
EXIT:;
	return result;
}

//******************************************************************************
// 設定保存
//******************************************************************************
int MTColorConf::Save()
{
	int result = 0;
	unsigned long paletteNo = 0;
	
	//選択カラーパレット番号保存
	result = m_ConfFile.SetCurSection(_T("ColorSelect"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("SelectedColorPaletteNo"), m_SelectedColorPaletteNo);
	if (result != 0) goto EXIT;
	
	//カラーパレット 1-6 保存
	for (paletteNo = 1; paletteNo < MT_COLOR_PALETTE_NUM_MAX; paletteNo++) {
		result = _SaveColorPalette(paletteNo, m_pColorPalette[paletteNo]);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// カラーパレット保存
//******************************************************************************
int MTColorConf::_SaveColorPalette(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR section[32] = {_T('\0')};
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	D3DXCOLOR color;
	
	//セクション設定
	_stprintf_s(section, 32, _T("ColorPalette-%u"), paletteNo);
	result = m_ConfFile.SetCurSection(section);
	if (result != 0) goto EXIT;
	
	//チャンネル色登録
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo+1);
		result = pColorPalette->GetChColor(chNo, &color);
		if (result != 0) goto EXIT;
		DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
		result = m_ConfFile.SetStr(key, hexColor);
		if (result != 0) goto EXIT;
	}
	
	//背景色登録
	pColorPalette->GetBackgroundColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("BackGroundRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
	//グリッドライン色登録
	pColorPalette->GetGridLineColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("GridLineRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
	//カウンター色登録
	pColorPalette->GetCounterColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("CaptionRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}


