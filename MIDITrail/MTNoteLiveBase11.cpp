//******************************************************************************
//
// MIDITrail / MTNoteLiveBase11
//
// Live note renderer base class.
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteLiveBase11.h"


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteLiveBase11::MTNoteLiveBase11()
{
	m_NoteNum = 0;
	m_LiveMonitorDisplayDuration = 30000;
	m_LiveTimeMSec = 0;
	m_pContext = NULL;
	m_pNotePitchBend = NULL;
	m_pNoteDesign = NULL;
	m_isLightEnable = true;
	m_NoteVertexNum = 0;
	m_NoteIndexNum = 0;
	_ResetNoteStatus();
}

MTNoteLiveBase11::~MTNoteLiveBase11()
{
}

//******************************************************************************
// Reset
//******************************************************************************
void MTNoteLiveBase11::Reset()
{
	m_NoteNum = 0;
	_ResetNoteStatus();
}

//******************************************************************************
// Update (template method)
//******************************************************************************
int MTNoteLiveBase11::Update(const MTSceneUpdateContext& ctx)
{
	int result = 0;

	m_LiveTimeMSec = ctx.liveTimeMSec;

	_UpdateStatusOfNotes(m_LiveTimeMSec);

	result = _UpdateVertexOfNotes(m_LiveTimeMSec);
	if (result != 0) goto EXIT;

	{
		DirectX::SimpleMath::Matrix world = _ComputeWorldMatrix(ctx);
		m_PrimNotes.SetWorldMatrix(world);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteLiveBase11::Draw(
		ID3D11DeviceContext* pContext,
		const DirectX::SimpleMath::Matrix& viewProj,
		const DirectX::SimpleMath::Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	if (m_NoteNum > 0) {
		m_PrimNotes.SetLightEnable(m_isLightEnable);
		result = m_PrimNotes.Draw(pContext, viewProj, lightDir,
			(int)(m_NoteNum * m_NoteIndexNum / 3));
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteLiveBase11::Release()
{
	m_PrimNotes.Release();
	m_pContext = NULL;
	m_pNotePitchBend = NULL;
	m_pNoteDesign = NULL;
}

//******************************************************************************
// Update vertex of notes
//******************************************************************************
int MTNoteLiveBase11::_UpdateVertexOfNotes(unsigned long curTimeMs)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long noteNum = 0;

	result = m_PrimNotes.LockVertex(m_pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimNotes.LockIndex(m_pContext, &pIndex);
	if (result != 0) goto EXIT;

	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			result = _CreateVertexOfNote(
				m_NoteStatus[i],
				&pVertex[m_NoteVertexNum * noteNum],
				m_NoteVertexNum * noteNum,
				&pIndex[m_NoteIndexNum * noteNum],
				curTimeMs
			);
			if (result != 0) goto EXIT;
			noteNum++;
		}
	}
	m_NoteNum = noteNum;

	m_PrimNotes.UnlockVertex(m_pContext);
	m_PrimNotes.UnlockIndex(m_pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Note ON
//******************************************************************************
void MTNoteLiveBase11::SetNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	unsigned long clearedIndex = 0;
	bool isFind = false;
	unsigned long curTime = m_LiveTimeMSec;

	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			if ((m_NoteStatus[i].endTime != 0)
				&& ((curTime - m_NoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_NoteStatus[i].isActive = false;
				clearedIndex = i;
				isFind = true;
				break;
			}
		}
		else {
			clearedIndex = i;
			isFind = true;
			break;
		}
	}

	if (!isFind) {
		_ClearOldestNoteStatus(&clearedIndex);
	}

	m_NoteStatus[clearedIndex].isActive = true;
	m_NoteStatus[clearedIndex].portNo = portNo;
	m_NoteStatus[clearedIndex].chNo = chNo;
	m_NoteStatus[clearedIndex].noteNo = noteNo;
	m_NoteStatus[clearedIndex].startTime = m_LiveTimeMSec;
	m_NoteStatus[clearedIndex].endTime = 0;
}

//******************************************************************************
// Note OFF
//******************************************************************************
void MTNoteLiveBase11::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if ((m_NoteStatus[i].isActive)
			&& (m_NoteStatus[i].portNo == portNo)
			&& (m_NoteStatus[i].chNo == chNo)
			&& (m_NoteStatus[i].noteNo == noteNo)
			&& (m_NoteStatus[i].endTime == 0)) {
			m_NoteStatus[i].endTime = m_LiveTimeMSec;
			break;
		}
	}
}

//******************************************************************************
// All Note OFF
//******************************************************************************
void MTNoteLiveBase11::AllNoteOff()
{
	unsigned long curTime = m_LiveTimeMSec;
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if ((m_NoteStatus[i].isActive) && (m_NoteStatus[i].endTime == 0)) {
			m_NoteStatus[i].endTime = curTime;
		}
	}
}

//******************************************************************************
// All Note OFF (channel)
//******************************************************************************
void MTNoteLiveBase11::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	unsigned long curTime = m_LiveTimeMSec;
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if ((m_NoteStatus[i].isActive) && (m_NoteStatus[i].endTime == 0)
			&& (m_NoteStatus[i].portNo == portNo) && (m_NoteStatus[i].chNo == chNo)) {
			m_NoteStatus[i].endTime = curTime;
		}
	}
}

//******************************************************************************
// Update note status (expire old notes)
//******************************************************************************
void MTNoteLiveBase11::_UpdateStatusOfNotes(unsigned long curTimeMs)
{
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			if ((m_NoteStatus[i].endTime != 0)
				&& ((curTimeMs - m_NoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_NoteStatus[i].isActive = false;
				m_NoteStatus[i].startTime = 0;
				m_NoteStatus[i].endTime = 0;
			}
		}
	}
}

//******************************************************************************
// Clear oldest note status
//******************************************************************************
void MTNoteLiveBase11::_ClearOldestNoteStatus(unsigned long* pClearedIndex)
{
	unsigned long oldestIndex = 0;
	bool isFind = false;

	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		if (m_NoteStatus[i].isActive) {
			if (!isFind) {
				oldestIndex = i;
				isFind = true;
			}
			else if (m_NoteStatus[i].startTime < m_NoteStatus[oldestIndex].startTime) {
				oldestIndex = i;
			}
		}
	}

	m_NoteStatus[oldestIndex].isActive = false;
	*pClearedIndex = oldestIndex;
}

//******************************************************************************
// Reset note status array
//******************************************************************************
void MTNoteLiveBase11::_ResetNoteStatus()
{
	for (unsigned long i = 0; i < MTNOTELIVENOTE_MAX_NUM; i++) {
		m_NoteStatus[i].isActive = false;
		m_NoteStatus[i].portNo = 0;
		m_NoteStatus[i].chNo = 0;
		m_NoteStatus[i].noteNo = 0;
		m_NoteStatus[i].startTime = 0;
		m_NoteStatus[i].endTime = 0;
	}
}
