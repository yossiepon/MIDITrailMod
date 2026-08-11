//******************************************************************************
//
// MIDITrail / MTNoteLiveBase11
//
// Live note renderer base class (DX11).
// Manages NoteStatus array, SetNoteOn/Off events, expiry logic,
// and shared Update/Draw/Release infrastructure.
// Subclasses provide vertex generation (_CreateVertexOfNote)
// and world matrix (_ComputeWorldMatrix).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTSceneComponent11.h"
#include "MTNoteDesign11.h"
#include "MTNotePitchBend.h"

//******************************************************************************
// Parameters
//******************************************************************************
#define MTNOTELIVENOTE_MAX_NUM  (2048)


//******************************************************************************
// Live note renderer base class
//******************************************************************************
class MTNoteLiveBase11 : public MTSceneComponent11
{
public:

	MTNoteLiveBase11();
	virtual ~MTNoteLiveBase11();

	// Note events (called from scene event handlers, use wall-clock time)
	void SetNoteOn(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo,
			unsigned char velocity
		);
	void SetNoteOff(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo
		);
	void AllNoteOff();
	void AllNoteOffOnCh(unsigned char portNo, unsigned char chNo);

	int Update(const MTSceneUpdateContext& ctx) override;

	int Draw(
			ID3D11DeviceContext* pContext,
			const DirectX::SimpleMath::Matrix& viewProj,
			const DirectX::SimpleMath::Vector4& lightDir
		);

	void Release();
	void Reset() override;

	void SetLightEnable(bool enable) { m_isLightEnable = enable; }

protected:

	//----------------------------------------------------------------------
	// Note status
	//----------------------------------------------------------------------
	struct NoteStatus {
		bool isActive;
		unsigned char portNo;
		unsigned char chNo;
		unsigned char noteNo;
		unsigned long startTime;
		unsigned long endTime;
	};

	NoteStatus m_NoteStatus[MTNOTELIVENOTE_MAX_NUM];
	unsigned long m_NoteNum;
	unsigned long m_LiveMonitorDisplayDuration;
	unsigned long m_LiveTimeMSec;

	// Shared rendering infrastructure
	DXPrimitive11 m_PrimNotes;
	ID3D11DeviceContext* m_pContext;
	MTNotePitchBend* m_pNotePitchBend;
	MTNoteDesign11* m_pNoteDesign;
	bool m_isLightEnable;
	unsigned long m_NoteVertexNum;
	unsigned long m_NoteIndexNum;

	// Virtual hooks for derived classes
	virtual DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				const MTSceneUpdateContext& ctx) = 0;
	virtual int _CreateVertexOfNote(
				const NoteStatus& note,
				DXPRIMITIVE11_VERTEX* pVertex,
				unsigned long vertexOffset,
				unsigned long* pIndex,
				unsigned long curTimeMs) = 0;

	void _UpdateStatusOfNotes(unsigned long curTimeMs);
	void _ClearOldestNoteStatus(unsigned long* pClearedIndex);
	void _ResetNoteStatus();
	int _UpdateVertexOfNotes(unsigned long curTimeMs);
};
