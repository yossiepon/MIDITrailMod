//******************************************************************************
//
// MIDITrail / MTStars11
//
// DX11 star particle renderer.
// Randomly distributes points on a sphere and renders them as a point list.
// Follows the camera position so stars appear infinitely distant.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTSceneComponent11.h"
#include <directxtk/SimpleMath.h>


//******************************************************************************
// DX11 star particle renderer
//******************************************************************************
class MTStars11 : public MTSceneComponent11
{
public:

	MTStars11();
	virtual ~MTStars11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName);
	void Release();

	void Update(const MTSceneUpdateContext& ctx) override;
	int Draw(ID3D11DeviceContext* pContext,
	             const DirectX::SimpleMath::Matrix& viewProj,
	             float rollAngle);

	bool IsReady() const { return m_isReady; }

private:

	DXPrimitive11 m_Primitive;
	int m_NumOfStars;
	bool m_isReady;

	int _CreateVertices(ID3D11DeviceContext* pContext);
	int _LoadConfFile(const TCHAR* pSceneName);
};
