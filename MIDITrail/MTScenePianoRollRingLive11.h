//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingLive11
//
// PianoRoll Ring scene (Live).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRollRingBase11.h"
#include "MTNoteTrackerLive.h"
#include "MTNoteCylindricalLive11.h"
#include "MTNoteDesignRingLive11.h"


//******************************************************************************
// PianoRoll Ring Live scene (DX11)
//******************************************************************************
class MTScenePianoRollRingLive11 : public MTScenePianoRollRingBase11
{
public:

	MTScenePianoRollRingLive11();
	virtual ~MTScenePianoRollRingLive11();

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

	MTNoteTrackerLive      m_NoteTrackerLive;
	MTNoteCylindricalLive11* m_pNoteBoxLive;
	MTNoteDesignRingLive11 m_NoteDesignRingLive;
};
