//******************************************************************************
//
// MIDITrail / MTScenePianoRollRainLive11
//
// PianoRoll Rain scene (Live).
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRollRainLive11.h"
#include "MTPianoKeyboardCtrlRainLive11.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRollRainLive11::MTScenePianoRollRainLive11(bool is2D)
	: MTScenePianoRollRainBase11(is2D)
{
	m_IsLive = true;
	m_pNoteRainLive = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRollRainLive11::~MTScenePianoRollRainLive11()
{
	Release();
}

//******************************************************************************
// Get scene name
//******************************************************************************
const TCHAR* MTScenePianoRollRainLive11::GetName() const
{
	return m_Is2D ? _T("PianoRollRain2DLive") : _T("PianoRollRainLive");
}

//******************************************************************************
// Mode-specific component creation
//******************************************************************************
int MTScenePianoRollRainLive11::_CreateModeComponents(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData,
		const MTLoadProgressContext* pProgress
	)
{
	int result = 0;

	// Live Notes
	try { m_pNoteRainLive = new MTNoteAABBLive11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = m_pNoteRainLive->Create(pDevice, pContext, GetName(), &m_NotePitchBend,
	                                 MTAABBLiveMode::Rain);
	if (result != 0) goto EXIT;
	m_pNoteRainLive->SetLightEnable(false);

	// NoteTrackerLive
	result = m_NoteTrackerLive.Create();
	if (result != 0) goto EXIT;

	// Keyboard (Live)
	try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRainLive11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = ((MTPianoKeyboardCtrlRainLive11*)m_pKeyboardCtrl)->Create(
		pDevice, pContext, GetName(), &m_NotePitchBend, m_Is2D);
	if (result != 0) goto EXIT;
	m_NoteTrackerLive.AddListener((MTPianoKeyboardCtrlRainLive11*)m_pKeyboardCtrl, NoteEventType::Note);

	// Dashboard (Live monitor)
	result = m_DashboardLive.Create(pDevice, pContext, GetName(), m_hWnd);
	if (result != 0) goto EXIT;
	m_NoteTrackerLive.AddListener(&m_DashboardLive, NoteEventType::Note);

	// DiagOverlay
	result = m_DiagOverlay.Create(pDevice, pContext, m_hWnd);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Mode-specific component registration
//******************************************************************************
void MTScenePianoRollRainLive11::_RegisterModeComponents()
{
	_RegisterComponent(&m_DashboardLive);
	_RegisterComponent(&m_NoteTrackerLive);
	_RegisterComponent(m_pNoteRainLive);
	_RegisterComponent(m_pKeyboardCtrl);
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRainLive11::Release()
{
	delete m_pNoteRainLive;
	m_pNoteRainLive = NULL;

	MTScenePianoRollRainBase11::Release();
}

//******************************************************************************
// Draw notes
//******************************************************************************
int MTScenePianoRollRainLive11::_DrawNotes(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	if (m_pNoteRainLive != NULL) {
		return m_pNoteRainLive->Draw(pContext, viewProj, lightDir);
	}
	return 0;
}

//******************************************************************************
// Playback start
//******************************************************************************
int MTScenePianoRollRainLive11::OnPlayStart()
{
	MTScenePianoRollRainBase11::OnPlayStart();
	m_isMonitoringActive = true;
	m_DashboardLive.SetMonitoringStatus(true);
	return 0;
}

//******************************************************************************
// Playback end
//******************************************************************************
int MTScenePianoRollRainLive11::OnPlayEnd()
{
	MTScenePianoRollRainBase11::OnPlayEnd();
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
		m_DashboardLive.SetMonitoringStatus(false);
	}
	return 0;
}

//******************************************************************************
// Live note events
//******************************************************************************
void MTScenePianoRollRainLive11::SetNoteOnLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo, unsigned char velocity)
{
	if (!m_isMonitoringActive) return;
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->SetNoteOn(portNo, chNo, noteNo, velocity);
		m_NoteTrackerLive.SetNoteOn(portNo, chNo, noteNo, velocity);
	}
}

void MTScenePianoRollRainLive11::SetNoteOffLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo)
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->SetNoteOff(portNo, chNo, noteNo);
		m_NoteTrackerLive.SetNoteOff(portNo, chNo, noteNo);
	}
}

void MTScenePianoRollRainLive11::AllNoteOffLive()
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
	}
}

void MTScenePianoRollRainLive11::AllNoteOffOnChLive(
		unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->AllNoteOffOnCh(portNo, chNo);
		m_NoteTrackerLive.AllNoteOffOnCh(portNo, chNo);
	}
}

//******************************************************************************
// Dashboard overrides (Live)
//******************************************************************************
int MTScenePianoRollRainLive11::_DrawDashboard(
		ID3D11DeviceContext* pContext,
		unsigned int screenWidth, unsigned int screenHeight,
		MTSceneLayoutInfo* pLayoutInfo)
{
	return m_DashboardLive.Draw(pContext, screenWidth, screenHeight, pLayoutInfo);
}

void MTScenePianoRollRainLive11::_OnDashboardWindowResize()
{
	m_DashboardLive.OnWindowResize();
	m_DiagOverlay.OnWindowResize();
}

void MTScenePianoRollRainLive11::_SetDashboardEnable(bool isEnable)
{
	m_DashboardLive.SetEnable(isEnable);
}

int MTScenePianoRollRainLive11::OnMIDIINDeviceChanged(const TCHAR* pName)
{
	return m_DashboardLive.SetMIDIINDeviceName(pName);
}
