//******************************************************************************
//
// MIDITrail / MTNoteDesignMod
//
// Note design Mod class.
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include "MTNoteDesign.h"

//******************************************************************************
// Note key status (envelope phase)
//******************************************************************************
enum MTKeyStatus {
	BeforeNoteON,
	NoteON,
	AfterNoteOFF
};

//******************************************************************************
// Note envelope result
//******************************************************************************
struct MTNoteEnvelopeResult {
	float keyDownRate;
	MTKeyStatus keyStatus;
};

//******************************************************************************
// Note design Mod class
//******************************************************************************
class MTNoteDesignMod : public MTNoteDesign
{
public:

	MTNoteDesignMod();
	virtual ~MTNoteDesignMod();

	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	// Ripple timing
	unsigned long GetRippleDecayDuration();
	unsigned long GetRippleReleaseDuration();

	// Note envelope (3-phase: Decay/Sustain/Release)
	MTNoteEnvelopeResult CalcNoteEnvelope(
				unsigned long playTimeMSec,
				unsigned long startTime,
				unsigned long endTime
			);

	// Ripple blend settings (ini values match D3D11_BLEND numeric values)
	D3D11_BLEND GetRippleSrcBlend();
	D3D11_BLEND GetRippleDestBlend();
	unsigned long GetRippleOverwriteTimes();
	float GetRippleSpacing();

	// Ripple size (rate-based decay)
	float GetRippleHeight(float rate);
	float GetRippleWidth(float rate);
	float GetRippleAlpha(float rate);
	float GetDecayCoefficient(float rate, float saturation = 20.0f);

	// Active note box vertex positions (with decay rate)
	virtual void GetActiveNoteBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				DirectX::SimpleMath::Vector3* pVector0,
				DirectX::SimpleMath::Vector3* pVector1,
				DirectX::SimpleMath::Vector3* pVector2,
				DirectX::SimpleMath::Vector3* pVector3,
				short pitchBendValue = 0,
				unsigned char pitchBendSensitivity = 0,
				float rate = 0.0f
			);

	// Active note box color (with decay rate)
	DirectX::SimpleMath::Color GetActiveNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				float rate
			);

protected:

	virtual void _Clear();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

private:

	int m_RippleDecayDuration;
	int m_RippleReleaseDuration;

	D3D11_BLEND m_RippleSrcBlend;
	D3D11_BLEND m_RippleDestBlend;

	int m_RippleOverwriteTimes;
	float m_RippleSpacing;
};
