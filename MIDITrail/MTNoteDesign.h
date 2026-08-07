//******************************************************************************
//
// MIDITrail / MTNoteDesign
//
// Note design class.
// Computes note box positions, sizes, colors, and layout parameters
// from configuration files and sequence data.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <directxtk/SimpleMath.h>
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// Note design class
//******************************************************************************
class MTNoteDesign
{
public:

	MTNoteDesign();
	virtual ~MTNoteDesign();

	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	// Playback position
	float GetPlayPosX(unsigned long curTickTime);

	// Live monitor note position
	float GetLivePosX(unsigned long elapsedTime);

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

	// Live monitor display duration
	unsigned long GetLiveMonitorDisplayDuration();

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

	// Active note box vertex positions
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
				unsigned long elapsedTime = 0
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

	// Ripple parameters
	float GetRippleHeight(unsigned long elapsedTime = 0);
	float GetRippleWidth(unsigned long elapsedTime = 0);
	float GetRippleAlpha(unsigned long elapsedTime = 0);

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
				unsigned long elapsedTime
			);

	DirectX::SimpleMath::Color GetActiveNoteEmissive();
	DirectX::SimpleMath::Color GetGridLineColor();
	DirectX::SimpleMath::Color GetPlaybackSectionColor();

	int GetActiveNoteDuration() const { return m_ActiveNoteDuration; }
	float GetActiveNoteWhiteRate() const { return m_ActiveNoteWhiteRate; }
	float GetActiveNoteBoxSizeRatio() const { return m_ActiveNoteBoxSizeRatio; }

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

	int m_ActiveNoteDuration;
	float m_ActiveNoteWhiteRate;
	float m_ActiveNoteBoxSizeRatio;

	int m_RippleDuration;

	int m_LiveMonitorDisplayDuration;
	float m_LiveNoteLengthPerSecond;

	virtual void _Clear();
	virtual int _LoadConfFile(const TCHAR* pSceneName);
	int _LoadUserConf();
};
