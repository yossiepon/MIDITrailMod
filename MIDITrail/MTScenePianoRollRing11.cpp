//******************************************************************************
//
// MIDITrail / MTScenePianoRollRing11
//
// PianoRoll Ring scene (Playback).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2019-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMIDILib.h"
#include "MTScenePianoRollRing11.h"
#include "MTGridRing11.h"
#include "SMMsgParser.h"

using namespace YNBaseLib;
using namespace SMIDILib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRollRing11::MTScenePianoRollRing11()
	: MTScenePianoRollRingBase11()
{
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRollRing11::~MTScenePianoRollRing11()
{
	Release();
}

//******************************************************************************
// Get scene name
//******************************************************************************
const TCHAR* MTScenePianoRollRing11::GetName() const
{
	return _T("PianoRollRing");
}

//******************************************************************************
// Mode-specific component creation
//******************************************************************************
int MTScenePianoRollRing11::_CreateModeComponents(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData,
		const MTLoadProgressContext* pProgress
	)
{
	int result = 0;

	if (pSeqData == NULL) goto EXIT;

	// Ring NoteDesign (shared by NoteBox, Ripple, Lyrics)
	result = m_NoteDesignRing.Initialize(GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// Grid Ring (Playback)
	try { m_pGridRing = new MTGridRing11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = ((MTGridRing11*)m_pGridRing)->Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// Time indicator ring
	result = m_TimeIndicator.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// PictBoard ring
	result = m_PictBoard.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// Dashboard
	result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, m_hWnd);
	if (result != 0) goto EXIT;

	// NoteTracker (progress band: 0% ~ 20%)
	{
		MTProgressBand band = { pProgress, 0.0f, 0.2f };
		MTLoadProgressContext ctx = (pProgress != NULL) ? band.ToContext() : MTLoadProgressContext();
		result = m_NoteTracker.Create(pSeqData, (pProgress != NULL) ? &ctx : NULL);
	}
	if (result != 0) goto EXIT;

	// NoteBox (GPU-instanced Ring renderer, progress band: 20% ~ 90%)
	{
		MTProgressBand band = { pProgress, 0.2f, 0.9f };
		MTLoadProgressContext ctx = (pProgress != NULL) ? band.ToContext() : MTLoadProgressContext();
		result = m_NoteBox.Create(pDevice, pContext, GetName(), pSeqData,
		                          &m_NoteTracker, &m_NotePitchBend, &m_NoteDesignRing,
		                          (pProgress != NULL) ? &ctx : NULL);
	}
	if (result != 0) goto EXIT;

	// Ripple (with Ring NoteDesign injection)
	result = m_Ripple.Create(pDevice, pContext, GetName(), pSeqData,
	                         &m_NotePitchBend, &m_NoteDesignRing);
	if (result != 0) goto EXIT;
	m_NoteTracker.AddListener(&m_Ripple, NoteEventType::Note);

	// Lyrics (with Ring NoteDesign injection)
	result = m_Lyrics.Create(pDevice, pContext, GetName(), pSeqData,
	                         &m_NotePitchBend, &m_NoteDesignRing);
	if (result != 0) goto EXIT;
	m_NoteTracker.AddListener(&m_Lyrics, NoteEventType::Lyric);
	m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

EXIT:;
	return result;
}

//******************************************************************************
// Mode-specific component registration
//******************************************************************************
void MTScenePianoRollRing11::_RegisterModeComponents()
{
	_RegisterComponent(&m_Dashboard);
	_RegisterComponent(&m_NoteTracker);
	_RegisterComponent(&m_NoteBox);
	_RegisterComponent(&m_Ripple);
	_RegisterComponent(&m_Lyrics);
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRing11::Release()
{
	m_NoteTracker.RemoveListener(&m_Ripple);
	m_NoteTracker.RemoveListener(&m_Lyrics);
	m_NoteBox.Release();
	m_Lyrics.Release();
	m_NoteTracker.Release();

	MTScenePianoRollRingBase11::Release();
}

//******************************************************************************
// Draw notes
//******************************************************************************
int MTScenePianoRollRing11::_DrawNotes(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	return m_NoteBox.Draw(pContext, viewProj, lightDir);
}

//******************************************************************************
// Draw lyrics
//******************************************************************************
int MTScenePianoRollRing11::_DrawLyrics(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir,
		const Vector3& camPos
	)
{
	return m_Lyrics.Draw(pContext, viewProj, lightDir, camPos);
}

//******************************************************************************
// Sequencer message reception
//******************************************************************************
int MTScenePianoRollRing11::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	result = MTScenePianoRollRingBase11::_OnRecvSequencerMsg(param1, param2);
	if (result != 0) goto EXIT;

	parser.Parse(param1, param2);

	if (parser.GetMsg() == SMMsgParser::MsgSkipStart) {
		if (parser.GetSkipStartDirection() == SMMsgParser::SkipBack) {
			m_NotePitchBend.Reset();
		}
		m_NoteBox.Reset();
		m_NoteBox.SetSkipStatus(true);
		m_Ripple.SetSkipStatus(true);
		m_Lyrics.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_NoteBox.SetSkipStatus(false);
		m_Ripple.SetSkipStatus(false);
		m_Lyrics.SetSkipStatus(false);
		m_IsSkipping = false;
		m_NoteTracker.Seek(m_PlayTimeMSec);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Get note count
//******************************************************************************
unsigned long MTScenePianoRollRing11::GetNoteCount() const
{
	return m_NoteBox.GetNoteCount();
}
