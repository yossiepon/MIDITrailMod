//******************************************************************************
//
// MIDITrail / MTGridBoxBase11
//
// Grid box base class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 yossiepon Oniichan. All Rights Reserved.
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
class MTGridBoxBase11 : public MTSceneComponent11
{
public:

	MTGridBoxBase11();
	virtual ~MTGridBoxBase11();

	virtual void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(ID3D11DeviceContext* pContext,
	         const DirectX::SimpleMath::Matrix& viewProj,
	         const DirectX::SimpleMath::Vector4& lightDir);

	bool IsReady() const { return m_isReady; }

protected:

	DXPrimitive11 m_Primitive;
	MTNoteDesign11 m_NoteDesign;
	bool m_isReady;
	bool m_isVisible;

	void _SetupPrimitive();
};
