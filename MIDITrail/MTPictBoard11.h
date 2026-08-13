//******************************************************************************
//
// MIDITrail / MTPictBoard11
//
// Picture board renderer.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "DXPrimitive11.h"
#include "MTNoteDesign11.h"
#include "SMIDILib.h"
#include <directxtk/SimpleMath.h>

using namespace SMIDILib;


//******************************************************************************
// DX11 picture board renderer
//******************************************************************************
class MTPictBoard11 : public MTSceneComponent11
{
public:

	MTPictBoard11();
	virtual ~MTPictBoard11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	           const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(ID3D11DeviceContext* pContext,
	             const DirectX::SimpleMath::Matrix& viewProj,
	             const DirectX::SimpleMath::Vector4& lightDir,
	             float rollAngle);
	void Reset() override;
	void OnPlayStart();
	void OnPlayEnd();

	bool IsReady() const { return m_isReady; }

private:

	DXPrimitive11 m_Primitive;
	ID3D11ShaderResourceView* m_pSRV;
	unsigned int m_ImgWidth;
	unsigned int m_ImgHeight;
	unsigned long m_CurTickTime;
	bool m_isPlay;
	bool m_isReady;
	MTNoteDesign11 m_NoteDesign;

	int _CreateVertices(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	int _LoadTexture(ID3D11Device* pDevice, const TCHAR* pSceneName);
};
