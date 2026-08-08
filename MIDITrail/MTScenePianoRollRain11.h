//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain11
//
// DX11 PianoRoll Rain scene. Notes fall downward as flat rectangles.
// Handles both playback and live modes (isLive flag).
// 2D mode uses single keyboard and disables lighting.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneBase11.h"
#include "MTStars11.h"
#include "MTBackgroundImage11.h"
#include "MTDashboard11.h"
#include "MTNoteRain11.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"
#include "MTPianoKeyboardCtrlRain11.h"


//******************************************************************************
// PianoRoll Rain scene (DX11)
//******************************************************************************
class MTScenePianoRollRain11 : public MTSceneBase11
{
public:

	MTScenePianoRollRain11(bool isLive = false, bool is2D = false);
	virtual ~MTScenePianoRollRain11();

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
	void _Reset() override;
	int _DrawSceneComponents(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				float rollAngle,
				const DirectX::SimpleMath::Vector3& camPos) override;

private:

	bool m_Is2D;
	HWND m_hWnd;

	MTStars11           m_Stars;
	MTBackgroundImage11 m_BackgroundImage;
	MTDashboard11       m_Dashboard;
	MTNoteRain11        m_NoteRain;
	MTNoteTracker       m_NoteTracker;
	MTNotePitchBend     m_NotePitchBend;
	MTPianoKeyboardCtrlRain11 m_KeyboardCtrl;
};
