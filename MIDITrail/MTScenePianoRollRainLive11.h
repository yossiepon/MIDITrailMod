//******************************************************************************
//
// MIDITrail / MTScenePianoRollRainLive11
//
// PianoRoll Rain scene (Live).
//
// Copyright (C) 2012-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRollRainBase11.h"
#include "MTNoteTrackerLive.h"
#include "MTNoteAABBLive11.h"


//******************************************************************************
// PianoRoll Rain Live scene (DX11)
//******************************************************************************
class MTScenePianoRollRainLive11 : public MTScenePianoRollRainBase11
{
public:

	MTScenePianoRollRainLive11(bool is2D = false);
	virtual ~MTScenePianoRollRainLive11();

	// IMTScene11
	const TCHAR* GetName() const override;
	void Release() override;
	int  OnPlayStart() override;
	int  OnPlayEnd() override;

	void SetNoteOnLive(unsigned char portNo, unsigned char chNo,
	                   unsigned char noteNo, unsigned char velocity) override;
	void SetNoteOffLive(unsigned char portNo, unsigned char chNo,
	                    unsigned char noteNo) override;
	void AllNoteOffLive() override;
	void AllNoteOffOnChLive(unsigned char portNo, unsigned char chNo) override;

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

	MTNoteTrackerLive  m_NoteTrackerLive;
	MTNoteAABBLive11*  m_pNoteRainLive;
};
