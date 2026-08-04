//******************************************************************************
//
// MIDITrail / MTGridBox11
//
// DX11 grid box renderer.
// Draws the wireframe grid and bar lines for the piano roll view.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTNoteDesign.h"
#include "SMIDILib.h"
#include <directxtk/SimpleMath.h>

using namespace SMIDILib;


//******************************************************************************
// DX11 grid box renderer
//******************************************************************************
class MTGridBox11
{
public:

	MTGridBox11();
	virtual ~MTGridBox11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	void Transform(float rollAngle);
	int DrawDX11(ID3D11DeviceContext* pContext,
	             const DirectX::SimpleMath::Matrix& viewProj,
	             const DirectX::SimpleMath::Vector4& lightDir,
	             float rollAngle);

	void SetEnable(bool isEnable) { m_isEnable = isEnable; }
	bool IsReady() const { return m_isReady; }

private:

	DXPrimitive11 m_Primitive;
	MTNoteDesign m_NoteDesign;
	bool m_isEnable;
	bool m_isReady;

	int _CreateVertices(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			SMSeqData* pSeqData);
};
