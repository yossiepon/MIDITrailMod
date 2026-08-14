//******************************************************************************
//
// MIDITrail / MTColorPalette
//
// Color palette class.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "SMIDILib.h"
#include <directxtk/SimpleMath.h>


//******************************************************************************
// Color palette class
//******************************************************************************
class MTColorPalette
{
public:

	MTColorPalette();
	virtual ~MTColorPalette();

	int Initialize();

	int  GetChColor(unsigned int chNo, DirectX::SimpleMath::Color* pColor);
	int  SetChColor(unsigned int chNo, DirectX::SimpleMath::Color color);

	void GetBackgroundColor(DirectX::SimpleMath::Color* pColor);
	void SetBackgroundColor(DirectX::SimpleMath::Color color);

	void GetGridLineColor(DirectX::SimpleMath::Color* pColor);
	void SetGridLineColor(DirectX::SimpleMath::Color color);

	void GetCounterColor(DirectX::SimpleMath::Color* pColor);
	void SetCounterColor(DirectX::SimpleMath::Color color);

	int CopyFrom(MTColorPalette* pColorSrc);

private:

	void operator=(const MTColorPalette&);
	MTColorPalette(const MTColorPalette&);

	DirectX::SimpleMath::Color m_ChColor[SM_MAX_CH_NUM];
	DirectX::SimpleMath::Color m_BgColor;
	DirectX::SimpleMath::Color m_GridLineColor;
	DirectX::SimpleMath::Color m_CounterColor;

	void _Clear();
};
