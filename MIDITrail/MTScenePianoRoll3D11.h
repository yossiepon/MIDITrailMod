//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D11
//
// PianoRoll 3D/2D scene (Playback).
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRoll3DBase11.h"
#include "MTNoteTracker.h"
#include "MTNoteAABBInstanced11.h"
#include "MTNoteLyrics11.h"


//******************************************************************************
// PianoRoll 3D/2D Playback scene (DX11)
//******************************************************************************
class MTScenePianoRoll3D11 : public MTScenePianoRoll3DBase11
{
public:

	MTScenePianoRoll3D11(bool is2D = false);
	virtual ~MTScenePianoRoll3D11();

	// IMTScene11
	const TCHAR* GetName() const override;
	void Release() override;
	int _OnRecvSequencerMsg(unsigned long param1, unsigned long param2) override;
	unsigned long GetNoteCount() const override;

protected:

	int _CreateModeComponents(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				SMIDILib::SMSeqData* pSeqData,
				const MTLoadProgressContext* pProgress = NULL) override;
	void _RegisterModeComponents() override;
	int _DrawNotes(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir) override;
	int _DrawLyrics(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir,
				const DirectX::SimpleMath::Vector3& camPos) override;

private:

	MTNoteTracker         m_NoteTracker;
	MTNoteAABBInstanced11 m_NoteBox;
	MTNoteLyrics11        m_Lyrics;
};
