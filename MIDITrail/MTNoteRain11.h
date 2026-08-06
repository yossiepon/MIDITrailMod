//******************************************************************************
//
// MIDITrail / MTNoteRain11
//
// DX11 note rain renderer.
// Draws each note as a flat rectangle falling downward.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "DXPrimitive11.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "MTPianoKeyboardDesign.h"
#include "SMIDILib.h"

using namespace SMIDILib;

#define MTNOTERAIN_MAX_ACTIVENOTE_NUM  (100)


//******************************************************************************
// DX11 note rain renderer
//******************************************************************************
class MTNoteRain11 : public MTSceneComponent11
{
public:

	MTNoteRain11();
	virtual ~MTNoteRain11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	int Update(const MTSceneUpdateContext& ctx) override;
	void Reset() override;

	int Draw(
			ID3D11DeviceContext* pContext,
			const DirectX::SimpleMath::Matrix& viewProj,
			const DirectX::SimpleMath::Vector4& lightDir
		);

	void Release();
	float GetPos() const { return m_CurPos; }
	unsigned long GetNoteCount() const;
	void SetSkipStatus(bool isSkipping) { m_isSkipping = isSkipping; }

private:

	struct NoteStatus {
		bool isActive;
		unsigned long index;
	};

	MTNoteDesign m_NoteDesign;
	MTPianoKeyboardDesign m_KeyboardDesign;
	SMNoteList m_NoteList;

	DXPrimitive11 m_Primitive;
	DXPRIMITIVE11_VERTEX* m_pCpuVertex;
	unsigned long m_VertexCount;

	ID3D11DeviceContext* m_pContext;

	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	NoteStatus* m_pNoteStatus;
	float m_CurPos;
	bool m_isSkipping;

	MTNotePitchBend* m_pNotePitchBend;

	int _CreateAllNoteRain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void _CreateVertexOfNote(SMNote note, unsigned long noteIndex);
	int _CreateNoteStatus();
	void _UpdateStatusOfActiveNotes();
	void _UpdateVertexOfNote(unsigned long index, bool isEnablePitchBendShift = false);
	int _FlushToGPU();
};
