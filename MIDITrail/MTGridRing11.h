//******************************************************************************
//
// MIDITrail / MTGridRing11
//
// DX11 grid ring renderer.
// Draws 128-segment ring lines at each bar position.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "DXPrimitive11.h"
#include "MTNoteDesignRing11.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// DX11 grid ring renderer
//******************************************************************************
class MTGridRing11 : public MTSceneComponent11
{
public:

	MTGridRing11();
	virtual ~MTGridRing11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData
		);

	int Update(const MTSceneUpdateContext& ctx) override;

	int Draw(
			ID3D11DeviceContext* pContext,
			const DirectX::SimpleMath::Matrix& viewProj,
			const DirectX::SimpleMath::Vector4& lightDir
		);

	void Release();

private:

	DXPrimitive11 m_Primitive;
	MTNoteDesignRing11 m_NoteDesign;
	bool m_isVisible;

	int _CreateVertexOfGrid(
			DXPRIMITIVE11_VERTEX* pVertex,
			unsigned long* pIndex,
			unsigned long totalTickTime,
			SMBarList* pBarList
		);

	void _CreateVertexOfRing(
			DXPRIMITIVE11_VERTEX* pVertex,
			unsigned long* pVertexIndex,
			unsigned long* pIndex,
			DirectX::SimpleMath::Vector3 basePos,
			unsigned long color
		);
};
