//******************************************************************************
//
// MIDITrail / MTTimeIndicator11
//
// Time indicator renderer.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTSceneComponent11.h"
#include "MTNoteDesign11.h"
#include "SMIDILib.h"
#include <directxtk/SimpleMath.h>

using namespace SMIDILib;


//******************************************************************************
// DX11 time indicator renderer
//******************************************************************************
class MTTimeIndicator11 : public MTSceneComponent11
{
public:

	MTTimeIndicator11();
	virtual ~MTTimeIndicator11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(ID3D11DeviceContext* pContext,
	             const DirectX::SimpleMath::Matrix& viewProj,
	             const DirectX::SimpleMath::Vector4& lightDir,
	             float rollAngle);

	void Reset() override;
	float GetPos() const;
	DirectX::SimpleMath::Vector3 GetMoveVector() const;

	bool IsReady() const { return m_isReady; }

private:

	DXPrimitive11 m_Primitive;
	DXPrimitive11 m_PrimitiveLine;
	MTNoteDesign11 m_NoteDesign;
	float m_CurPos;
	unsigned long m_CurTickTime;
	bool m_isEnableLine;
	bool m_isReady;

	int _CreatePrimitive(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	int _CreatePrimitiveLine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};
