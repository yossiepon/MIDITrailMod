//******************************************************************************
//
// MIDITrail / MTColorPalette
//
// Color palette class.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTColorPalette.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTColorPalette::MTColorPalette()
{
	_Clear();
}

MTColorPalette::~MTColorPalette()
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTColorPalette::Initialize()
{
	_Clear();
	return 0;
}

//******************************************************************************
// Clear
//******************************************************************************
void MTColorPalette::_Clear()
{
	for (unsigned int chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		m_ChColor[chNo] = Color(1.0f, 1.0f, 1.0f, 1.0f);
	}
	m_BgColor       = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_GridLineColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_CounterColor  = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

//******************************************************************************
// Channel color
//******************************************************************************
int MTColorPalette::GetChColor(unsigned int chNo, Color* pColor)
{
	if (chNo >= SM_MAX_CH_NUM) {
		return YN_SET_ERR("Program error.", chNo, 0);
	}
	*pColor = m_ChColor[chNo];
	return 0;
}

int MTColorPalette::SetChColor(unsigned int chNo, Color color)
{
	if (chNo >= SM_MAX_CH_NUM) {
		return YN_SET_ERR("Program error.", chNo, 0);
	}
	m_ChColor[chNo] = color;
	return 0;
}

//******************************************************************************
// Background color
//******************************************************************************
void MTColorPalette::GetBackgroundColor(Color* pColor)
{
	*pColor = m_BgColor;
}

void MTColorPalette::SetBackgroundColor(Color color)
{
	m_BgColor = color;
}

//******************************************************************************
// Grid line color
//******************************************************************************
void MTColorPalette::GetGridLineColor(Color* pColor)
{
	*pColor = m_GridLineColor;
}

void MTColorPalette::SetGridLineColor(Color color)
{
	m_GridLineColor = color;
}

//******************************************************************************
// Counter color
//******************************************************************************
void MTColorPalette::GetCounterColor(Color* pColor)
{
	*pColor = m_CounterColor;
}

void MTColorPalette::SetCounterColor(Color color)
{
	m_CounterColor = color;
}

//******************************************************************************
// Copy
//******************************************************************************
int MTColorPalette::CopyFrom(MTColorPalette* pColorSrc)
{
	int result = 0;

	for (unsigned int chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		result = pColorSrc->GetChColor(chNo, &m_ChColor[chNo]);
		if (result != 0) goto EXIT;
	}

	pColorSrc->GetBackgroundColor(&m_BgColor);
	pColorSrc->GetGridLineColor(&m_GridLineColor);
	pColorSrc->GetCounterColor(&m_CounterColor);

EXIT:;
	return result;
}
