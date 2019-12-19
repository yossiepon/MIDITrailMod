//******************************************************************************
//
// MIDITrail / MTNoteRippleRing
//
// リング用ノート波紋描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRipple.h"

using namespace SMIDILib;


//******************************************************************************
// リング用ノート波紋描画クラス
//******************************************************************************
class MTNoteRippleRing : public MTNoteRipple
{
public:

	//コンストラクタ／デストラクタ
	MTNoteRippleRing(void);
	virtual ~MTNoteRippleRing(void);

private:

	virtual int _CreateNoteDesign();

};


