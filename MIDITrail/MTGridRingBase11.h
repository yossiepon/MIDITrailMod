//******************************************************************************
//
// MIDITrail / MTGridRingBase11
//
// Grid ring base class.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTSceneComponent11.h"
#include "MTNoteDesignRing11.h"

#define GRID_RING_SEGMENTS  (128)


//******************************************************************************
// Grid ring base class
//******************************************************************************
class MTGridRingBase11 : public MTSceneComponent11
{
public:

	MTGridRingBase11();
	virtual ~MTGridRingBase11();

	virtual void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(ID3D11DeviceContext* pContext,
	         const DirectX::SimpleMath::Matrix& viewProj,
	         const DirectX::SimpleMath::Vector4& lightDir);

protected:

	DXPrimitive11 m_Primitive;
	MTNoteDesignRing11 m_NoteDesign;
	bool m_isVisible;

	void _SetupPrimitive();
	void _CreateVertexOfRing(
			DXPRIMITIVE11_VERTEX* pVertex,
			unsigned long* pVertexIndex,
			unsigned long* pIndex,
			DirectX::SimpleMath::Vector3 basePos,
			unsigned long color);
};
