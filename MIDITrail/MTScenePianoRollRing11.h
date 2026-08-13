//******************************************************************************
//
// MIDITrail / MTScenePianoRollRing11
//
// PianoRoll Ring scene (Playback).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2019-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRollRingBase11.h"
#include "MTNoteDesignRing11.h"
#include "MTNoteCylindricalInstanced11.h"
#include "MTNoteLyrics11.h"
#include "MTNoteTracker.h"


//******************************************************************************
// PianoRoll Ring Playback scene (DX11)
//******************************************************************************
class MTScenePianoRollRing11 : public MTScenePianoRollRingBase11
{
public:

	MTScenePianoRollRing11();
	virtual ~MTScenePianoRollRing11();

	// IMTScene11
	const TCHAR* GetName() const override;
	void Release() override;
	int _OnRecvSequencerMsg(unsigned long param1, unsigned long param2) override;
	unsigned long GetNoteCount() const override;

protected:

	int _CreateModeComponents(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				SMIDILib::SMSeqData* pSeqData) override;
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

	MTNoteDesignRing11           m_NoteDesignRing;
	MTNoteCylindricalInstanced11 m_NoteBox;
	MTNoteLyrics11               m_Lyrics;
	MTNoteTracker                m_NoteTracker;
};
