//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingLive11
//
// PianoRoll Ring scene (Live).
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRollRingLive11.h"
#include "MTGridRingLive11.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRollRingLive11::MTScenePianoRollRingLive11()
	: MTScenePianoRollRingBase11()
{
	m_IsLive = true;
	m_pNoteBoxLive = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRollRingLive11::~MTScenePianoRollRingLive11()
{
	Release();
}

//******************************************************************************
// Get scene name
//******************************************************************************
const TCHAR* MTScenePianoRollRingLive11::GetName() const
{
	return _T("PianoRollRingLive");
}

//******************************************************************************
// Mode-specific component creation
//******************************************************************************
int MTScenePianoRollRingLive11::_CreateModeComponents(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData
	)
{
	int result = 0;

	// Live Notes
	try { m_pNoteBoxLive = new MTNoteCylindricalLive11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = m_pNoteBoxLive->Create(pDevice, pContext, GetName(), &m_NotePitchBend);
	if (result != 0) goto EXIT;
	m_pNoteBoxLive->SetLightEnable(false);

	// NoteTrackerLive
	result = m_NoteTrackerLive.Create();
	if (result != 0) goto EXIT;

	// Grid Ring (Live)
	try { m_pGridRing = new MTGridRingLive11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = ((MTGridRingLive11*)m_pGridRing)->Create(pDevice, pContext, GetName());
	if (result != 0) goto EXIT;

	// TimeIndicator Ring (pSeqData=NULL OK)
	result = m_TimeIndicator.Create(pDevice, pContext, GetName(), NULL);
	if (result != 0) goto EXIT;

	// PictBoard Ring (pSeqData=NULL OK)
	result = m_PictBoard.Create(pDevice, pContext, GetName(), NULL);
	if (result != 0) goto EXIT;

	// Ripple (inject NoteDesignRingLive11 for linear decay + Ring coordinates)
	result = m_NoteDesignRingLive.Initialize(GetName());
	if (result != 0) goto EXIT;
	result = m_Ripple.Create(pDevice, pContext, GetName(), NULL, &m_NotePitchBend, &m_NoteDesignRingLive);
	if (result != 0) goto EXIT;
	m_NoteTrackerLive.AddListener(&m_Ripple, NoteEventType::Note);

	// Dashboard (Live monitor)
	result = m_DashboardLive.Create(pDevice, pContext, GetName(), m_hWnd);
	if (result != 0) goto EXIT;
	m_NoteTrackerLive.AddListener(&m_DashboardLive, NoteEventType::Note);

EXIT:;
	return result;
}

//******************************************************************************
// Mode-specific component registration
//******************************************************************************
void MTScenePianoRollRingLive11::_RegisterModeComponents()
{
	_RegisterComponent(&m_DashboardLive);
	_RegisterComponent(&m_NoteTrackerLive);
	_RegisterComponent(&m_Ripple);
	_RegisterComponent(m_pNoteBoxLive);
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRingLive11::Release()
{
	delete m_pNoteBoxLive;
	m_pNoteBoxLive = NULL;

	MTScenePianoRollRingBase11::Release();
}

//******************************************************************************
// Draw notes
//******************************************************************************
int MTScenePianoRollRingLive11::_DrawNotes(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	if (m_pNoteBoxLive != NULL) {
		return m_pNoteBoxLive->Draw(pContext, viewProj, lightDir);
	}
	return 0;
}

//******************************************************************************
// Playback start
//******************************************************************************
int MTScenePianoRollRingLive11::OnPlayStart()
{
	MTScenePianoRollRingBase11::OnPlayStart();
	m_isMonitoringActive = true;
	m_DashboardLive.SetMonitoringStatus(true);
	return 0;
}

//******************************************************************************
// Playback end
//******************************************************************************
int MTScenePianoRollRingLive11::OnPlayEnd()
{
	MTScenePianoRollRingBase11::OnPlayEnd();
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
		m_DashboardLive.SetMonitoringStatus(false);
	}
	return 0;
}

//******************************************************************************
// Live note events
//******************************************************************************
void MTScenePianoRollRingLive11::SetNoteOnLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo, unsigned char velocity)
{
	if (!m_isMonitoringActive) return;
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOn(portNo, chNo, noteNo, velocity);
		m_NoteTrackerLive.SetNoteOn(portNo, chNo, noteNo, velocity);
	}
}

void MTScenePianoRollRingLive11::SetNoteOffLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOff(portNo, chNo, noteNo);
		m_NoteTrackerLive.SetNoteOff(portNo, chNo, noteNo);
	}
}

void MTScenePianoRollRingLive11::AllNoteOffLive()
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
	}
}

void MTScenePianoRollRingLive11::AllNoteOffOnChLive(
		unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOffOnCh(portNo, chNo);
		m_NoteTrackerLive.AllNoteOffOnCh(portNo, chNo);
	}
}

//******************************************************************************
// Dashboard overrides (Live)
//******************************************************************************
int MTScenePianoRollRingLive11::_DrawDashboard(
		ID3D11DeviceContext* pContext,
		unsigned int screenWidth, unsigned int screenHeight)
{
	return m_DashboardLive.Draw(pContext, screenWidth, screenHeight);
}

void MTScenePianoRollRingLive11::_OnDashboardWindowResize()
{
	m_DashboardLive.OnWindowResize();
}

void MTScenePianoRollRingLive11::_SetDashboardEnable(bool isEnable)
{
	m_DashboardLive.SetEnable(isEnable);
}

int MTScenePianoRollRingLive11::OnMIDIINDeviceChanged(const TCHAR* pName)
{
	return m_DashboardLive.SetMIDIINDeviceName(pName);
}
