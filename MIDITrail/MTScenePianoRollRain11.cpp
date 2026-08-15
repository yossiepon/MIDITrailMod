//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain11
//
// PianoRoll Rain scene (Playback).
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRollRain11.h"
#include "MTPianoKeyboardCtrlRain11.h"
#include "SMMsgParser.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRollRain11::MTScenePianoRollRain11(bool is2D)
	: MTScenePianoRollRainBase11(is2D)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRollRain11::~MTScenePianoRollRain11()
{
	Release();
}

//******************************************************************************
// Get scene name
//******************************************************************************
const TCHAR* MTScenePianoRollRain11::GetName() const
{
	return m_Is2D ? _T("PianoRollRain2D") : _T("PianoRollRain");
}

//******************************************************************************
// Mode-specific component creation
//******************************************************************************
int MTScenePianoRollRain11::_CreateModeComponents(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData,
		const MTLoadProgressContext* pProgress
	)
{
	int result = 0;

	if (pSeqData == NULL) goto EXIT;

	// NoteTracker
	result = m_NoteTracker.Create(pSeqData);
	if (result != 0) goto EXIT;

	// Keyboard (Playback)
	try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRain11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = ((MTPianoKeyboardCtrlRain11*)m_pKeyboardCtrl)->Create(
		pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend, m_Is2D);
	if (result != 0) goto EXIT;
	((MTPianoKeyboardCtrlRain11*)m_pKeyboardCtrl)->SetPlaybackPosTracking(false);

	// Note rain
	result = m_NoteRain.Create(pDevice, pContext, GetName(), pSeqData, nullptr, &m_NotePitchBend,
	                           m_Is2D ? MTAABBMode::Rain2D : MTAABBMode::Rain);
	if (result != 0) goto EXIT;

	// Dashboard
	result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, m_hWnd);
	if (result != 0) goto EXIT;
	m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

EXIT:;
	return result;
}

//******************************************************************************
// Mode-specific component registration
//******************************************************************************
void MTScenePianoRollRain11::_RegisterModeComponents()
{
	_RegisterComponent(&m_Dashboard);
	_RegisterComponent(&m_NoteTracker);
	_RegisterComponent(m_pKeyboardCtrl);
	_RegisterComponent(&m_NoteRain);
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRain11::Release()
{
	m_NoteRain.Release();
	m_NoteTracker.Release();

	MTScenePianoRollRainBase11::Release();
}

//******************************************************************************
// Draw notes
//******************************************************************************
int MTScenePianoRollRain11::_DrawNotes(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	return m_NoteRain.Draw(pContext, viewProj, lightDir);
}

//******************************************************************************
// Sequencer message reception
//******************************************************************************
int MTScenePianoRollRain11::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	result = MTScenePianoRollRainBase11::_OnRecvSequencerMsg(param1, param2);
	if (result != 0) goto EXIT;

	parser.Parse(param1, param2);

	if (parser.GetMsg() == SMMsgParser::MsgSkipStart) {
		if (parser.GetSkipStartDirection() == SMMsgParser::SkipBack) {
			m_NotePitchBend.Reset();
		}
		m_NoteRain.Reset();
		m_NoteRain.SetSkipStatus(true);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->Reset();
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(true);
		m_IsSkipping = true;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_NoteRain.SetSkipStatus(false);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(false);
		m_IsSkipping = false;
		m_NoteTracker.Seek(m_PlayTimeMSec);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get note count
//******************************************************************************
unsigned long MTScenePianoRollRain11::GetNoteCount() const
{
	return m_NoteRain.GetNoteCount();
}
