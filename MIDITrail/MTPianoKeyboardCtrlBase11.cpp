//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlBase11
//
// Piano keyboard controller base class (DX11).
//
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPianoKeyboardCtrlBase11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTPianoKeyboardCtrlBase11::MTPianoKeyboardCtrlBase11()
{
	m_pContext = NULL;
	m_pSRV = NULL;
	m_pNotePitchBend = NULL;
	m_pNoteDesign = NULL;
	m_NumKbd = 0;
	m_KeyDownDurMs = 0;
	m_KeyUpDurMs = 0;
	m_isSingleKeyboard = false;
	m_isSkipping = false;
	ZeroMemory(m_Subs, sizeof(m_Subs));
}

MTPianoKeyboardCtrlBase11::~MTPianoKeyboardCtrlBase11()
{
	Release();
}

//******************************************************************************
// Release
//******************************************************************************
void MTPianoKeyboardCtrlBase11::Release()
{
	for (unsigned long k = 0; k < SM_MAX_CH_NUM; k++) {
		_ReleaseSub(&m_Subs[k]);
	}
	m_NumKbd = 0;

	if (m_pSRV != NULL) {
		m_pSRV->Release();
		m_pSRV = NULL;
	}
	m_pNotePitchBend = NULL;
}

//******************************************************************************
// Update (template method)
//******************************************************************************
int MTPianoKeyboardCtrlBase11::Update(
		const MTSceneUpdateContext& ctx
	)
{
	int result = 0;

	for (unsigned long k = 0; k < m_NumKbd; k++) {
		if (m_Subs[k].pKeyboard == NULL) continue;

		if (!m_isSkipping) {
			_UpdateKeyStates(k, ctx);
		}

		Matrix world = _ComputeWorldMatrix(k, ctx);
		result = m_Subs[k].pKeyboard->Update(m_pContext, m_Subs[k].keyStates, world);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTPianoKeyboardCtrlBase11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	for (unsigned long k = 0; k < m_NumKbd; k++) {
		if (m_Subs[k].pKeyboard == NULL) continue;
		result = m_Subs[k].pKeyboard->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTPianoKeyboardCtrlBase11::Reset()
{
	for (unsigned long k = 0; k < m_NumKbd; k++) {
		ZeroMemory(m_Subs[k].keyCursor, sizeof(m_Subs[k].keyCursor));
		ZeroMemory(m_Subs[k].keyStates, sizeof(m_Subs[k].keyStates));
	}
}

//******************************************************************************
// Load texture
//******************************************************************************
int MTPianoKeyboardCtrlBase11::_LoadTexture(
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
	result = confFile.GetStr(_T("Keyboard"), bmpFileName, _MAX_PATH, MT_IMGFILE_KEYBOARD);
	if (result != 0) goto EXIT;

	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	result = DXTexture11::LoadFromFile(pDevice, imgFilePath, &m_pSRV);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release sub-keyboard
//******************************************************************************
void MTPianoKeyboardCtrlBase11::_ReleaseSub(MTKbdSub* pSub)
{
	if (pSub->pKeyboard != NULL) {
		pSub->pKeyboard->Release();
		delete pSub->pKeyboard;
		pSub->pKeyboard = NULL;
	}
	if (pSub->pNotes != NULL) {
		free(pSub->pNotes);
		pSub->pNotes = NULL;
	}
	pSub->noteCount = 0;
}
