//******************************************************************************
//
// MIDITrail / MTNoteDesignRing11
//
// Note design class for Ring scenes.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2019-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign11.h"

using namespace SMIDILib;


//******************************************************************************
// Ring note design class
//******************************************************************************
class MTNoteDesignRing11 : public MTNoteDesign11
{
public:

	MTNoteDesignRing11();
	virtual ~MTNoteDesignRing11();

	int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData) override;

	DirectX::SimpleMath::Vector3 GetNoteBoxCenterPosX(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				short pitchBendValue = 0,
				unsigned char pitchBendSensitivity = 0
			) override;

	void GetNoteBoxVirtexPos(
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
			) override;

	void GetActiveNoteBoxVirtexPos(
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
			) override;

	void GetNoteBoxVirtexPosLive(
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
			) override;

	void GetGridRingBasePos(
				unsigned long tickTime,
				DirectX::SimpleMath::Vector3* pBasePos
			);

	void GetGridRingBasePosLive(
				DirectX::SimpleMath::Vector3* pBasePosStart,
				DirectX::SimpleMath::Vector3* pBasePosEnd
			);

	float GetPortOriginY(unsigned char portNo) override;
	float GetPortOriginZ(unsigned char portNo) override;

	DirectX::SimpleMath::Vector3 GetWorldMoveVector() override;

	float GetPitchBendAngleShift(
				short pitchBendValue,
				unsigned char pitchBendSensitivity
			);

	float GetNoteAngleStep() const { return m_NoteAngleStep; }

protected:

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

	void _CalcRingActiveVertices(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				DirectX::SimpleMath::Vector3* pVector0,
				DirectX::SimpleMath::Vector3* pVector1,
				DirectX::SimpleMath::Vector3* pVector2,
				DirectX::SimpleMath::Vector3* pVector3,
				short pitchBendValue,
				unsigned char pitchBendSensitivity,
				float curSizeRatio
			);

	int _LoadConfFile(const TCHAR* pSceneName) override;
};
