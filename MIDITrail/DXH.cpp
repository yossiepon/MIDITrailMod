//******************************************************************************
//
// MIDITrail / DXH
//
// ヘルパ関数クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXH.h"


//******************************************************************************
// 座標回転：YZ平面
//******************************************************************************
D3DXVECTOR3 DXH::RotateYZ(
		float centerY,
		float centerZ,
		D3DXVECTOR3 p1,
		float angle
	)
{
	D3DXVECTOR3 p2;
	float rad = 0.0f;

	rad = D3DXToRadian(angle);
	p2.x = p1.x;
	p2.y = (float)(centerY + (sin(rad) * (p1.z - centerZ)) + (cos(rad) * (p1.y - centerY)));
	p2.z = (float)(centerZ + (cos(rad) * (p1.z - centerZ)) - (sin(rad) * (p1.y - centerY)));

	return p2;
}


