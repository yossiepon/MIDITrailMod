//******************************************************************************
//
// MIDITrail / MTNoteRipple11
//
// DX11 note ripple renderer.
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "DXTexture11.h"
#include "MTNoteRipple11.h"
#include "MTSceneConst.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteRipple11::MTNoteRipple11()
{
	m_pContext = NULL;
	m_pTextureSRV = NULL;
	m_pNotePitchBend = NULL;
	m_ActiveNoteNum = 0;
	m_CamPos = Vector3(0.0f, 0.0f, 0.0f);
}

MTNoteRipple11::~MTNoteRipple11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteRipple11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend,
		MTNoteDesignMod* pNoteDesign
	)
{
	int result = 0;
	unsigned long vertexNum = 0;

	m_pContext = pContext;
	m_pNotePitchBend = pNotePitchBend;

	// Initialize base class (NoteDesign, slot array)
	result = MTNoteEffect::Create(pSceneName, pSeqData, pNoteDesign);
	if (result != 0) goto EXIT;

	// Load ripple texture
	result = _LoadTexture(pDevice, pSceneName);
	if (result != 0) goto EXIT;

	// Create vertex buffer: 6 vertices per ripple * overwrite times * max slots
	{
		unsigned long overwriteTimes = m_pNoteDesign->GetRippleOverwriteTimes();
		vertexNum = 6 * overwriteTimes * NOTEEFFECT_MAX_SLOTS;
	}
	result = m_Prim.CreateVertexBuffer(pDevice, vertexNum);
	if (result != 0) goto EXIT;

	m_Prim.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTNoteRipple11::Release()
{
	if (m_pTextureSRV != NULL) {
		m_pTextureSRV->Release();
		m_pTextureSRV = NULL;
	}
	m_Prim.Release();
	m_pNotePitchBend = NULL;
	m_pContext = NULL;

	MTNoteEffect::Release();
}

//******************************************************************************
// OnActivate / OnDeactivate (NOP for ripple)
//******************************************************************************
int MTNoteRipple11::OnActivate(NoteEffectStatus& status)
{
	return 0;
}

int MTNoteRipple11::OnDeactivate(NoteEffectStatus& status)
{
	return 0;
}

//******************************************************************************
// BuildVertices
//******************************************************************************
int MTNoteRipple11::BuildVertices(
		unsigned long playTimeMSec
	)
{
	int result = 0;

	if (m_isSkipping) goto EXIT;
	if (m_pContext == NULL) goto EXIT;

	// World matrix: Rotation(rollAngle) * Translation(WorldMoveVector)
	Vector3 moveVec = m_pNoteDesign->GetWorldMoveVector();
	Matrix world = Matrix::CreateRotationX(XMConvertToRadians(m_RollAngle))
	             * Matrix::CreateTranslation(moveVec);
	m_Prim.SetWorldMatrix(world);

	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	result = m_Prim.LockVertex(m_pContext, &pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));

	unsigned long overwriteTimes = m_pNoteDesign->GetRippleOverwriteTimes();
	float spacing = m_pNoteDesign->GetRippleSpacing();
	unsigned long activeNoteNum = 0;

	for (int i = 0; i < NOTEEFFECT_MAX_SLOTS; i++) {
		if (!m_Status[i].isActive) continue;

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

			// Ripple size from rate
			float rh = m_pNoteDesign->GetRippleHeight(s.keyDownRate);
			float rw = m_pNoteDesign->GetRippleWidth(s.keyDownRate);
			float alpha = m_pNoteDesign->GetRippleAlpha(s.keyDownRate);

			if (rh <= 0.0f || rw <= 0.0f) continue;

			// Z-fighting avoidance: offset ripple along X from camera
			if (center.x < m_CamPos.x) {
				center.x -= spacing * (MTNOTELYRICS_MAX_LYRICS_NUM
					+ MTNOTERIPPLE_MAX_RIPPLE_NUM - (i + 1));
			}
			else {
				center.x -= spacing * (MTNOTELYRICS_MAX_LYRICS_NUM + i + 1);
			}

			// Note color
			Color color = m_pNoteDesign->GetNoteBoxColor(s.portNo, s.chNo, s.noteNo);
			unsigned long c = Color(color.R(), color.G(), color.B(), alpha).BGRA();

			for (unsigned long j = 0; j < overwriteTimes; j++) {
				DXPRIMITIVE11_VERTEX* v = &pVertex[activeNoteNum * 6];

				// Quad: two triangles (0-1-2, 3-4-5)
				v[0].pos[0] = center.x; v[0].pos[1] = center.y + rh/2.0f; v[0].pos[2] = center.z + rw/2.0f;
				v[1].pos[0] = center.x; v[1].pos[1] = center.y + rh/2.0f; v[1].pos[2] = center.z - rw/2.0f;
				v[2].pos[0] = center.x; v[2].pos[1] = center.y - rh/2.0f; v[2].pos[2] = center.z + rw/2.0f;
				v[3] = v[2];
				v[4] = v[1];
				v[5].pos[0] = center.x; v[5].pos[1] = center.y - rh/2.0f; v[5].pos[2] = center.z - rw/2.0f;

				// Normal, color, UV
				for (int k = 0; k < 6; k++) {
					v[k].normal[0] = 0.0f; v[k].normal[1] = 0.0f; v[k].normal[2] = -1.0f;
					v[k].color = c;
				}
				v[0].uv[0] = 0.0f; v[0].uv[1] = 0.0f;
				v[1].uv[0] = 1.0f; v[1].uv[1] = 0.0f;
				v[2].uv[0] = 0.0f; v[2].uv[1] = 1.0f;
				v[3].uv[0] = 0.0f; v[3].uv[1] = 1.0f;
				v[4].uv[0] = 1.0f; v[4].uv[1] = 0.0f;
				v[5].uv[0] = 1.0f; v[5].uv[1] = 1.0f;

				activeNoteNum++;
			}

			m_KeyDownRate[s.portNo][s.chNo][s.noteNo] = s.keyDownRate;
		}
	}

	m_ActiveNoteNum = activeNoteNum;
	m_Prim.UnlockVertex(m_pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteRipple11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir,
		const Vector3& camPos
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;
	if (m_ActiveNoteNum == 0) goto EXIT;

	m_CamPos = camPos;

	m_Prim.SetTexture(m_pTextureSRV);
	m_Prim.SetAdditiveBlend(true);
	m_Prim.SetDepthWrite(false);
	m_Prim.SetLightEnable(false);

	result = m_Prim.Draw(pContext, viewProj, lightDir,
		2 * (int)m_ActiveNoteNum);
	if (result != 0) goto EXIT;

	m_Prim.SetAdditiveBlend(false);
	m_Prim.SetDepthWrite(true);

EXIT:;
	return result;
}

//******************************************************************************
// Load ripple texture
//******************************************************************************
int MTNoteRipple11::_LoadTexture(
		ID3D11Device* pDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR bmpFileName[_MAX_PATH] = {_T('\0')};
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Bitmap"));
	if (result != 0) goto EXIT;
	result = confFile.GetStr(_T("Ripple"), bmpFileName, _MAX_PATH, MT_IMGFILE_RIPPLE);
	if (result != 0) goto EXIT;

	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	result = DXTexture11::LoadFromFile(pDevice, imgFilePath, &m_pTextureSRV);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}
