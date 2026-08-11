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

protected:

	DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				const MTSceneUpdateContext& ctx) override;
	int _CreateVertexOfNote(
				const NoteStatus& note,
				DXPRIMITIVE11_VERTEX* pVertex,
				unsigned long vertexOffset,
				unsigned long* pIndex,
				unsigned long curTimeMs) override;

private:

	MTNoteDesignRing11 m_NoteDesignLocal;
};
