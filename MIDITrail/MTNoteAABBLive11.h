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

#include "DXPrimitive11.h"
#include "MTNoteLiveBase11.h"
#include "MTNoteDesign11.h"
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

	MTNoteDesign11 m_NoteDesign;
	MTPianoKeyboardDesign11 m_KeyboardDesign;
	DXPrimitive11 m_PrimNotes;
	ID3D11DeviceContext* m_pContext;

	MTAABBLiveMode m_Mode;
	bool m_isLightEnable;
	unsigned long m_NoteVertexNum;
	unsigned long m_NoteIndexNum;

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
