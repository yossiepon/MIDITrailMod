//******************************************************************************
//
// MIDITrail / MTColorConf
//
// Color configuration class.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "DXColorUtil.h"

using namespace YNBaseLib;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
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
// Destructor
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
// Initialize
//******************************************************************************
int MTColorConf::Initialize(const TCHAR* pDefaultSceneName)
{
	int result = 0;
	int i = 0;
		
	//Create and initialize color palette
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
	
	//Initialize config file
	result = _InitConfFile();
	if (result != 0) goto EXIT;
	
	//Load user settings
	result = _LoadColorConf(pDefaultSceneName);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// Get selected color palette number: 0 = default, 1-6 = palette number
//******************************************************************************
unsigned long MTColorConf::GetSelectedColorPaletteNo()
{
	return m_SelectedColorPaletteNo;
}

//******************************************************************************
// Set selected color palette number: 0 = default, 1-6 = palette number
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
// Get color palette: 0 = default, 1-6 = palette number
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
// Get selected color palette
//******************************************************************************
void MTColorConf::GetSelectedColorPalette(MTColorPalette* pColorPalette)
{
	pColorPalette->CopyFrom(m_pColorPalette[m_SelectedColorPaletteNo]);
}

//******************************************************************************
// Set color palette: 1-6 = palette number; 0 (default) cannot be set
//******************************************************************************
int MTColorConf::SetColorPalette(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	
	//The default palette (0) cannot be overwritten
	if ((paletteNo == 0) || (paletteNo >= MT_COLOR_PALETTE_NUM_MAX)) {
		result = YN_SET_ERR("Program error.", paletteNo, 0);
		goto EXIT;
	}
	
	m_pColorPalette[paletteNo]->CopyFrom(pColorPalette);
	
EXIT:;
	return result;
}

//******************************************************************************
// Initialize config file
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
// Load user settings
//******************************************************************************
int MTColorConf::_LoadColorConf(const TCHAR* pDefaultSceneName)
{
	int result = 0;
	unsigned long paletteNo = 0;
	
	//Set section
	result = m_ConfFile.SetCurSection(_T("ColorSelect"));
	if (result != 0) goto EXIT;
	
	//Get user setting: selected color palette number
	result = m_ConfFile.GetInt(_T("SelectedColorPaletteNo"), &m_SelectedColorPaletteNo, 0);
	if (result != 0) goto EXIT;
	if ((m_SelectedColorPaletteNo < 0) || (m_SelectedColorPaletteNo >= MT_COLOR_PALETTE_NUM_MAX)) {
		m_SelectedColorPaletteNo = 0;
	}
	
	//Load default color palette
	result = _LoadColorPaletteDefault(pDefaultSceneName, m_pColorPalette[0]);
	if (result != 0) goto EXIT;

	//Load color palette settings
	for (paletteNo = 1; paletteNo < MT_COLOR_PALETTE_NUM_MAX; paletteNo++) {
		result = _LoadColorPalettes(paletteNo, m_pColorPalette[paletteNo]);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Load default color palette
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
	
	//Load config file
	result = confFile.Initialize(pDefaultSceneName);
	if (result != 0) goto EXIT;
	
	//Set section
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;
	
	//Get channel color
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02d-NoteRGBA"), chNo+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;
		pColorPalette->SetChColor(chNo, DXColorUtil::MakeColorFromHexRGBA(hexColor));
	}
	//Get background color (converts RGB 6-digit to RGBA 8-digit before loading)
	result = confFile.GetStr(_T("BackGroundRGB"), hexColor, 16, _T("000000"));
	if (result != 0) goto EXIT;
	{
		TCHAR hexRGBA[16] = {0};
		_stprintf_s(hexRGBA, 16, _T("%sFF"), hexColor);
		pColorPalette->SetBackgroundColor(DXColorUtil::MakeColorFromHexRGBA(hexRGBA));
	}
	
	//Get grid line color
	result = confFile.GetStr(_T("GridLineRGBA"), hexColor, 16, "444444FF");
	if (result != 0) goto EXIT;
	pColorPalette->SetGridLineColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//Get counter color
	result = confFile.GetStr(_T("CaptionRGBA"), hexColor, 16, "FFFFFFFF");
	if (result != 0) goto EXIT;
	pColorPalette->SetCounterColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));

EXIT:;
	return result;
}

//******************************************************************************
// Load color palettes
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
	
	//Set section
	_stprintf_s(section, 32, _T("ColorPalette-%u"), paletteNo);
	result = m_ConfFile.SetCurSection(section);
	if (result != 0) goto EXIT;

	//Get channel color
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo+1);
		result = m_ConfFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;
		pColorPalette->SetChColor(chNo, DXColorUtil::MakeColorFromHexRGBA(hexColor));
	}
	
	//Get background color
	result = m_ConfFile.GetStr(_T("BackGroundRGBA"), hexColor, 16, _T("000000FF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetBackgroundColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//Get grid line color
	result = m_ConfFile.GetStr(_T("GridLineRGBA"), hexColor, 16, _T("444444FF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetGridLineColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//Get counter color
	result = m_ConfFile.GetStr(_T("CaptionRGBA"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetCounterColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
EXIT:;
	return result;
}

//******************************************************************************
// Save settings
//******************************************************************************
int MTColorConf::Save()
{
	int result = 0;
	unsigned long paletteNo = 0;
	
	//Save selected color palette number
	result = m_ConfFile.SetCurSection(_T("ColorSelect"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("SelectedColorPaletteNo"), m_SelectedColorPaletteNo);
	if (result != 0) goto EXIT;
	
	//Save color palettes 1-6
	for (paletteNo = 1; paletteNo < MT_COLOR_PALETTE_NUM_MAX; paletteNo++) {
		result = _SaveColorPalette(paletteNo, m_pColorPalette[paletteNo]);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Save color palette
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
	Color color;
	
	//Set section
	_stprintf_s(section, 32, _T("ColorPalette-%u"), paletteNo);
	result = m_ConfFile.SetCurSection(section);
	if (result != 0) goto EXIT;

	//Register channel color
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo+1);
		result = pColorPalette->GetChColor(chNo, &color);
		if (result != 0) goto EXIT;
		DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
		result = m_ConfFile.SetStr(key, hexColor);
		if (result != 0) goto EXIT;
	}
	
	//Register background color
	pColorPalette->GetBackgroundColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("BackGroundRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
	//Register grid line color
	pColorPalette->GetGridLineColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("GridLineRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
	//Register counter color
	pColorPalette->GetCounterColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("CaptionRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}


