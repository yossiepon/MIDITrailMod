//******************************************************************************
//
// MIDITrail / MTPictBoardRing11
//
// DX11 picture board ring renderer.
// Wraps the keyboard texture around a cylinder at the playback position.
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "DXPrimitive11.h"
#include "DXTexture11.h"
#include "MTNoteDesignRing11.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// DX11 picture board ring renderer
//******************************************************************************
class MTPictBoardRing11 : public MTSceneComponent11
{
public:

	MTPictBoardRing11();
	virtual ~MTPictBoardRing11();

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

private:

	DXPrimitive11 m_Primitive;
	ID3D11ShaderResourceView* m_pSRV;
	MTNoteDesignRing11 m_NoteDesign;
	unsigned long m_CurTickTime;
	unsigned int m_ImgWidth;
	unsigned int m_ImgHeight;

	int _CreateVertexOfBoard(
			DXPRIMITIVE11_VERTEX* pVertex,
			unsigned long* pIndex
		);

	float _GetRippleMargin();
	int _LoadTexture(ID3D11Device* pDevice, const TCHAR* pSceneName);
};
