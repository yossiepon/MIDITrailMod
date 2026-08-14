//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain11
//
// PianoRoll Rain scene (Playback).
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRollRainBase11.h"
#include "MTNoteAABBInstanced11.h"
#include "MTNoteTracker.h"


//******************************************************************************
// PianoRoll Rain Playback scene (DX11)
//******************************************************************************
class MTScenePianoRollRain11 : public MTScenePianoRollRainBase11
{
public:

	MTScenePianoRollRain11(bool is2D = false);
	virtual ~MTScenePianoRollRain11();

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

private:

	MTNoteTracker         m_NoteTracker;
	MTNoteAABBInstanced11 m_NoteRain;
};
