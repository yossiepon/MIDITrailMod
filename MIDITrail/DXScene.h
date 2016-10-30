//******************************************************************************
//
// MIDITrail / DXScene
//
// シーン基底クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
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

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

};

