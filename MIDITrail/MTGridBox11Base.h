//******************************************************************************
//
// MIDITrail / MTGridBox11Base
//
// Grid box base class (DX11).
// Common: DXPrimitive11 management, Update (world matrix), Draw, Release.
// Derived classes provide vertex generation in their Create methods.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTSceneComponent11.h"
#include "MTNoteDesign11.h"
#include <directxtk/SimpleMath.h>


//******************************************************************************
// Grid box base class
//******************************************************************************
class MTGridBox11Base : public MTSceneComponent11
{
public:

	MTGridBox11Base();
	virtual ~MTGridBox11Base();

	virtual void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(ID3D11DeviceContext* pContext,
	         const DirectX::SimpleMath::Matrix& viewProj,
	         const DirectX::SimpleMath::Vector4& lightDir,
	         float rollAngle = 0.0f);

	bool IsReady() const { return m_isReady; }

protected:

	DXPrimitive11 m_Primitive;
	MTNoteDesign11 m_NoteDesign;
	bool m_isReady;

	void _SetupPrimitive();
};
