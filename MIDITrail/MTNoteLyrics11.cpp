//******************************************************************************
//
// MIDITrail / MTNoteLyrics11
//
// Note lyrics renderer.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteLyrics11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteLyrics11::MTNoteLyrics11()
{
	m_pDevice = NULL;
	m_pContext = NULL;
	m_pNotePitchBend = NULL;
	m_DrawSRVCount = 0;
	m_CamPos = Vector3(0.0f, 0.0f, 0.0f);
	ZeroMemory(m_pDrawSRV, sizeof(m_pDrawSRV));
}

MTNoteLyrics11::~MTNoteLyrics11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteLyrics11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend,
		MTNoteDesign11* pNoteDesign
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	unsigned long* pIndex = NULL;

	m_pDevice = pDevice;
	m_pContext = pContext;
	m_pNotePitchBend = pNotePitchBend;

	// Initialize base class (NoteDesign, slot array)
	result = MTNoteEffect::Create(pSceneName, pSeqData, pNoteDesign);
	if (result != 0) goto EXIT;

	// Vertex buffer: 4 vertices per lyric * max slots
	vertexNum = 4 * NOTEEFFECT_MAX_SLOTS;
	indexNum = 6 * NOTEEFFECT_MAX_SLOTS;

	result = m_Prim.CreateVertexBuffer(pDevice, vertexNum);
	if (result != 0) goto EXIT;

	result = m_Prim.CreateIndexBuffer(pDevice, indexNum);
	if (result != 0) goto EXIT;

	// Fill index buffer (static pattern: 0,1,2, 0,2,3 per quad)
	result = m_Prim.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;
	for (unsigned long q = 0; q < (unsigned long)NOTEEFFECT_MAX_SLOTS; q++) {
		unsigned long base = q * 4;
		unsigned long idx = q * 6;
		pIndex[idx + 0] = base + 0;
		pIndex[idx + 1] = base + 1;
		pIndex[idx + 2] = base + 2;
		pIndex[idx + 3] = base + 0;
		pIndex[idx + 4] = base + 2;
		pIndex[idx + 5] = base + 3;
	}
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteLyrics11::Release()
{
	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		m_FontTextures[i].Clear();
	}
	ZeroMemory(m_pDrawSRV, sizeof(m_pDrawSRV));
	m_DrawSRVCount = 0;

	m_Prim.Release();
	m_pDevice = NULL;
	m_pContext = NULL;
	m_pNotePitchBend = NULL;

	MTNoteEffect::Release();
}

//******************************************************************************
// OnReset
//******************************************************************************
void MTNoteLyrics11::OnReset()
{
	MTNoteEffect::OnReset();
	m_DrawSRVCount = 0;
	ZeroMemory(m_pDrawSRV, sizeof(m_pDrawSRV));
}

//******************************************************************************
// OnActivate (create font texture for lyric)
//******************************************************************************
int MTNoteLyrics11::OnActivate(
		NoteEffectStatus& status
	)
{
	int result = 0;
	int slotIndex = (int)(&status - &m_Status[0]);

	if (m_pDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = m_FontTextures[slotIndex].SetFont(L"HGSSoeiKakugothicUB", 64, 0x00FFFFFF, false);
	if (result != 0) goto EXIT;

	result = m_FontTextures[slotIndex].CreateTexture(m_pDevice, status.lyric);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// OnDeactivate (release font texture)
//******************************************************************************
int MTNoteLyrics11::OnDeactivate(
		NoteEffectStatus& status
	)
{
	int slotIndex = (int)(&status - &m_Status[0]);
	m_FontTextures[slotIndex].Clear();
	m_pDrawSRV[slotIndex] = NULL;
	return 0;
}

//******************************************************************************
// BuildVertices
//******************************************************************************
int MTNoteLyrics11::BuildVertices(
		unsigned long playTimeMSec
	)
{
	int result = 0;
	// Declared before the first goto below for C++20 compatibility (jump over initialization is not allowed).
	Vector3 moveVec;
	Matrix world;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long activeNoteNum = 0;

	if (m_isSkipping) goto EXIT;
	if (m_pContext == NULL) goto EXIT;

	// World matrix: Rotation(rollAngle) * Translation(WorldMoveVector)
	moveVec = m_pNoteDesign->GetWorldMoveVector();
	world = Matrix::CreateRotationX(XMConvertToRadians(m_RollAngle))
	             * Matrix::CreateTranslation(moveVec);
	m_Prim.SetWorldMatrix(world);

	result = m_Prim.LockVertex(m_pContext, &pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));

	for (int ai = 0; ai < (int)m_ActiveSlots.size(); ai++) {
		int i = m_ActiveSlots[ai];
		NoteEffectStatus& s = m_Status[i];

		if ((s.portNo < NOTEEFFECT_MAX_PORT)
		 && (m_KeyDownRate[s.portNo][s.chNo][s.noteNo] < s.keyDownRate)) {

			// Get pitch bend
			short pbValue = 0;
			unsigned char pbSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;
			if (m_pNotePitchBend != NULL) {
				pbValue = m_pNotePitchBend->GetValue(s.portNo, s.chNo);
				pbSensitivity = m_pNotePitchBend->GetSensitivity(s.portNo, s.chNo);
			}

			// Note center position (raw coords — WorldMoveVector applied via world matrix)
			Vector3 center = m_pNoteDesign->GetNoteBoxCenterPosX(
				m_CurTickTime, s.portNo, s.chNo, s.noteNo,
				pbValue, pbSensitivity);

			// Lyric size from texture dimensions
			unsigned long tx = 0, ty = 0;
			m_FontTextures[i].GetTextureSize(&tx, &ty);
			float decayCoeff = m_pNoteDesign->GetDecayCoefficient(s.keyDownRate);
			float rh = tx * decayCoeff / 64.0f;
			float rw = ty * decayCoeff / 64.0f;

			if (rh <= 0.0f || rw <= 0.0f) continue;

			center.x -= m_pNoteDesign->GetRippleSpacing();

			float alpha = m_pNoteDesign->GetRippleAlpha(s.keyDownRate);
			Color color = m_pNoteDesign->GetNoteBoxColor(s.portNo, s.chNo, s.noteNo);
			unsigned long c = Color(color.R(), color.G(), color.B(), alpha).BGRA();

			DXPRIMITIVE11_VERTEX* v = &pVertex[activeNoteNum * 4];

			// Quad: 4 vertices (indexed as 0,1,2, 2,1,3)
			v[0].pos[0] = center.x; v[0].pos[1] = center.y + rh/2.0f; v[0].pos[2] = center.z - rw/2.0f;
			v[1].pos[0] = center.x; v[1].pos[1] = center.y + rh/2.0f; v[1].pos[2] = center.z + rw/2.0f;
			v[2].pos[0] = center.x; v[2].pos[1] = center.y - rh/2.0f; v[2].pos[2] = center.z + rw/2.0f;
			v[3].pos[0] = center.x; v[3].pos[1] = center.y - rh/2.0f; v[3].pos[2] = center.z - rw/2.0f;

			for (int k = 0; k < 4; k++) {
				v[k].normal[0] = -1.0f; v[k].normal[1] = 0.0f; v[k].normal[2] = 0.0f;
				v[k].color = c;
			}

			v[0].uv[0] = 1.0f; v[0].uv[1] = 0.0f;
			v[1].uv[0] = 0.0f; v[1].uv[1] = 0.0f;
			v[2].uv[0] = 0.0f; v[2].uv[1] = 1.0f;
			v[3].uv[0] = 1.0f; v[3].uv[1] = 1.0f;

			m_pDrawSRV[activeNoteNum] = m_FontTextures[i].GetTexture();
			activeNoteNum++;

			m_KeyDownRate[s.portNo][s.chNo][s.noteNo] = s.keyDownRate;
		}
	}

	m_DrawSRVCount = activeNoteNum;
	m_Prim.UnlockVertex(m_pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteLyrics11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir,
		const Vector3& camPos
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;
	if (m_DrawSRVCount == 0) goto EXIT;

	m_CamPos = camPos;

	m_Prim.SetAdditiveBlend(true);
	m_Prim.SetDepthWrite(false);
	m_Prim.SetLightEnable(false);

	// Each lyric has its own texture — draw one quad at a time
	for (unsigned long i = 0; i < m_DrawSRVCount; i++) {
		if (m_pDrawSRV[i] == NULL) continue;
		m_Prim.SetTexture(m_pDrawSRV[i]);
		result = m_Prim.Draw(pContext, viewProj, lightDir, 2, (int)i * 2);
		if (result != 0) goto EXIT;
	}

	m_Prim.SetAdditiveBlend(false);
	m_Prim.SetDepthWrite(true);

EXIT:;
	return result;
}
