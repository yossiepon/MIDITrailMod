//******************************************************************************
//
// MIDITrail / MTScenePianoRollRing11
//
// DX11 PianoRoll Ring scene. Notes wrap around a cylinder.
// Handles both playback and live modes (isLive flag).
// Mod features integrated (ADR-0054).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneBase11.h"
#include "MTStars11.h"
#include "MTBackgroundImage11.h"
#include "MTDashboard11.h"
#include "MTNoteDesignRing11.h"
#include "MTGridRing11.h"
#include "MTTimeIndicatorRing11.h"
#include "MTPictBoardRing11.h"
#include "MTNoteCylindricalInstanced11.h"
#include "MTNoteCylindricalLive11.h"
#include "MTNoteRipple11.h"
#include "MTNoteLyrics11.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// PianoRoll Ring scene (DX11)
//******************************************************************************
class MTScenePianoRollRing11 : public MTSceneBase11
{
public:

	MTScenePianoRollRing11(bool isLive = false);
	virtual ~MTScenePianoRollRing11();

	// IMTScene11
	const TCHAR* GetName() const override;
	int  Create(HWND hWnd, ID3D11Device* pDevice,
	            ID3D11DeviceContext* pContext,
	            SMIDILib::SMSeqData* pSeqData) override;
	void Release() override;
	int  Draw(ID3D11DeviceContext* pContext,
	          const DirectX::SimpleMath::Matrix& viewProj,
	          float rollAngle,
	          const DirectX::SimpleMath::Vector3& camPos) override;
	int  OnPlayStart() override;
	int  OnPlayEnd() override;
	int _OnRecvSequencerMsg(unsigned long param1, unsigned long param2) override;
	void SetEffect(MTEffectType type, bool isEnable) override;
	void SetPlaySpeedRatio(unsigned long ratio) override;
	unsigned long GetNoteCount() const override;

	void SetNoteOnLive(unsigned char portNo, unsigned char chNo,
	                   unsigned char noteNo, unsigned char velocity) override;
	void SetNoteOffLive(unsigned char portNo, unsigned char chNo,
	                    unsigned char noteNo) override;
	void AllNoteOffLive() override;
	void AllNoteOffOnChLive(unsigned char portNo, unsigned char chNo) override;

protected:

	void _ComputeDefaultViewParam(MTViewParamMap* pParamMap) override;
	float _GetViewpointCompensation() const override;
	void _Reset() override;
	int _DrawSceneComponents(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				float rollAngle,
				const DirectX::SimpleMath::Vector3& camPos) override;

private:

	HWND m_hWnd;

	MTNoteDesignRing11  m_NoteDesignRing;
	MTStars11           m_Stars;
	MTBackgroundImage11 m_BackgroundImage;
	MTDashboard11       m_Dashboard;
	MTGridRing11        m_GridRing;
	MTTimeIndicatorRing11 m_TimeIndicator;
	MTPictBoardRing11   m_PictBoard;
	MTNoteCylindricalInstanced11 m_NoteBox;
	MTNoteCylindricalLive11* m_pNoteBoxLive;
	MTNoteRipple11      m_Ripple;
	MTNoteLyrics11      m_Lyrics;
	MTNoteTracker       m_NoteTracker;
	MTNotePitchBend     m_NotePitchBend;
};
