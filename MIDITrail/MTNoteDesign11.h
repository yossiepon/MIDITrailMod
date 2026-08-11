//******************************************************************************
//
// MIDITrail / MTNoteDesign11
//
// Note design class for DX11.
// Unified from MTNoteDesign + MTNoteDesignMod (ADR-0054 Mod standardization).
// Playback uses rate-based decay; Live methods use elapsedTime-based positioning.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include "SMIDILib.h"

class MTNotePitchBend;

using namespace SMIDILib;


//******************************************************************************
// Note key status (envelope phase)
//******************************************************************************
enum MTKeyStatus {
	BeforeNoteON,
	NoteON,
	AfterNoteOFF
};

//******************************************************************************
// Decay curve saturation constants
//******************************************************************************
#define MTNOTEDESIGN_DECAY_SATURATION_SMOOTH   30.0f
#define MTNOTEDESIGN_DECAY_SATURATION_HOLD     20.0f

#define MTNOTEDESIGN_STRINGIFY_(x) #x
#define MTNOTEDESIGN_STRINGIFY(x)  MTNOTEDESIGN_STRINGIFY_(x)

//******************************************************************************
// Note envelope result
//******************************************************************************
struct MTNoteEnvelopeResult {
	float keyDownRate;
	MTKeyStatus keyStatus;
};

//******************************************************************************
// Envelope configuration (for GPU shader parameters)
//******************************************************************************
struct MTEnvelopeConfig {
	float decayDurationMs;
	float releaseDurationMs;
	float decayRatio;
	float sustainRatio;
};

//******************************************************************************
// Note design class for DX11
//******************************************************************************
class MTNoteDesign11
{
public:

	MTNoteDesign11();
	virtual ~MTNoteDesign11();

	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	// Playback position
	float GetPlayPosX(unsigned long curTickTime);

	// Live monitor note position
	float GetLivePosX(unsigned long elapsedTime);

	// Live monitor display duration
	unsigned long GetLiveMonitorDisplayDuration();

	// Note box center position
	virtual DirectX::SimpleMath::Vector3 GetNoteBoxCenterPosX(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				short pitchBendValue = 0,
				unsigned char pitchBendSensitivity = 0
			);

	// Note box dimensions
	float GetNoteBoxHeight();
	float GetNoteBoxWidth();

	// Spacing
	float GetNoteStep();
	float GetChStep();

	// Pitch bend Y shift in note-space coordinates
	float GetPitchBendShift(short pitchBendValue, unsigned char pitchBendSensitivity);
	float GetMaxPitchBendShift(MTNotePitchBend* pNotePitchBend, unsigned char portNo);

	// Note box vertex positions (4 corners of the front face)
	virtual void GetNoteBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				DirectX::SimpleMath::Vector3* pVector0,
				DirectX::SimpleMath::Vector3* pVector1,
				DirectX::SimpleMath::Vector3* pVector2,
				DirectX::SimpleMath::Vector3* pVector3,
				short pitchBendValue = 0,
				unsigned char pitchBendSensitivity = 0
			);

	// Active note box vertex positions (rate-based decay)
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

	// Live monitor note box vertex positions
	virtual void GetNoteBoxVirtexPosLive(
				unsigned long elapsedTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				DirectX::SimpleMath::Vector3* pVector0,
				DirectX::SimpleMath::Vector3* pVector1,
				DirectX::SimpleMath::Vector3* pVector2,
				DirectX::SimpleMath::Vector3* pVector3,
				short pitchBendValue = 0,
				unsigned char pitchBendSensitivity = 0
			);

	// Grid box vertex positions
	void GetGridBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				DirectX::SimpleMath::Vector3* pVector0,
				DirectX::SimpleMath::Vector3* pVector1,
				DirectX::SimpleMath::Vector3* pVector2,
				DirectX::SimpleMath::Vector3* pVector3
			);

	// Live grid box vertex positions
	void GetGridBoxVirtexPosLive(
				unsigned long elapsedTime,
				unsigned char portNo,
				DirectX::SimpleMath::Vector3* pVector0,
				DirectX::SimpleMath::Vector3* pVector1,
				DirectX::SimpleMath::Vector3* pVector2,
				DirectX::SimpleMath::Vector3* pVector3
			);

	// Playback section vertex positions
	void GetPlaybackSectionVirtexPos(
				unsigned long curTickTime,
				DirectX::SimpleMath::Vector3* pVector0,
				DirectX::SimpleMath::Vector3* pVector1,
				DirectX::SimpleMath::Vector3* pVector2,
				DirectX::SimpleMath::Vector3* pVector3
			);

	// Ripple size (rate-based decay)
	float GetRippleHeight(float rate);
	float GetRippleWidth(float rate);
	float GetRippleAlpha(float rate);
	float GetDecayCoefficient(float rate, float saturation = MTNOTEDESIGN_DECAY_SATURATION_HOLD);

	// Ripple timing
	unsigned long GetRippleDecayDuration();
	unsigned long GetRippleReleaseDuration();

	// Envelope configuration for GPU shader
	MTEnvelopeConfig GetEnvelopeConfig();

	// Note envelope (3-phase: Decay/Sustain/Release)
	virtual MTNoteEnvelopeResult CalcNoteEnvelope(
				unsigned long playTimeMSec,
				unsigned long startTime,
				unsigned long endTime
			);

protected:

	static MTNoteEnvelopeResult _CalcLiveEnvelope(
				unsigned long playTimeMSec,
				unsigned long startTime,
				unsigned long endTime,
				const MTEnvelopeConfig& envConfig,
				unsigned long decayDuration,
				unsigned long releaseDuration
			);

public:

	// Ripple blend settings
	D3D11_BLEND GetRippleSrcBlend();
	D3D11_BLEND GetRippleDestBlend();
	unsigned long GetRippleOverwriteTimes();
	float GetRippleSpacing();

	// Picture board relative position
	float GetPictBoardRelativePos();

	// Port origin coordinates
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	// World move vector
	virtual DirectX::SimpleMath::Vector3 GetWorldMoveVector();

	// Colors
	DirectX::SimpleMath::Color GetNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo
			);

	DirectX::SimpleMath::Color GetActiveNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				float rate
			);

	DirectX::SimpleMath::Color GetActiveNoteEmissive();
	DirectX::SimpleMath::Color GetGridLineColor();
	DirectX::SimpleMath::Color GetPlaybackSectionColor();

	float GetActiveNoteWhiteRate();
	float GetActiveNoteBoxSizeRatio();

protected:

	enum NoteColorType {
		Channel,
		Scale
	};

	unsigned long m_TimeDivision;
	float m_QuarterNoteLength;
	float m_NoteBoxHeight;
	float m_NoteBoxWidth;
	float m_NoteStep;
	float m_ChStep;
	float m_RippleHeight;
	float m_RippleWidth;
	float m_PictBoardRelativePos;
	SMPortList m_PortList;
	unsigned char m_PortIndex[256];

	NoteColorType m_NoteColorType;
	DirectX::SimpleMath::Color m_NoteColor[16];
	DirectX::SimpleMath::Color m_NoteColorOfScale[12];
	DirectX::SimpleMath::Color m_ActiveNoteEmissive;
	DirectX::SimpleMath::Color m_GridLineColor;
	DirectX::SimpleMath::Color m_PlaybackSectionColor;

	float m_ActiveNoteWhiteRate;
	float m_ActiveNoteBoxSizeRatio;

	int m_LiveMonitorDisplayDuration;
	float m_LiveNoteLengthPerSecond;

	int m_RippleDecayDuration;
	int m_RippleReleaseDuration;
	D3D11_BLEND m_RippleSrcBlend;
	D3D11_BLEND m_RippleDestBlend;
	int m_RippleOverwriteTimes;
	float m_RippleSpacing;

	virtual void _Clear();
	virtual int _LoadConfFile(const TCHAR* pSceneName);
	int _LoadUserConf();
};
