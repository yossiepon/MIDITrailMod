//******************************************************************************
//
// MIDITrail / MTColorConf
//
// Color configuration class.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTColorPalette.h"

using namespace YNBaseLib;


//******************************************************************************
// Parameter definitions
//******************************************************************************
//Maximum number of color palettes (including the default)
#define MT_COLOR_PALETTE_NUM_MAX	(7)


//******************************************************************************
// Color configuration class
//******************************************************************************
class MTColorConf
{
public:
	
	//Constructor / Destructor
	MTColorConf(void);
	virtual ~MTColorConf(void);
	
	//Initialize
	int Initialize(const TCHAR* pDefaultSceneName);
	
	//Get selected color palette number: 0 = default, 1-6 = palette number
	unsigned long GetSelectedColorPaletteNo();
	
	//Set selected color palette number: 0 = default, 1-6 = palette number
	int SetSelectedColorPaletteNo(unsigned long paletteNo);
	
	//Get color palette: 0 = default, 1-6 = palette number
	int GetColorPalette(unsigned long paletteNo, MTColorPalette* pColorPalette);
	
	//Get selected color palette
	void GetSelectedColorPalette(MTColorPalette* pColorPalette);
	
	//Set color palette: 1-6 = palette number; 0 (default) cannot be set
	int SetColorPalette(unsigned long paletteNo, MTColorPalette* pColorPalette);
	
	//Save the setting
	int Save();
	
private:
	
	//Prohibit assignment and copy constructor
	void operator=(const MTColorConf&);
	MTColorConf(const MTColorConf&);
	
	YNConfFile m_ConfFile;
	int m_SelectedColorPaletteNo;
	MTColorPalette* m_pColorPalette[MT_COLOR_PALETTE_NUM_MAX];
	
	int _InitConfFile();
	int _LoadColorConf(const TCHAR* pDefaultSceneName);
	int _LoadColorPaletteDefault(const TCHAR* pDefaultSceneName, MTColorPalette* pColorPalette);
	int _LoadColorPalettes(unsigned long paletteNo, MTColorPalette* pColorPalette);
	int _SaveColorPalette(unsigned long paletteNo, MTColorPalette* pColorPalette);
	
};


