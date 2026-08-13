//******************************************************************************
//
// MIDITrail / MTNoteLyrics11
//
// Note lyrics renderer.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTNoteEffect.h"
#include "MTNotePitchBend.h"
#include "MTFontTexture11.h"


//******************************************************************************
// DX11 note lyrics renderer
//******************************************************************************
class MTNoteLyrics11 : public MTNoteEffect
{
public:

	MTNoteLyrics11();
	virtual ~MTNoteLyrics11();

	int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				MTNotePitchBend* pNotePitchBend,
				MTNoteDesign11* pNoteDesign = NULL
			);

	void Release();

	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir,
				const DirectX::SimpleMath::Vector3& camPos
			);

protected:

	// IMTNoteTrackerListener
	void OnReset() override;

	// MTNoteEffect Template Method
	int OnActivate(NoteEffectStatus& status) override;
	int OnDeactivate(NoteEffectStatus& status) override;
	int BuildVertices(unsigned long playTimeMSec) override;

private:

	DXPrimitive11 m_Prim;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	MTNotePitchBend* m_pNotePitchBend;

	MTFontTexture11 m_FontTextures[NOTEEFFECT_MAX_SLOTS];
	ID3D11ShaderResourceView* m_pDrawSRV[NOTEEFFECT_MAX_SLOTS];
	unsigned long m_DrawSRVCount;

	DirectX::SimpleMath::Vector3 m_CamPos;
};
