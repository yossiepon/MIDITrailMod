//******************************************************************************
//
// MIDITrail / MTNoteCylindricalLive11
//
// Cylindrical note renderer (Live).
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteLiveBase11.h"
#include "MTNoteDesignRing11.h"


//******************************************************************************
// Cylindrical live note renderer
//******************************************************************************
class MTNoteCylindricalLive11 : public MTNoteLiveBase11
{
public:

	MTNoteCylindricalLive11();
	virtual ~MTNoteCylindricalLive11();

	int Create(
			ID3D11Device* pDevice,
			ID3D11DeviceContext* pContext,
			const TCHAR* pSceneName,
			MTNotePitchBend* pNotePitchBend
		);

protected:

	DirectX::SimpleMath::Matrix _ComputeWorldMatrix(
				const MTSceneUpdateContext& ctx) override;
	int _CreateVertexOfNote(
				const NoteStatus& note,
				DXPRIMITIVE11_VERTEX* pVertex,
				unsigned long vertexOffset,
				unsigned long* pIndex,
				unsigned long curTimeMs) override;

private:

	MTNoteDesignRing11 m_NoteDesignLocal;
};
