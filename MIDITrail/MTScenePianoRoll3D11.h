//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D11
//
// DX11 PianoRoll 3D/2D scene. Handles both playback and live modes.
// Mod features (NoteBoxMod, RippleMod, Lyrics, KeyboardCtrl) are standard.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include "MTSceneBase11.h"
#include "MTStars11.h"
#include "MTGridBox11.h"
#include "MTTimeIndicator11.h"
#include "MTPictBoard11.h"
#include "MTBackgroundImage11.h"
#include "MTDashboard11.h"
#include "MTNoteTracker.h"
#include "MTNoteBox11.h"
#include "MTNoteRipple11.h"
#include "MTNoteLyrics11.h"
#include "MTNotePitchBend.h"
#include "MTPianoKeyboardCtrlMod11.h"


//******************************************************************************
// PianoRoll 3D/2D scene (DX11)
//******************************************************************************
class MTScenePianoRoll3D11 : public MTSceneBase11
{
public:

	MTScenePianoRoll3D11(bool isLive = false, bool is2D = false);
	virtual ~MTScenePianoRoll3D11();

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

protected:

	void _ComputeDefaultViewParam(MTViewParamMap* pParamMap) override;
	float _GetViewpointCompensation() const override;
	int _UpdateComponents(const MTSceneUpdateContext& ctx) override;
	void _Reset() override;
	int _DrawSceneComponents(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				float rollAngle,
				const DirectX::SimpleMath::Vector3& camPos) override;

private:

	bool m_Is2D;
	HWND m_hWnd;

	MTStars11          m_Stars;
	MTGridBox11        m_Grid;
	MTTimeIndicator11  m_TimeIndicator;
	MTPictBoard11      m_PictBoard;
	MTBackgroundImage11 m_BackgroundImage;
	MTDashboard11      m_Dashboard;
	MTNoteTracker      m_NoteTracker;
	MTNoteBox11        m_NoteBox;
	MTNoteRipple11     m_Ripple;
	MTNoteLyrics11     m_Lyrics;
	MTNotePitchBend    m_NotePitchBend;
	MTPianoKeyboardCtrlMod11 m_KeyboardCtrl;
};
