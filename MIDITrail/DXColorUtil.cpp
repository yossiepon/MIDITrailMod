//******************************************************************************
//
// MIDITrail / DXColorUtil
//
// カラーユーティリティクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXColorUtil.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
DXColorUtil::DXColorUtil(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DXColorUtil::~DXColorUtil(void)
{
}

//******************************************************************************
// RGBA（16進数文字列）からの数値変換
//******************************************************************************
D3DXCOLOR DXColorUtil::MakeColorFromHexRGBA(
		const TCHAR* pHexRGBA
	)
{
	float cr = 0.0f;
	float cg = 0.0f;
	float cb = 0.0f;
	float alpha = 0.0f;
	TCHAR* stopped = NULL;
	TCHAR buf[3];

	if (pHexRGBA == NULL) goto EXIT;
	if (_tcslen(pHexRGBA) < 4) goto EXIT;

	buf[2] = _T('\0');

	buf[0] = pHexRGBA[0];
	buf[1] = pHexRGBA[1];
	cr     = _tcstol(buf, &stopped, 16) / 255.0f;

	buf[0] = pHexRGBA[2];
	buf[1] = pHexRGBA[3];
	cg     = _tcstol(buf, &stopped, 16) / 255.0f;

	buf[0] = pHexRGBA[4];
	buf[1] = pHexRGBA[5];
	cb     = _tcstol(buf, &stopped, 16) / 255.0f;

	buf[0] = pHexRGBA[6];
	buf[1] = pHexRGBA[7];
	alpha  = _tcstol(buf, &stopped, 16) / 255.0f;

EXIT:;
	return D3DXCOLOR(cr, cg, cb, alpha);
}

//******************************************************************************
// RGB（16進数文字列）からの数値変換
//******************************************************************************
D3DCOLOR DXColorUtil::MakeColorFromHexRGB(
		const TCHAR* pHexRGB
	)
{
	long cr = 0;
	long cg = 0;
	long cb = 0;
	TCHAR* stopped = NULL;
	TCHAR buf[3];

	if (pHexRGB == NULL) goto EXIT;
	if (_tcslen(pHexRGB) < 6) goto EXIT;

	buf[2] = _T('\0');

	buf[0] = pHexRGB[0];
	buf[1] = pHexRGB[1];
	cr     = _tcstol(buf, &stopped, 16);

	buf[0] = pHexRGB[2];
	buf[1] = pHexRGB[3];
	cg     = _tcstol(buf, &stopped, 16);

	buf[0] = pHexRGB[4];
	buf[1] = pHexRGB[5];
	cb     = _tcstol(buf, &stopped, 16);

EXIT:;
	return D3DCOLOR_XRGB(cr, cg, cb);
}

