//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DLive11
//
// PianoRoll 3D/2D scene (Live).
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRoll3DBase11.h"
#include "MTNoteTrackerLive.h"
#include "MTNoteAABBLive11.h"
#include "MTNoteDesignLive11.h"
#include "MTDashboardLive11.h"


//******************************************************************************
// PianoRoll 3D/2D Live scene (DX11)
//******************************************************************************
class MTScenePianoRoll3DLive11 : public MTScenePianoRoll3DBase11
{
public:

	MTScenePianoRoll3DLive11(bool is2D = false);
	virtual ~MTScenePianoRoll3DLive11();

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
	int  OnMIDIINDeviceChanged(const TCHAR* pName) override;

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
	int _DrawDashboard(
				ID3D11DeviceContext* pContext,
				unsigned int screenWidth, unsigned int screenHeight) override;
	void _OnDashboardWindowResize() override;
	void _SetDashboardEnable(bool isEnable) override;

private:

	MTNoteTrackerLive  m_NoteTrackerLive;
	MTNoteAABBLive11*  m_pNoteBoxLive;
	MTNoteDesignLive11 m_NoteDesignLive;
	MTDashboardLive11  m_DashboardLive;
};
