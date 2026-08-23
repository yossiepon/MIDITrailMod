//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingBase11
//
// PianoRoll Ring scene base class.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2019-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneBase11.h"
#include "MTStars11.h"
#include "MTBackgroundImage11.h"
#include "MTDashboard11.h"
#include "MTDiagOverlay11.h"
#include "MTGridRingBase11.h"
#include "MTTimeIndicatorRing11.h"
#include "MTPictBoardRing11.h"
#include "MTNoteRipple11.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// PianoRoll Ring scene intermediate base (DX11)
//******************************************************************************
class MTScenePianoRollRingBase11 : public MTSceneBase11
{
public:

	MTScenePianoRollRingBase11();
	virtual ~MTScenePianoRollRingBase11();

	// IMTScene11
	int  Create(HWND hWnd, ID3D11Device* pDevice,
	            ID3D11DeviceContext* pContext,
	            SMIDILib::SMSeqData* pSeqData,
	            const MTLoadProgressContext* pProgress = NULL) override;
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
	void OnWindowResize() override;
	unsigned long GetNoteCount() const override { return 0; }

protected:

	void _ComputeDefaultViewParam(MTViewParamMap* pParamMap) override;
	float _GetViewpointCompensation() const override;
	void _Reset() override;
	int _DrawSceneComponents(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				float rollAngle,
				const DirectX::SimpleMath::Vector3& camPos) override;

	// Mode-specific hooks
	virtual int _CreateModeComponents(
					ID3D11Device* pDevice,
					ID3D11DeviceContext* pContext,
					SMIDILib::SMSeqData* pSeqData,
					const MTLoadProgressContext* pProgress = NULL) = 0;
	virtual void _RegisterModeComponents() = 0;
	virtual int _DrawNotes(
					ID3D11DeviceContext* pContext,
					const DirectX::SimpleMath::Matrix& viewProj,
					const DirectX::SimpleMath::Vector4& lightDir) = 0;
	virtual int _DrawLyrics(
					ID3D11DeviceContext* pContext,
					const DirectX::SimpleMath::Matrix& viewProj,
					const DirectX::SimpleMath::Vector4& lightDir,
					const DirectX::SimpleMath::Vector3& camPos) { return 0; }
	virtual int _DrawDashboard(
					ID3D11DeviceContext* pContext,
					unsigned int screenWidth, unsigned int screenHeight);
	virtual void _OnDashboardWindowResize();
	virtual void _SetDashboardEnable(bool isEnable);

	// Shared members
	HWND m_hWnd;

	MTStars11             m_Stars;
	MTBackgroundImage11   m_BackgroundImage;
	MTDashboard11         m_Dashboard;
	MTDiagOverlay11       m_DiagOverlay;
	MTGridRingBase11*     m_pGridRing;
	MTTimeIndicatorRing11 m_TimeIndicator;
	MTPictBoardRing11     m_PictBoard;
	MTNoteRipple11        m_Ripple;
	MTNotePitchBend       m_NotePitchBend;
};
