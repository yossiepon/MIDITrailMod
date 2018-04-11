//******************************************************************************
//
// MIDITrail / DXScene
//
// シーン基底クラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DXRendererに対応する抽象クラス。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// シーン基底クラス
//******************************************************************************
class DXScene
{
public:

	//コンストラクタ／デストラクタ
	DXScene(void);
	virtual ~DXScene(void);

	//背景色設定
	virtual void SetBGColor(D3DCOLOR color);

	//背景色取得
	virtual D3DCOLOR GetBGColor();

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//背景色
	D3DCOLOR m_BGColor;

};

