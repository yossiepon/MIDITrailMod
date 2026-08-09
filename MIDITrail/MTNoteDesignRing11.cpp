//******************************************************************************
//
// MIDITrail / MTNoteDesignRing11
//
// Ring note design class (DX11).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTNoteDesignRing11.h"
#include "DXH.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteDesignRing11::MTNoteDesignRing11()
{
	m_NoteAngleStep = 360.0f / (float)SM_MAX_NOTE_NUM;
	m_RingRadius = 0.0f;
}

MTNoteDesignRing11::~MTNoteDesignRing11()
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTNoteDesignRing11::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	result = MTNoteDesignMod::Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Note box center position
//******************************************************************************
Vector3 MTNoteDesignRing11::GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	Vector3 basePos = _GetNoteBasePos(curTickTime, portNo, chNo);
	float angle = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);
	return DXH::RotateYZ(0.0f, 0.0f, basePos, angle);
}

//******************************************************************************
// Note box vertex positions
//******************************************************************************
void MTNoteDesignRing11::GetNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		Vector3* pVector0,
		Vector3* pVector1,
		Vector3* pVector2,
		Vector3* pVector3,
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	Vector3 basePos0 = _GetNoteBasePos(curTickTime, portNo, chNo);
	Vector3 basePos1 = basePos0;
	basePos1.y -= GetNoteBoxWidth() / 2.0f;
	Vector3 basePos2 = basePos0;
	basePos2.y += GetNoteBoxWidth() / 2.0f;

	float angle0 = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);
	float angle1 = angle0 - (m_NoteAngleStep / 2.0f);
	float angle2 = angle0 + (m_NoteAngleStep / 2.0f);

	*pVector0 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle1);
	*pVector1 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle2);
	*pVector2 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle1);
	*pVector3 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle2);
}

//******************************************************************************
// Active note box vertex positions (elapsed time variant)
//******************************************************************************
void MTNoteDesignRing11::GetActiveNoteBoxVirtexPos(
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
		unsigned long elapsedTime
	)
{
	float curSizeRatio = 1.0f;
	if (elapsedTime < (unsigned long)m_ActiveNoteDuration) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f)
		             * (1.0f - (float)elapsedTime / (float)m_ActiveNoteDuration);
	}
	_CalcRingActiveVertices(curTickTime, portNo, chNo, noteNo,
		pVector0, pVector1, pVector2, pVector3,
		pitchBendValue, pitchBendSensitivity, curSizeRatio);
}

//******************************************************************************
// Active note box vertex positions (rate variant, Mod-specific)
//******************************************************************************
void MTNoteDesignRing11::GetActiveNoteBoxVirtexPos(
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
	float curSizeRatio = 1.0f;
	if (rate > 0.0f) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f) * GetDecayCoefficient(rate, MTNOTEDESIGN_DECAY_SATURATION_SMOOTH);
	}
	_CalcRingActiveVertices(curTickTime, portNo, chNo, noteNo,
		pVector0, pVector1, pVector2, pVector3,
		pitchBendValue, pitchBendSensitivity, curSizeRatio);
}

//******************************************************************************
// Ring active vertex calculation (shared by elapsedTime and rate variants)
//******************************************************************************
void MTNoteDesignRing11::_CalcRingActiveVertices(
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
		float curSizeRatio
	)
{
	Vector3 basePos0 = _GetNoteBasePos(curTickTime, portNo, chNo);
	Vector3 basePos1 = basePos0;
	basePos1.y -= GetNoteBoxWidth() * curSizeRatio / 2.0f;
	Vector3 basePos2 = basePos0;
	basePos2.y += GetNoteBoxWidth() * curSizeRatio / 2.0f;

	float angle0 = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);
	float angle1 = angle0 - (m_NoteAngleStep * curSizeRatio / 2.0f);
	float angle2 = angle0 + (m_NoteAngleStep * curSizeRatio / 2.0f);

	*pVector0 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle1);
	*pVector1 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle2);
	*pVector2 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle1);
	*pVector3 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle2);
}

//******************************************************************************
// Grid ring base position
//******************************************************************************
void MTNoteDesignRing11::GetGridRingBasePos(
		unsigned long tickTime,
		Vector3* pBasePos
	)
{
	float chStep = GetChStep();
	*pBasePos = Vector3(
					GetPlayPosX(tickTime),
					GetPortOriginY(0) + (chStep * (float)(SM_MAX_CH_NUM + 2)),
					GetPortOriginZ(0));
}

//******************************************************************************
// Port origin coordinates
//******************************************************************************
float MTNoteDesignRing11::GetPortOriginY(unsigned char portNo)
{
	float portIndex = (float)(m_PortIndex[portNo]);
	float portWidth = GetChStep() * (float)SM_MAX_CH_NUM;
	return (m_RingRadius + (portWidth * portIndex));
}

float MTNoteDesignRing11::GetPortOriginZ(unsigned char portNo)
{
	return 0.0f;
}

//******************************************************************************
// World move vector
//******************************************************************************
Vector3 MTNoteDesignRing11::GetWorldMoveVector()
{
	return Vector3(0.0f, 0.0f, 0.0f);
}

//******************************************************************************
// Note base position
//******************************************************************************
Vector3 MTNoteDesignRing11::_GetNoteBasePos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo
	)
{
	Vector3 v;
	v.x = GetPlayPosX(curTickTime);
	v.y = GetPortOriginY(portNo) + (GetChStep() * chNo);
	v.z = GetPortOriginZ(portNo);
	return v;
}

//******************************************************************************
// Note angle
//******************************************************************************
float MTNoteDesignRing11::_GetNoteAngle(
		unsigned char noteNo,
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	float pb = 0.0f;
	if (pitchBendValue < 0) {
		pb = m_NoteAngleStep * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		pb = m_NoteAngleStep * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	float angle = ((m_NoteAngleStep * noteNo) + (m_NoteAngleStep / 2.0f) + pb) * (-1.0f);

	return angle;
}

//******************************************************************************
// Load configuration
//******************************************************************************
int MTNoteDesignRing11::_LoadConfFile(const TCHAR* pSceneName)
{
	int result = 0;
	MTConfFile confFile;

	result = MTNoteDesignMod::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Scale"));
	if (result != 0) goto EXIT;
	confFile.GetFloat(_T("RingRadius"), &m_RingRadius, 5.0f);

EXIT:;
	return result;
}
