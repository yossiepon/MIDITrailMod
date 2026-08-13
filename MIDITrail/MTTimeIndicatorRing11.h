//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRing11
//
// Time indicator ring renderer.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "DXPrimitive11.h"
#include "MTNoteDesignRing11.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// DX11 time indicator ring renderer
//******************************************************************************
class MTTimeIndicatorRing11 : public MTSceneComponent11
{
public:

	MTTimeIndicatorRing11();
	virtual ~MTTimeIndicatorRing11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData
		);

	int Update(const MTSceneUpdateContext& ctx) override;
	void Reset() override;

	int Draw(
			ID3D11DeviceContext* pContext,
			const DirectX::SimpleMath::Matrix& viewProj,
			const DirectX::SimpleMath::Vector4& lightDir
		);

	void Release();

	float GetPos() const { return m_CurPos; }

private:

	DXPrimitive11 m_Primitive;
	MTNoteDesignRing11 m_NoteDesign;
	unsigned long m_CurTickTime;
	float m_CurPos;

	int _CreateVertexOfRing(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext
		);
};
