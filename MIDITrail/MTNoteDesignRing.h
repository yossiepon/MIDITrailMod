//******************************************************************************
//
// MIDITrail / MTNoteDesignRing
//
// Note design ring class.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// Note design ring class
//******************************************************************************
class MTNoteDesignRing : public MTNoteDesign
{
public:

	MTNoteDesignRing();
	virtual ~MTNoteDesignRing();

	void SetLiveMode();

	virtual DirectX::SimpleMath::Vector3 GetNoteBoxCenterPosX(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				short pitchBendValue = 0,
				unsigned char pitchBendSensitivity = 0
			);

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

	void GetGridRingBasePos(
			unsigned long tickTime,
			DirectX::SimpleMath::Vector3* pBasePos
		);

	void GetGridRingBasePosLive(
			DirectX::SimpleMath::Vector3* pBasePosStart,
			DirectX::SimpleMath::Vector3* pBasePosEnd
		);

	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	virtual DirectX::SimpleMath::Vector3 GetWorldMoveVector();

protected:

	bool m_isLiveMode;
	float m_NoteAngleStep;
	float m_RingRadius;

	DirectX::SimpleMath::Vector3 _GetNoteBasePos(
			unsigned long curTickTime,
			unsigned char portNo,
			unsigned char chNo
		);

	float _GetNoteAngle(
			unsigned char noteNo,
			short pitchBendValue,
			unsigned char pitchBendSensitivity
		);

	virtual int _LoadConfFile(const TCHAR* pSceneName);
};
