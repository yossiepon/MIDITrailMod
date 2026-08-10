//******************************************************************************
//
// MIDITrail / MTGridRing11Base
//
// Grid ring base class (DX11).
// Common: DXPrimitive11 management, Update, Draw, Release, ring vertex helper.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
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
class MTGridRing11Base : public MTSceneComponent11
{
public:

	MTGridRing11Base();
	virtual ~MTGridRing11Base();

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
