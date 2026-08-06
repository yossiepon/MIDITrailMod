//******************************************************************************
//
// MIDITrail / MTNoteBox11
//
// DX11 note box renderer.
// Draws all notes as 3D boxes (24 vertices per note, 6 faces).
// Active notes are tracked via CPU forward scan and rendered with
// size/color variation based on envelope (CalcNoteEnvelope).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTSceneComponent11.h"
#include "DXPrimitive11.h"
#include "MTNoteDesignMod.h"
#include "MTNoteTracker.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// Constants
//******************************************************************************
#define MTNOTEBOX11_MAX_ACTIVENOTE_NUM  (100)
#define MTNOTEBOX11_MAX_PORT_NUM        (8)
#define MTNOTEBOX11_NOTE_VERTEX_NUM_BOX   (4 * 6)
#define MTNOTEBOX11_NOTE_INDEX_NUM_BOX    (3 * 2 * 6)
#define MTNOTEBOX11_NOTE_VERTEX_NUM_FLAT  (4)
#define MTNOTEBOX11_NOTE_INDEX_NUM_FLAT   (6)


//******************************************************************************
// DX11 note box renderer
//******************************************************************************
class MTNoteBox11 : public MTSceneComponent11
{
public:

	MTNoteBox11();
	virtual ~MTNoteBox11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNoteTracker* pNoteTracker,
				MTNotePitchBend* pNotePitchBend,
				MTNoteDesignMod* pNoteDesign = NULL,
				bool isFlatMode = false
			);
	void Release();

	int Update(const MTSceneUpdateContext& ctx) override;
	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

	void Reset() override;
	void SetSkipStatus(bool isSkipping) { m_isSkipping = isSkipping; }
	void SetLightEnable(bool enable) { m_isLightEnable = enable; }
	unsigned long GetNoteCount() const;

private:

	struct NoteStatus {
		bool isActive;
		unsigned long index;
		MTKeyStatus keyStatus;
		float keyDownRate;
	};

	DXPrimitive11 m_PrimAllNotes;
	DXPrimitive11 m_PrimActiveNotes;
	ID3D11DeviceContext* m_pContext;

	MTNoteDesignMod m_NoteDesignLocal;
	MTNoteDesignMod* m_pNoteDesign;
	MTNoteTracker* m_pNoteTracker;
	MTNotePitchBend* m_pNotePitchBend;

	unsigned long m_CurTickTime;
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurNoteIndex;
	unsigned long m_ActiveNoteNum;
	float m_KeyDownRate[MTNOTEBOX11_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	NoteStatus m_NoteStatus[MTNOTEBOX11_MAX_ACTIVENOTE_NUM];
	bool m_isSkipping;
	bool m_isFlatMode;
	bool m_isLightEnable;
	unsigned long m_NoteVertexNum;
	unsigned long m_NoteIndexNum;

	int _CreateAllNoteBox(ID3D11Device* pDevice);
	int _CreateActiveNoteBox(ID3D11Device* pDevice);

	int _CreateVertexOfNote(
				const NoteData& note,
				DXPRIMITIVE11_VERTEX* pVertex,
				unsigned long vertexOffset,
				unsigned long* pIndex,
				float keyDownRate = 0.0f,
				bool isEnablePitchBend = false
			);

	int _UpdateStatusOfActiveNotes();
	int _UpdateVertexOfActiveNotes();
};
