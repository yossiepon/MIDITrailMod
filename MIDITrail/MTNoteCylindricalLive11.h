//******************************************************************************
//
// MIDITrail / MTNoteCylindricalLive11
//
// Cylindrical (Ring) live note renderer (DX11).
// Handles ring-mode live MIDI input with cylindrical coordinate placement.
// Parallel to MTNoteCylindricalInstanced11 (Playback).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTNoteLiveBase11.h"
#include "MTNoteDesignRing11.h"


//******************************************************************************
// Cylindrical live note renderer
//******************************************************************************
class MTNoteCylindricalLive11 : public MTNoteLiveBase11
{
public:

	MTNoteCylindricalLive11();
	virtual ~MTNoteCylindricalLive11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName,
			MTNotePitchBend* pNotePitchBend
		);

	int Update(const MTSceneUpdateContext& ctx) override;

	int Draw(
			ID3D11DeviceContext* pContext,
			const DirectX::SimpleMath::Matrix& viewProj,
			const DirectX::SimpleMath::Vector4& lightDir
		);

	void Release();
	void Reset() override;

	void SetLightEnable(bool enable) { m_isLightEnable = enable; }

private:

	MTNoteDesignRing11 m_NoteDesign;
	DXPrimitive11 m_PrimNotes;
	ID3D11DeviceContext* m_pContext;
	bool m_isLightEnable;

	int _CreateNoteBuffer(ID3D11Device* pDevice);
	int _UpdateVertexOfNotes(unsigned long curTimeMs);

	int _CreateVertexOfNote(
			const NoteStatus& note,
			DXPRIMITIVE11_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIndex,
			unsigned long curTimeMs
		);
};
