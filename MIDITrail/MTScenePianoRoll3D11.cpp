//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D11
//
// PianoRoll 3D/2D scene (Playback).
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll3D11.h"
#include "MTGridBox11.h"
#include "MTPianoKeyboardCtrlRoll11.h"
#include "SMMsgParser.h"
#include "MTLoadingDefs.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRoll3D11::MTScenePianoRoll3D11(bool is2D)
	: MTScenePianoRoll3DBase11(is2D)
{
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRoll3D11::~MTScenePianoRoll3D11()
{
	Release();
}

//******************************************************************************
// Get scene name
//******************************************************************************
const TCHAR* MTScenePianoRoll3D11::GetName() const
{
	return m_Is2D ? _T("PianoRoll2D") : _T("PianoRoll3D");
}

//******************************************************************************
// Mode-specific component creation
//******************************************************************************
int MTScenePianoRoll3D11::_CreateModeComponents(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData,
		const MTLoadProgressContext* pProgress
	)
{
	int result = 0;

	if (pSeqData == NULL) goto EXIT;

	{
		LARGE_INTEGER freq, t0, t1;
		QueryPerformanceFrequency(&freq);

		// Grid (Playback)
		MTLoadLog("Component Grid begin\n"); QueryPerformanceCounter(&t0);
		try { m_pGrid = new MTGridBox11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTGridBox11*)m_pGrid)->Create(pDevice, pContext, GetName(), pSeqData);
		QueryPerformanceCounter(&t1); MTLoadLog("Component Grid: %lld ms\n", (t1.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart);
		if (result != 0) goto EXIT;

		// TimeIndicator
		MTLoadLog("Component TimeIndicator begin\n"); QueryPerformanceCounter(&t0);
		result = m_TimeIndicator.Create(pDevice, pContext, GetName(), pSeqData);
		QueryPerformanceCounter(&t1); MTLoadLog("Component TimeIndicator: %lld ms\n", (t1.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart);
		if (result != 0) goto EXIT;

		// Dashboard
		MTLoadLog("Component Dashboard begin\n"); QueryPerformanceCounter(&t0);
		result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, m_hWnd);
		QueryPerformanceCounter(&t1); MTLoadLog("Component Dashboard: %lld ms\n", (t1.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart);
		if (result != 0) goto EXIT;

		// NoteTracker (progress band: 16% ~ 63%)
		{
			MTProgressBand band = { pProgress, MTLoadBand::TRACKER_START, MTLoadBand::TRACKER_END };
			MTLoadProgressContext ctx = (pProgress != NULL) ? band.ToContext() : MTLoadProgressContext();
			result = m_NoteTracker.Create(pSeqData, (pProgress != NULL) ? &ctx : NULL);
		}
		if (result != 0) goto EXIT;

		m_Dashboard.SetNoteNum(m_NoteTracker.GetNoteCount());

		// Ripple
		MTLoadLog("Component Ripple begin\n"); QueryPerformanceCounter(&t0);
		result = m_Ripple.Create(pDevice, pContext, GetName(), pSeqData, &m_NotePitchBend);
		QueryPerformanceCounter(&t1); MTLoadLog("Component Ripple: %lld ms\n", (t1.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart);
		if (result != 0) goto EXIT;
		m_NoteTracker.AddListener(&m_Ripple, NoteEventType::Note);

		// Lyrics
		MTLoadLog("Component Lyrics begin\n"); QueryPerformanceCounter(&t0);
		result = m_Lyrics.Create(pDevice, pContext, GetName(), pSeqData, &m_NotePitchBend);
		QueryPerformanceCounter(&t1); MTLoadLog("Component Lyrics: %lld ms\n", (t1.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart);
		if (result != 0) goto EXIT;
		m_NoteTracker.AddListener(&m_Lyrics, NoteEventType::Lyric);
		m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

		// NoteBox (Instanced, progress band: 66% ~ 98%)
		{
			MTProgressBand band = { pProgress, MTLoadBand::INSTANCED_START, MTLoadBand::INSTANCED_END };
			MTLoadProgressContext ctx = (pProgress != NULL) ? band.ToContext() : MTLoadProgressContext();
			result = m_NoteBox.Create(pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend,
			                          m_Is2D ? MTAABBMode::Roll2D : MTAABBMode::Roll3D, NULL,
			                          (pProgress != NULL) ? &ctx : NULL);
		}
		if (result != 0) goto EXIT;

		// Keyboard (Playback)
		MTLoadLog("Component Keyboard begin\n"); QueryPerformanceCounter(&t0);
		try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRoll11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTPianoKeyboardCtrlRoll11*)m_pKeyboardCtrl)->Create(
			pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend, m_Is2D);
		QueryPerformanceCounter(&t1); MTLoadLog("Component Keyboard: %lld ms\n", (t1.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart);
	}
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Mode-specific component registration
//******************************************************************************
void MTScenePianoRoll3D11::_RegisterModeComponents()
{
	_RegisterComponent(&m_Dashboard);
	_RegisterComponent(&m_NoteTracker);
	_RegisterComponent(&m_Ripple);
	_RegisterComponent(&m_Lyrics);
	_RegisterComponent(&m_NoteBox);
	_RegisterComponent(m_pKeyboardCtrl);
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRoll3D11::Release()
{
	m_NoteTracker.RemoveListener(&m_Ripple);
	m_NoteTracker.RemoveListener(&m_Lyrics);
	m_NoteBox.Release();
	m_Lyrics.Release();
	m_NoteTracker.Release();

	MTScenePianoRoll3DBase11::Release();
}

//******************************************************************************
// Draw notes
//******************************************************************************
int MTScenePianoRoll3D11::_DrawNotes(
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
int MTScenePianoRoll3D11::_DrawLyrics(
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
int MTScenePianoRoll3D11::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	result = MTScenePianoRoll3DBase11::_OnRecvSequencerMsg(param1, param2);
	if (result != 0) goto EXIT;

	parser.Parse(param1, param2);

	if (parser.GetMsg() == SMMsgParser::MsgSkipStart) {
		if (parser.GetSkipStartDirection() == SMMsgParser::SkipBack) {
			m_NotePitchBend.Reset();
		}
		m_NoteBox.Reset();
		m_NoteBox.SetSkipStatus(true);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->Reset();
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(true);
		m_Ripple.SetSkipStatus(true);
		m_Lyrics.SetSkipStatus(true);
		m_NoteTracker.Seek(0);
		m_IsSkipping = true;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_NoteBox.SetSkipStatus(false);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(false);
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
unsigned long MTScenePianoRoll3D11::GetNoteCount() const
{
	return m_NoteBox.GetNoteCount();
}
