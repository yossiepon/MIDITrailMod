//******************************************************************************
//
// MIDITrail / MTNoteDesignMod
//
// Note design Mod class.
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTConfFile.h"
#include "MTNoteDesignMod.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteDesignMod::MTNoteDesignMod()
{
	_Clear();
}

MTNoteDesignMod::~MTNoteDesignMod()
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTNoteDesignMod::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	MTNoteDesign::Initialize(pSceneName, pSeqData);

	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Ripple parameters
//******************************************************************************
unsigned long MTNoteDesignMod::GetRippleDecayDuration()
{
	return (unsigned long)m_RippleDecayDuration;
}

unsigned long MTNoteDesignMod::GetRippleReleaseDuration()
{
	return (unsigned long)m_RippleReleaseDuration;
}

D3D11_BLEND MTNoteDesignMod::GetRippleSrcBlend()
{
	return m_RippleSrcBlend;
}

D3D11_BLEND MTNoteDesignMod::GetRippleDestBlend()
{
	return m_RippleDestBlend;
}

unsigned long MTNoteDesignMod::GetRippleOverwriteTimes()
{
	return (unsigned long)m_RippleOverwriteTimes;
}

float MTNoteDesignMod::GetRippleSpacing()
{
	return m_RippleSpacing;
}

//******************************************************************************
// Envelope configuration for GPU shader
//******************************************************************************
MTEnvelopeConfig MTNoteDesignMod::GetEnvelopeConfig()
{
	MTEnvelopeConfig config;
	config.decayDurationMs = (float)m_RippleDecayDuration;
	config.releaseDurationMs = (float)m_RippleReleaseDuration;
	config.decayRatio = 0.3f;
	config.sustainRatio = 0.4f;
	return config;
}

//******************************************************************************
// Note envelope (3-phase: Decay/Sustain/Release)
//******************************************************************************
MTNoteEnvelopeResult MTNoteDesignMod::CalcNoteEnvelope(
		unsigned long playTimeMSec,
		unsigned long startTime,
		unsigned long endTime
	)
{
	MTNoteEnvelopeResult result = { 0.0f, BeforeNoteON };

	if (playTimeMSec > endTime) {
		return result;
	}

	unsigned long decayDuration = (unsigned long)m_RippleDecayDuration;
	unsigned long releaseDuration = (unsigned long)m_RippleReleaseDuration;
	unsigned long noteLen = endTime - startTime;

	float decayRatio = 0.3f;
	float sustainRatio = 0.4f;
	float releaseRatio = 0.3f;

	if (noteLen < decayDuration) {
		// Case A: no adjustment (whole note stays in Decay)
	}
	else if (noteLen < (decayDuration + releaseDuration)) {
		// Case B: eliminate Sustain, shrink Release to fit
		releaseDuration = noteLen - decayDuration;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}
	else if (noteLen < (decayDuration + releaseDuration) * 2) {
		// Case C: midpoint split, eliminate Sustain
		unsigned long midTime = (startTime + decayDuration) / 2
		                      + (endTime - releaseDuration) / 2;
		decayDuration = midTime - startTime;
		releaseDuration = endTime - midTime;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}

	// Phase 1: Decay
	if (playTimeMSec < (startTime + decayDuration)) {
		result.keyStatus = BeforeNoteON;
		if (decayDuration == 0) {
			result.keyDownRate = 0.0f;
		}
		else {
			result.keyDownRate = decayRatio * (float)(playTimeMSec - startTime) / (float)decayDuration;
		}
		return result;
	}
	// Phase 2: Sustain
	if (playTimeMSec <= (endTime - releaseDuration)) {
		result.keyStatus = NoteON;
		unsigned long denominator = noteLen - (decayDuration + releaseDuration);
		if (denominator > 0) {
			result.keyDownRate = decayRatio + sustainRatio
				* (float)(playTimeMSec - (startTime + decayDuration)) / (float)denominator;
		}
		else {
			result.keyDownRate = decayRatio + sustainRatio;
		}
		return result;
	}
	// Phase 3: Release
	result.keyStatus = AfterNoteOFF;
	if (releaseDuration == 0) {
		result.keyDownRate = 1.0f;
	}
	else {
		result.keyDownRate = decayRatio + sustainRatio + releaseRatio
			* (float)(playTimeMSec - (endTime - releaseDuration)) / (float)releaseDuration;
	}
	return result;
}

//******************************************************************************
// Ripple size (rate-based decay)
//******************************************************************************
float MTNoteDesignMod::GetRippleHeight(float rate)
{
	return m_RippleHeight * GetDecayCoefficient(rate);
}

float MTNoteDesignMod::GetRippleWidth(float rate)
{
	return m_RippleWidth * GetDecayCoefficient(rate);
}

float MTNoteDesignMod::GetRippleAlpha(float rate)
{
	return GetDecayCoefficient(rate);
}

float MTNoteDesignMod::GetDecayCoefficient(float rate, float saturation)
{
	float coeff = 1.0f;

	if (rate < 0.5f) {
		coeff = (powf(2.0f, (0.5f - rate) * 8.0f) + 14.0f) / saturation;
	}
	else {
		coeff = (16.0f - powf(2.0f, (rate - 0.5f) * 8.0f)) / saturation;
	}

	if (coeff > 1.0f) coeff = 1.0f;

	return coeff;
}

//******************************************************************************
// Active note box vertex positions
//******************************************************************************
void MTNoteDesignMod::GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		Vector3* pVector0,
		Vector3* pVector1,
		Vector3* pVector2,
		Vector3* pVector3,
		short pitchBendValue,
		unsigned char pitchBendSensitivity,
		float rate
	)
{
	Vector3 center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo,
	                                        pitchBendValue, pitchBendSensitivity);
	float curSizeRatio = 1.0f;
	if (rate > 0.0f) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f) * GetDecayCoefficient(rate, 30.0f);
	}

	float bh = GetNoteBoxHeight() * curSizeRatio;
	float bw = GetNoteBoxWidth() * curSizeRatio;

	*pVector0 = Vector3(center.x, center.y + bh / 2.0f, center.z + bw / 2.0f);
	*pVector1 = Vector3(center.x, center.y + bh / 2.0f, center.z - bw / 2.0f);
	*pVector2 = Vector3(center.x, center.y - bh / 2.0f, center.z + bw / 2.0f);
	*pVector3 = Vector3(center.x, center.y - bh / 2.0f, center.z - bw / 2.0f);
}

//******************************************************************************
// Active note box color
//******************************************************************************
Color MTNoteDesignMod::GetActiveNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		float rate
	)
{
	float alpha = GetDecayCoefficient(rate, 30.0f);
	Color color = GetNoteBoxColor(portNo, chNo, noteNo);

	float r = color.R() + ((1.0f - color.R()) * alpha * m_ActiveNoteWhiteRate);
	float g = color.G() + ((1.0f - color.G()) * alpha * m_ActiveNoteWhiteRate);
	float b = color.B() + ((1.0f - color.B()) * alpha * m_ActiveNoteWhiteRate);
	float a = color.A();

	return Color(r, g, b, a);
}

//******************************************************************************
// Clear
//******************************************************************************
void MTNoteDesignMod::_Clear()
{
	MTNoteDesign::_Clear();

	m_RippleDecayDuration = 100;
	m_RippleReleaseDuration = 250;
	m_RippleSrcBlend = D3D11_BLEND_SRC_ALPHA;
	m_RippleDestBlend = D3D11_BLEND_ONE;
	m_RippleOverwriteTimes = 3;
	m_RippleSpacing = 0.002f;
}

//******************************************************************************
// Load configuration
//******************************************************************************
int MTNoteDesignMod::_LoadConfFile(const TCHAR* pSceneName)
{
	int result = 0;
	MTConfFile confFile;

	result = MTNoteDesign::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	confFile.GetInt(_T("DecayDuration"), &m_RippleDecayDuration, 100);
	confFile.GetInt(_T("ReleaseDuration"), &m_RippleReleaseDuration, 250);

	// Blend values: D3D9 D3DBLEND and D3D11 D3D11_BLEND share the same numeric values
	// for common modes (SRCALPHA=5, ONE=2, etc.), so ini files are compatible.
	confFile.GetInt(_T("SrcBlend"), (int*)&m_RippleSrcBlend, D3D11_BLEND_SRC_ALPHA);
	confFile.GetInt(_T("DestBlend"), (int*)&m_RippleDestBlend, D3D11_BLEND_ONE);

	confFile.GetInt(_T("OverwriteTimes"), &m_RippleOverwriteTimes, 3);
	confFile.GetFloat(_T("Spacing"), &m_RippleSpacing, 0.002f);

EXIT:;
	return result;
}
