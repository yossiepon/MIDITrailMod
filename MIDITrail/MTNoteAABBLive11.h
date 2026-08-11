//******************************************************************************
//
// MIDITrail / MTNoteAABBLive11
//
// AABB live note renderer (DX11).
// Handles Roll3D, Roll2D, and Rain modes for live MIDI input.
// Parallel to MTNoteAABBInstanced11 (Playback).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteLiveBase11.h"
#include "MTPianoKeyboardDesign11.h"

//******************************************************************************
// AABB mode
//******************************************************************************
enum class MTAABBLiveMode {
	Roll3D,
	Roll2D,
	Rain
};


//******************************************************************************
// AABB live note renderer
//******************************************************************************
class MTNoteAABBLive11 : public MTNoteLiveBase11
{
public:

	MTNoteAABBLive11();
	virtual ~MTNoteAABBLive11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName,
			MTNotePitchBend* pNotePitchBend,
			MTAABBLiveMode mode = MTAABBLiveMode::Roll3D
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

	MTNoteDesign11 m_NoteDesignLocal;
	MTPianoKeyboardDesign11 m_KeyboardDesign;
	MTAABBLiveMode m_Mode;
};
