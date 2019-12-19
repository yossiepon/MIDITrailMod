//******************************************************************************
//
// MIDITrail / MTNoteRippleRingLive
//
// ライブモニタ用ノート波紋リング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRipple.h"


//******************************************************************************
// ライブモニタ用ノート波紋リング描画クラス
//******************************************************************************
class MTNoteRippleRingLive : public MTNoteRipple
{
public:

	//コンストラクタ／デストラクタ
	MTNoteRippleRingLive(void);
	virtual ~MTNoteRippleRingLive(void);

private:

	virtual int _CreateNoteDesign();

};


